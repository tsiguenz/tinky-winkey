#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 minimum
#endif
#define NTDDI_VERSION 0x0A000000

#define WIN32_LEAN_AND_MEAN // reduce size of the windows headers
#define NOMINMAX
#pragma comment(lib, "Advapi32.lib")
#include <windows.h>
#include <cwchar>
#include <cstdio>

#define SVCNAME L"tinky"

// info about WINAPI in functions signatures :
// #define WINAPI __stdcall
// it is SCM that calls our code, and Windows expects __stdcall signature

SERVICE_STATUS          gSvcStatus{}; 
SERVICE_STATUS_HANDLE   gSvcStatusHandle = nullptr; 
HANDLE                  ghSvcStopEvent = nullptr;

VOID WINAPI SvcCtrlHandler(DWORD);
VOID ReportSvcStatus(DWORD, DWORD, DWORD);
VOID SvcInit(DWORD, LPWSTR*);
void SvcReportEvent(const wchar_t*);
void LogEvent(const wchar_t*);

void SvcReportEvent(const wchar_t* msg)
{
    wprintf(L"%s\n", msg);
}

void LogEvent(const wchar_t* msg)
{
    const wchar_t* logPath = L"C:\\Windows\\tmp\\winkey.log"; // sûr pour LocalSystem
    FILE* f = nullptr;
    if (_wfopen_s(&f, logPath, L"a+") == 0) {
        fwprintf(f, L"%s\n", msg);
        fclose(f);
    }
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
VOID SvcInit( DWORD dwArgc, LPWSTR *lpszArgv)
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
                         nullptr,    // default security attributes
                         TRUE,    // manual reset event
                         FALSE,   // not signaled
                         nullptr);   // no name

    if ( ghSvcStopEvent == nullptr)
    {
        ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
        return;
    }

    // Report running status when initialization is complete.

    ReportSvcStatus( SERVICE_RUNNING, NO_ERROR, 0 );

    // TO_DO: Perform work until service stops.

    int counter = 0;
    while(WaitForSingleObject(ghSvcStopEvent, 1000) == WAIT_TIMEOUT)
    {
        // Check whether to stop the service every 1000ms, reads WAIT_TIMEOUT if no stop
        // do work
        counter++;
        wprintf(L"Service running: %d seconds\n", counter);
    }
    wchar_t buf[128];
    swprintf_s(buf, L"Service stopped after %d seconds", counter);
    LogEvent(buf);
    ReportSvcStatus( SERVICE_STOPPED, NO_ERROR, 0 );
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
VOID ReportSvcStatus( DWORD dwCurrentState,
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
    else gSvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ( (dwCurrentState == SERVICE_RUNNING) ||
           (dwCurrentState == SERVICE_STOPPED) )
        gSvcStatus.dwCheckPoint = 0;
    else gSvcStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus( gSvcStatusHandle, &gSvcStatus );
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
VOID WINAPI SvcCtrlHandler( DWORD dwCtrl )
{
   // Handle the requested control code. 

   if (dwCtrl == SERVICE_CONTROL_STOP)
   { 
         ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);

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
VOID WINAPI SvcMain( DWORD dwArgc, LPWSTR *lpszArgv )
{
    (void)dwArgc;
    (void)lpszArgv;

    // Register the handler function for the service

    gSvcStatusHandle = RegisterServiceCtrlHandlerW( 
        SVCNAME, 
        SvcCtrlHandler);

    if( !gSvcStatusHandle )
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

    ReportSvcStatus( SERVICE_START_PENDING, NO_ERROR, 3000 );

    // Perform service-specific initialization and work.

    SvcInit( dwArgc, lpszArgv );
}

// If the function succeeds, the return value is nonzero.
// If the function fails, the return value is zero. To get extended error information, call GetLastError.
//The members of the last entry in the table must have NULL values to designate the end of the table.

int wmain()
{
    SERVICE_TABLE_ENTRYW table[] =
    {
        { (LPWSTR)L"Winkey", SvcMain },
        { nullptr, nullptr }
    };

    StartServiceCtrlDispatcherW(table);
    return 0;
}