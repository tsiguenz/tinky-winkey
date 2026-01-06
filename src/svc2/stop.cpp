#include "svc.h"

int ActionStop(void)
{
    SC_HANDLE scm = nullptr;
    SC_HANDLE serviceHandle = nullptr;
    SERVICE_STATUS_PROCESS ssp;
    DWORD dwStartTime = GetTickCount();
    DWORD dwWaitTime;
    DWORD dwTimeout = 30000; // 30-second time-out
    DWORD dwBytesNeeded;
    int returnValue = 0;

    scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (scm == nullptr) {
        std::cerr << "Error while opening SCM: " << GetLastError() << std::endl;
        return 1;
    }
    serviceHandle = OpenService(scm, L"tinky", SERVICE_ALL_ACCESS);
    if (serviceHandle == nullptr) {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_SERVICE_DOES_NOT_EXIST) {
            std::cout << "Service does not exist!\n";
        } else {
            std::cerr << "Error while opening the service: " << errorCode
                      << std::endl;
            returnValue = 1;
        }
        Cleanup(scm, serviceHandle);
        return returnValue;
    }

    // Make sure the service is not already stopped.

    if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
            sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
        Cleanup(scm, serviceHandle);
        return 1;
    }

    if (ssp.dwCurrentState == SERVICE_STOPPED) {
        std::cout << "Service is already stopped.\n";
        Cleanup(scm, serviceHandle);
        return 1;
    }

    // If a stop is pending, wait for it.

    while (ssp.dwCurrentState == SERVICE_STOP_PENDING) {
        std::cout << "Service stop pending...\n";

        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        dwWaitTime = ssp.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
            std::cerr << "QueryServiceStatusEx failed: ", GetLastError();
            Cleanup(scm, serviceHandle);
            return 1;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED) {
            std::cout << "Service stopped successfully.\n";
            Cleanup(scm, serviceHandle);
            return 0;
        }

        if (GetTickCount() - dwStartTime > dwTimeout) {
            std::cerr << "Service stop timed out.\n";
            Cleanup(scm, serviceHandle);
            return 1;
        }
    }

    // Send a stop code to the service

    if (!ControlService(serviceHandle, SERVICE_CONTROL_STOP, (LPSERVICE_STATUS)&ssp)) {
        std::cerr << "Can't send a stop code to the service: " << GetLastError()
                  << std::endl;
        Cleanup(scm, serviceHandle);
        return 1;
    }

    // Wait for the service to stop.

    while (ssp.dwCurrentState != SERVICE_STOPPED) {
        Sleep(ssp.dwWaitHint);
        if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
            std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
            Cleanup(scm, serviceHandle);
            return 1;
        }

        if (ssp.dwCurrentState == SERVICE_STOPPED)
            break;

        if (GetTickCount() - dwStartTime > dwTimeout) {
            std::cerr << "Wait timed out\n";
            Cleanup(scm, serviceHandle);
            return 1;
        }
    }
    std::cout << "Service stopped successfully!\n";
    Cleanup(scm, serviceHandle);
    return returnValue;
}