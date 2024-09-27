#ifndef WIN_SERVIICE_HEADER_H
#define WIN_SERVIICE_HEADER_H

#ifdef WIN32
#include "common/Types.h"

// Windows下句柄类单例
class WinService {
public:
    static WinService& GetInstace();
    SERVICE_STATUS_HANDLE& GetServiceHandle();
    mp_void InitServiceHandle();

private:
    WinService();
    ~WinService() {}
    SERVICE_STATUS_HANDLE hServiceStatus;
};
#endif

#endif
