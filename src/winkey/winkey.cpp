#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 minimum
#endif
#define NTDDI_VERSION 0x0A000000

#define WIN32_LEAN_AND_MEAN // reduce size of the windows headers
#define NOMINMAX
#define _UNICODE
#define UNICODE
#pragma comment(lib, "Advapi32.lib")
#include <windows.h>
#include <cwchar>
#include <cstdio>
#include <iostream>

#define SVCNAME L"tinky"

// info about WINAPI in functions signatures :
// #define WINAPI __stdcall
// it is SCM that calls our code, and Windows expects __stdcall signature

SERVICE_STATUS gSvcStatus{};
SERVICE_STATUS_HANDLE gSvcStatusHandle = nullptr;
HANDLE ghSvcStopEvent = nullptr;
HANDLE hwinkey = nullptr;

VOID WINAPI SvcCtrlHandler(DWORD);
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPWSTR *);
void SvcReportEvent(const wchar_t *);
void LogEvent(const wchar_t *);

// TOKEN IMPERSONATION*************************
#include <tlhelp32.h>

HANDLE GetSystemTokenFromWinlogon()
{
    // Our aim is to get the PID of winlogon to retrieve its token
    DWORD pid = 0;

    // 1. Find winlogon.exe
    // Takes a snapshot of the system current processes
    // the option TH32CS_SNAPPROCESS tells to take all the processes
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    // PROCESSENTRY32 is awindows structure that contains a process information
    // we have to precise the size according to windows
    PROCESSENTRY32 pe{sizeof(pe)};

    // to parse the snapshot by each process
    // Proces32First is called once, Process32Next is called for remaining iterations
    for (Process32First(snap, &pe); Process32Next(snap, &pe);)
    {
        // _wcsicmp = Wide Char String – Case Insensitive Compare from Windows
        if (_wcsicmp(pe.szExeFile, L"winlogon.exe") == 0)
        {
            pid = pe.th32ProcessID;
            break;
        }
    }
    CloseHandle(snap);

    // if winlogon not found
    if (!pid)
        return NULL;

    // 2. Open process SYSTEM
    HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (!hProc)
        return NULL;

    // 3. Open its token
    HANDLE hToken = NULL;
    if (!OpenProcessToken(hProc, TOKEN_DUPLICATE | TOKEN_QUERY, &hToken))
    {
        CloseHandle(hProc);
        return NULL;
    }

    CloseHandle(hProc);
    return hToken;
}

