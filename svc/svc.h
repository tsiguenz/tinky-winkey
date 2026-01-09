#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Specifies Windows 10 minimum to execute our code
#endif

#define WIN32_LEAN_AND_MEAN // Reduce size of the windows headers
#define NOMINMAX            // To avoid conflicts with Windows headers redefining std::max
#define _UNICODE            // Helps enabling default W windows functions
#define UNICODE             // Helps enabling default W windows functions
#include <windows.h>

#include <iostream>
#include <map>
#pragma comment(lib, "Advapi32.lib")

void Cleanup(SC_HANDLE scm, SC_HANDLE serviceHandle);
int ActionInstall(void);
int ActionStart(void);
int ActionStop(void);
int ActionDelete(void);