#include "svc.h"

int ActionInstall(void)
{
    SC_HANDLE scm = nullptr;
    SC_HANDLE serviceHandle = nullptr;

    scm = OpenSCManager(nullptr, nullptr,
        SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
    if (scm == nullptr) {
        std::cerr << "Error while opening SCM: " << GetLastError() << std::endl;
        return 1;
    }

    LPCWSTR serviceName = L"tinky";
    LPCWSTR serviceBinaryPath = L"C:\\Users\\tinky\\Documents\\src\\winkey.exe";

    // Install the service into SCM by calling CreateService
    serviceHandle = CreateService(scm, // SCManager database
        serviceName, // Name of service
        serviceName, // Name to display
        SERVICE_ALL_ACCESS, // Desired access
        SERVICE_WIN32_OWN_PROCESS, // Service type
        SERVICE_DEMAND_START, // Service start type
        SERVICE_ERROR_NORMAL, // Error control type
        serviceBinaryPath, // Service's binary
        nullptr, // No load ordering group
        nullptr, // No tag identifier
        nullptr, // Dependencies
        nullptr, // Service running account
        nullptr // Password of the account
    );
    if (serviceHandle == nullptr) {
        DWORD errorCode = GetLastError();
        int returnValue;
        if (errorCode == ERROR_SERVICE_EXISTS) {
            std::cout << "Service is already installed!\n";
            returnValue = 0;
        } else {
            std::cerr << "Error while creating the service: " << errorCode
                      << std::endl;
            returnValue = 1;
        }
        Cleanup(scm, serviceHandle);
        return returnValue;
    }
    Cleanup(scm, serviceHandle);
    std::cout << "Service installed successfully!\n";
    return 0;
}