#include "svc.h"

int ActionDelete(void)
{
    SC_HANDLE scm = nullptr;
    SC_HANDLE serviceHandle = nullptr;
    SERVICE_STATUS_PROCESS ssp;
    (void)ssp; // TODO: remove
    int returnValue = 0;

    scm = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (scm == nullptr) {
        std::cerr << "Error while opening SCM: " << GetLastError() << std::endl;
        return 1;
    }
    serviceHandle = OpenService(scm, L"tinky", DELETE);
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
    // TODO : check if running to stop before deleting
    if (!DeleteService(serviceHandle)) {
        std::cerr << "Fail to delete service\n";
        returnValue = 1;
    } else
        std::cout << "Service deleted successfully!\n";
    Cleanup(scm, serviceHandle);
    return returnValue;
}