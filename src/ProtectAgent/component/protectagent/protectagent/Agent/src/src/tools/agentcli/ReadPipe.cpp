#include "tools/agentcli/ReadPipe.h"
#include "common/Pipe.h"
#include "common/CMpThread.h"
#include "common/Log.h"
#include "common/CMpTime.h"

namespace {
    const mp_int32 TIME_OUT = 100;
    const mp_int32 TIME_OUT_UNIT = 10;
}

mp_int32 ReadPipe::Handle(const mp_string& strPipePath, const mp_string& strTimeOut)
{
    LOGGUARD("");
    m_pipePath = strPipePath;
    mp_int32 nTimeoutNum = atoi(strTimeOut.c_str());
    if (nTimeoutNum <= 0 || m_pipePath.empty()) {
        ERRLOG("error param, param1=%s, param2=%s.", strPipePath.c_str(), strTimeOut.c_str());
        return MP_FAILED;
    }

    thread_id_t thId;
    (mp_void)memset_s(&thId, sizeof(thId), 0, sizeof(thId));
    mp_int32 iRet = CMpThread::Create(&thId, ReadParamFunc, this);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create write thread failed, ret %d.", iRet);
        return MP_FAILED;
    }

    mp_int32 nIndex = 0;
    while (nIndex++ < nTimeoutNum * TIME_OUT_UNIT) {
        if (m_bRead.load()) {
            CMpThread::WaitForEnd(&thId, NULL);

            for (mp_string str : m_vecOutput) {
                printf("%s", str.c_str());
            }
            return MP_SUCCESS;
        }
        CMpTime::DoSleep(TIME_OUT);
    }

    ERRLOG("ReadPipe timeout.");
    {
        // 这里随意写点数据到命名管道，让线程退出
        CMpPipe pipeHandler;
        m_vecOutput.push_back(m_pipePath);
        if (pipeHandler.WritePipe(m_pipePath, m_vecOutput) == MP_SUCCESS) {
            CMpThread::WaitForEnd(&thId, NULL);
        }
    }
    return MP_FAILED;
}

mp_string ReadPipe::GetPipePath()
{
    return m_pipePath;
}

void ReadPipe::SetResult(const std::vector<mp_string>& vecOutput)
{
    m_vecOutput = vecOutput;
    m_bRead.store(true);
}

#ifdef WIN32
DWORD WINAPI ReadPipe::ReadParamFunc(void* pHandle)
#else
mp_void* ReadPipe::ReadParamFunc(void* pHandle)
#endif
{
    LOGGUARD("");
    ReadPipe* pThis = static_cast<ReadPipe*>(pHandle);
    if (!pThis) {
        COMMLOG(OS_LOG_ERROR, "cast to ReadPipe pointer failed.");
#ifdef WIN32
        return 0;
#else
        return NULL;
#endif
    }

    CMpPipe pipeHandler;
    std::vector<mp_string> vecOutput;
    pipeHandler.ReadPipe(pThis->GetPipePath(), vecOutput);
    pThis->SetResult(vecOutput);

#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}