#include "svc.h"

void Cleanup(SC_HANDLE scm, SC_HANDLE serviceHandle)
{
    if (scm)
    {
        CloseServiceHandle(scm);
        scm = nullptr;
    }
    if (serviceHandle)
    {
        CloseServiceHandle(serviceHandle);
        serviceHandle = nullptr;
    }
}