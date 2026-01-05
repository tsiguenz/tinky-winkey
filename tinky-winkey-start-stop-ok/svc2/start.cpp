#include "svc.h"

int ActionStart(void)
{
    SC_HANDLE scm = nullptr;
    SC_HANDLE serviceHandle = nullptr;
    SERVICE_STATUS_PROCESS ssStatus;
    DWORD dwOldCheckPoint;
    DWORD dwStartTickCount;
    DWORD dwWaitTime;
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

    // Make sure the service is not already started.

    if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssStatus,
            sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded)) {
        std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
        Cleanup(scm, serviceHandle);
        return 1;
    }

    // Check if the service is already running.

    if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING) {
        std::cout << "Cannot start the service because it is already running\n";
        Cleanup(scm, serviceHandle);
        return 0;
    }

    // Wait for the service to stop before attempting to start it.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth of the wait hint but not less than 1 second
        // and not more than 10 seconds.

        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Check the status until the service is no longer stop pending.

        if (!QueryServiceStatusEx(
                serviceHandle, // handle to service
                SC_STATUS_PROCESS_INFO, // information level
                (LPBYTE)&ssStatus, // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded)) // size needed if buffer is too small
        {
            std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
            Cleanup(scm, serviceHandle);
            return 1;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
            // Continue to wait and check.
            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                printf("Timeout waiting for service to stop\n");
                Cleanup(scm, serviceHandle);
                return 1;
            }
        }
    }

    // Attempt to start the service.

    if (!StartService(serviceHandle, 0, nullptr)) {
        std::cerr << "Fail to start the service: " << GetLastError() << std::endl;
        returnValue = 1;
    } else
        std::cout << "Service start pending...\n";

    // Check the status until the service is no longer start pending.

    if (!QueryServiceStatusEx(
            serviceHandle, // handle to service
            SC_STATUS_PROCESS_INFO, // info level
            (LPBYTE)&ssStatus, // address of structure
            sizeof(SERVICE_STATUS_PROCESS), // size of structure
            &dwBytesNeeded)) // if buffer too small
    {
        std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
        Cleanup(scm, serviceHandle);
        return 1;
    }

    // Save the tick count and initial checkpoint.

    dwStartTickCount = GetTickCount();
    dwOldCheckPoint = ssStatus.dwCheckPoint;

    while (ssStatus.dwCurrentState == SERVICE_START_PENDING) {
        // Do not wait longer than the wait hint. A good interval is
        // one-tenth the wait hint, but no less than 1 second and no
        // more than 10 seconds.
        std::cout << "Service start pending...\n";
        dwWaitTime = ssStatus.dwWaitHint / 10;

        if (dwWaitTime < 1000)
            dwWaitTime = 1000;
        else if (dwWaitTime > 10000)
            dwWaitTime = 10000;

        Sleep(dwWaitTime);

        // Check the status again.

        if (!QueryServiceStatusEx(
                serviceHandle, // handle to service
                SC_STATUS_PROCESS_INFO, // info level
                (LPBYTE)&ssStatus, // address of structure
                sizeof(SERVICE_STATUS_PROCESS), // size of structure
                &dwBytesNeeded)) // if buffer too small
        {
            std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
            break;
        }

        if (ssStatus.dwCheckPoint > dwOldCheckPoint) {
            // Continue to wait and check.

            dwStartTickCount = GetTickCount();
            dwOldCheckPoint = ssStatus.dwCheckPoint;
        } else {
            if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint) {
                // No progress made within the wait hint.
                break;
            }
        }
    }

    // Determine whether the service is running.

    if (ssStatus.dwCurrentState == SERVICE_RUNNING) {
        std::cout << "Service started successfully.\n";
    } else {
        std::cout << "Service not started. \n";
        std::cout << "  Current State: " << ssStatus.dwCurrentState << std::endl;
        std::cout << "  Exit Code: " << ssStatus.dwWin32ExitCode << std::endl;
        std::cout << "  Check Point: " << ssStatus.dwCheckPoint << std::endl;
        std::cout << "  Wait Hint: " << ssStatus.dwWaitHint << std::endl;
    }
    Cleanup(scm, serviceHandle);
    return returnValue;
}