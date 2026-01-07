#include "svc.h"

int ActionDelete(void)
{
    SC_HANDLE scm = nullptr;
    SC_HANDLE serviceHandle = nullptr;
    SERVICE_STATUS_PROCESS ssp;
    DWORD dwBytesNeeded;
    (void)ssp; // TODO: remove
    int returnValue = 0;

    scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (scm == nullptr)
    {
        std::cerr << "Error while opening SCM: " << GetLastError() << std::endl;
        return 1;
    }
    serviceHandle = OpenService(scm, L"tinky", SERVICE_ALL_ACCESS);
    if (serviceHandle == nullptr)
    {
        DWORD errorCode = GetLastError();
        if (errorCode == ERROR_SERVICE_DOES_NOT_EXIST)
        {
            std::cout << "Service does not exist!\n";
        }
        else
        {
            std::cerr << "Error while opening the service: " << errorCode
                      << std::endl;
            returnValue = 1;
        }
        Cleanup(scm, serviceHandle);
        return returnValue;
    }

    // Make sure the service is stopped

    if (!QueryServiceStatusEx(serviceHandle, SC_STATUS_PROCESS_INFO, (LPBYTE)&ssp,
                              sizeof(SERVICE_STATUS_PROCESS), &dwBytesNeeded))
    {
        std::cerr << "QueryServiceStatusEx failed: " << GetLastError() << std::endl;
        Cleanup(scm, serviceHandle);
        return 1;
    }

    if (ssp.dwCurrentState == SERVICE_RUNNING)
    {
        std::cout << "Service is running!\n";
        if (ActionStop() == 1)
        {
            Cleanup(scm, serviceHandle);
            return 1;
        }
    }

    if (!DeleteService(serviceHandle))
    {
        std::cerr << "Fail to delete service\n";
        returnValue = 1;
    }
    else
        std::cout << "Service deleted successfully!\n";
    Cleanup(scm, serviceHandle);
    return returnValue;
}