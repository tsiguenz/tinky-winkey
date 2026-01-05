#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 minimum
#endif

#define WIN32_LEAN_AND_MEAN // reduce size of the windows headers
#define NOMINMAX
#include <windows.h>

#include <iostream>
#include <map>
#pragma comment(lib, "Advapi32.lib")

void Cleanup(SC_HANDLE scm, SC_HANDLE serviceHandle);
int ActionInstall(void);
int ActionStart(void);
int ActionStop(void);
int ActionDelete(void);