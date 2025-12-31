#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 minimum
#endif

#define WIN32_LEAN_AND_MEAN // reduce size of the windows headers
#define NOMINMAX
#define UNICODE
#define _UNICODE
#include <iostream>
#include <map>
#include <windows.h>
#pragma comment(lib, "Advapi32.lib")

void Usage(void) {
  std::cerr << "Usage: svc.exe [install|start|stop|delete]" << std::endl;
}

void cleanup(SC_HANDLE scm, SC_HANDLE serviceHandle) {
  if (scm) {
    CloseServiceHandle(scm);
    scm = nullptr;
  }
  if (serviceHandle) {
    CloseServiceHandle(serviceHandle);
    serviceHandle = nullptr;
  }
}

int ActionInstall(void) {
  SC_HANDLE scm = nullptr;
  SC_HANDLE serviceHandle = nullptr;

  scm = OpenSCManager(nullptr, nullptr,
                      SC_MANAGER_CONNECT | SC_MANAGER_CREATE_SERVICE);
  if (scm == nullptr) {
    std::cerr << "Error while opening SCM: " << GetLastError() << std::endl;
    return 1;
  }

  LPCWSTR serviceName = L"tinky";
  LPCWSTR serviceBinaryPath = L"calc.exe";

  // Install the service into SCM by calling CreateService
  serviceHandle = CreateService(scm,                       // SCManager database
                                serviceName,               // Name of service
                                serviceName,               // Name to display
                                SERVICE_ALL_ACCESS,        // Desired access
                                SERVICE_WIN32_OWN_PROCESS, // Service type
                                SERVICE_DEMAND_START,      // Service start type
                                SERVICE_ERROR_NORMAL,      // Error control type
                                serviceBinaryPath,         // Service's binary
                                nullptr, // No load ordering group
                                nullptr, // No tag identifier
                                nullptr, // Dependencies
                                nullptr, // Service running account
                                nullptr  // Password of the account
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
    cleanup(scm, serviceHandle);
    return returnValue;
  }
  cleanup(scm, serviceHandle);
  std::cout << "Service installed successfully!\n";
  return 0;
}

int ActionStart(void) {
  std::cout << "Starting...\n";
  return 1;
}

int ActionStop(void) {
  std::cout << "Stopping...\n";
  return 0;
}

int ActionDelete(void) {
  SC_HANDLE scm = nullptr;
  SC_HANDLE serviceHandle = nullptr;
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
    cleanup(scm, serviceHandle);
    return returnValue;
  }
  if (!DeleteService(serviceHandle)) {
    std::cerr << "Fail to delete service\n";
    returnValue = 1;
  } else
    std::cout << "Service deleted successfully!\n";
  cleanup(scm, serviceHandle);
  return returnValue;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Bad number of arguments.\n";
    Usage();
    return 1;
  }

  const std::map<const std::string, int (*)(void)> actions{
      {"install", &ActionInstall},
      {"start", &ActionStart},
      {"stop", &ActionStop},
      {"delete", &ActionDelete}};

  try {
    int res = actions.at(argv[1])();
    if (res != 0) {
      std::cerr << "Error status: " << res << std::endl;
      return res;
    }
  } catch (std::exception &e) {
    (void)e;
    std::cerr << "Unknown action: " << argv[1] << std::endl;
    Usage();
    return 1;
  }

  std::cout << "Done!\n";

  return 0;
}
