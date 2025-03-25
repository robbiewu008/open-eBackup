#ifndef _MONITOR_HEADER_H_
#define _MONITOR_HEADER_H_

#include "common/Defines.h"
// Monitor退出的标记位
class Monitor {
public:
    static Monitor& GetInstace()
    {
        static Monitor monitorExitFlag;
        return monitorExitFlag;
    }

    mp_bool GetExitFlag()
    {
        return exitFlag;
    }

    mp_void SetExitFlag(mp_bool eFlag)
    {
        exitFlag = eFlag;
    }

private:
    mp_bool exitFlag;
    
private:
    Monitor() : exitFlag(MP_FALSE) {}
    ~Monitor() {}
};

#ifdef WIN32
DWORD WINAPI MonitorServiceHandler(DWORD request, DWORD dwEventType, LPVOID lpEventData, LPVOID lpContext);

// Windows下句柄类单例
class WinServerStatus {
public:
    static WinServerStatus& GetInstace()
    {
        static WinServerStatus winServerStatus;
        return winServerStatus;
    }

    SERVICE_STATUS_HANDLE& GetServiceHandle()
    {
        return hServiceStatus;
    }

    mp_void InitServiceHandle()
    {
        hServiceStatus = RegisterServiceCtrlHandlerExW((LPCWSTR)AGENT_SERVICE_NAME.c_str(), MonitorServiceHandler,
            NULL);
    }

private:
    WinServerStatus() {}
    ~WinServerStatus() {}
    SERVICE_STATUS_HANDLE hServiceStatus;
};

#endif

#endif
