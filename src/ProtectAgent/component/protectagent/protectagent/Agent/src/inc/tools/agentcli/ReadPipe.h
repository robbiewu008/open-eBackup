#ifndef _AGENTCLI_READPIPE_H_
#define _AGENTCLI_READPIPE_H_

#include <atomic>
#include <vector>
#include "common/Types.h"

class ReadPipe {
public:
    ReadPipe()
    {
        std::atomic_init(&m_bRead, false);
    }
    ~ReadPipe()
    {
    }
    mp_int32 Handle(const mp_string& strPipePath, const mp_string& strTimeOut);
    mp_string GetPipePath();
    void SetResult(const std::vector<mp_string>& vecOutput);

private:
#ifdef WIN32
    static DWORD WINAPI ReadParamFunc(void* pHandle);
#else
    static mp_void* ReadParamFunc(void* pHandle);
#endif

    std::atomic<bool> m_bRead;
    std::vector<mp_string> m_vecOutput;
    mp_string m_pipePath;
};
#endif