bool ImpersonateSystem(HANDLE hDupToken)
{
    // Checcks if the impresonated token still exists
    if (!hDupToken)
        return false;

    // Set the thread token to the duplicated one from impersonation
    if (!SetThreadToken(NULL, hDupToken))
    {
        std::cerr << "SetThreadToken failed: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}

void SvcReportEvent(const wchar_t *msg)
{
    wprintf(L"%s\n", msg);
}

void LogEvent(const wchar_t *msg)
{
    const wchar_t *logPath = L"C:\\Windows\\Temp\\winkey.log";
    FILE *f = nullptr;
    errno_t err = _wfopen_s(&f, logPath, L"a+");
    if (err != 0 || f == nullptr)
    {
        wchar_t buf[256];
        swprintf_s(buf, L"Failed to open log file, errno=%d, GetLastError=%lu", err, GetLastError());
        OutputDebugStringW(buf); // can see that with DebugView
        return;
    }
    fwprintf(f, L"%s\n", msg);
    fclose(f);
}

//
// Purpose:
//   The service code
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
// Return value:
//   None
//
VOID SvcInit(DWORD dwArgc, LPWSTR *lpszArgv)
{
    // TO_DO: Declare and set any required variables.
    //   Be sure to periodically call ReportSvcStatus() with
    //   SERVICE_START_PENDING. If initialization fails, call
    //   ReportSvcStatus with SERVICE_STOPPED.

    // Create an event. The control handler function, SvcCtrlHandler,
    // signals this event when it receives the stop control code.
    // IT IS USED TO WAKE OR STOP OUR CODE (for WaitSingleObject)

    LogEvent(L"SvcInit entered");
    (void)dwArgc;
    (void)lpszArgv;

    ghSvcStopEvent = CreateEvent(
        nullptr,  // default security attributes
        TRUE,     // manual reset event
        FALSE,    // not signaled
        nullptr); // no name

    if (ghSvcStopEvent == nullptr)
    {
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    // Report running status when initialization is complete.

    ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);

    // token duplication and impersonation
    HANDLE hTokenDup = nullptr;
    HANDLE hToken = GetSystemTokenFromWinlogon();
    if (hToken)
    {
        DuplicateTokenEx(
            hToken,
            TOKEN_ALL_ACCESS,
            nullptr,
            SecurityIdentification,
            TokenPrimary,
            &hTokenDup);
        CloseHandle(hToken);
    }

    if (!hTokenDup)
    {
        LogEvent(L"Failed to get SYSTEM token");
        ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
        return;
    }

    // TO_DO: Perform work until service stops.

    STARTUPINFOW si = {0};
    si.cb = sizeof(si);
    si.lpDesktop = (LPWSTR)L"WinSta0\\Default";

    PROCESS_INFORMATION pi;

    BOOL bResult = CreateProcessAsUserW(
        hTokenDup,
        L"C:\\Users\\tinky\\Documents\\src\\process.exe",
        nullptr,
        nullptr,
        nullptr,
        FALSE,
        DETACHED_PROCESS,
        nullptr,
        nullptr,
        &si,
        &pi);

    RevertToSelf();
    if (bResult == 0)
    {
        wchar_t buf[256];
        swprintf_s(buf, L"Failed to create, GetLastError=%lu", GetLastError());
        LogEvent(buf);
    }
    hwinkey = pi.hProcess;
    // success
    // if (bResult && pi.hProcess != INVALID_HANDLE_VALUE)
    // {
    //     WaitForSingleObject(pi.hProcess, INFINITE);
    //     CloseHandle(pi.hProcess);
    // }

    // if (pi.hThread != INVALID_HANDLE_VALUE)
    //     CloseHandle(pi.hThread);

    int counter = 0;
    while (WaitForSingleObject(ghSvcStopEvent, 1000) == WAIT_TIMEOUT)
    {
        counter++;
        // Check whether to stop the service every 1000ms, reads WAIT_TIMEOUT if no stop

        // guarantees that the following work is executed under the impersonated token
        // if (!ImpersonateSystem(hTokenDup))
        //     continue; // failure -> go to next iteration

        // __try and __finally are Windows extensions called SEH (Structured Exception Handling)
        // they are an equivalent to try catch
        // __try
        // {
        //     // Code under the impersonated token (should be SYSTEM from winlogon)
        //     counter++;
        // }
        // __finally
        // {
        //     // It HAS to be done under any circumstancces
        //     // do we close the handle of TokenDup ?
        //     RevertToSelf();
        // }
    }
    // TODO : kill winkey process
    wchar_t buf[128];
    swprintf_s(buf, L"Service stopped after %d seconds", counter);
    LogEvent(buf);
    ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
    return;
}

//
// Purpose:
//   Sets the current service status and reports it to the SCM.
//
// Parameters:
//   dwCurrentState - The current state (see SERVICE_STATUS)
//   dwWin32ExitCode - The system error code
//   dwWaitHint - Estimated time for pending operation,
//     in milliseconds
//
// Return value:
//   None
//
VOID ReportSvcStatus(DWORD dwCurrentState,
                     DWORD dwWin32ExitCode,
                     DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    gSvcStatus.dwCurrentState = dwCurrentState;
    gSvcStatus.dwWin32ExitCode = dwWin32ExitCode;
    gSvcStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        gSvcStatus.dwControlsAccepted = 0;
    else
        gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        gSvcStatus.dwCheckPoint = 0;
    else
        gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(gSvcStatusHandle, &gSvcStatus);
}

//
// Purpose:
//   Called by SCM whenever a control code is sent to the service
//   using the ControlService function.
//
// Parameters:
//   dwCtrl - control code
//
// Return value:
//   None
//
VOID WINAPI SvcCtrlHandler(DWORD dwCtrl)
{
    // Handle the requested control code.

    if (dwCtrl == SERVICE_CONTROL_STOP)
    {
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

        if (hwinkey != nullptr) {
            if (TerminateProcess(hwinkey, 0) == 0)
                LogEvent(L"Error while terminating the process winkey!");
        }
        // Signal the service to stop.

        SetEvent(ghSvcStopEvent);
        ReportSvcStatus(gSvcStatus.dwCurrentState, NO_ERROR, 0);
    }
}

//
// Purpose:
//   Entry point for the service
//
// Parameters:
//   dwArgc - Number of arguments in the lpszArgv array
//   lpszArgv - Array of strings. The first string is the name of
//     the service and subsequent strings are passed by the process
//     that called the StartService function to start the service.
//
// Return value:
//   None.
//
VOID WINAPI SvcMain(DWORD dwArgc, LPWSTR *lpszArgv)
{
    (void)dwArgc;
    (void)lpszArgv;

    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandlerW(
        SVCNAME,
        SvcCtrlHandler);

    if (!gSvcStatusHandle)
    {
        SvcReportEvent(L"RegisterServiceCtrlHandler");
        return;
    }

    // These SERVICE_STATUS members remain as set here

    gSvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    gSvcStatus.dwWin32ExitCode = 0;
    gSvcStatus.dwServiceSpecificExitCode = 0;
    gSvcStatus.dwCheckPoint = 0;
    gSvcStatus.dwWaitHint = 0;

    // Report initial status to the SCM

    ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

    // Perform service-specific initialization and work.

    SvcInit(dwArgc, lpszArgv);
}

// If the function succeeds, the return value is nonzero.
// If the function fails, the return value is zero. To get extended error information, call GetLastError.
// The members of the last entry in the table must have NULL values to designate the end of the table.

int wmain()
{
    SERVICE_TABLE_ENTRYW table[] =
        {
            {(LPWSTR)L"tinky", SvcMain},
            {nullptr, nullptr}};

    StartServiceCtrlDispatcherW(table);
    return 0;
}