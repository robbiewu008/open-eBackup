#ifndef AGENT_ORACLE_NATIVE_TASK
#define AGENT_ORACLE_NATIVE_TASK

#include "common/Types.h"
#include "common/Uuid.h"
#include "common/CMpThread.h"
#include "taskmanager/Task.h"

class OracleNativeTask : public Task {
public:
    OracleNativeTask(const mp_string& taskID);
    virtual ~OracleNativeTask();
    mp_bool GetStatusFlag();
    int GetProgressInterval()
    {
        return m_progressInterval;
    }

protected:
    thread_id_t statusTid;
    mp_bool statusFlag;
    int m_progressInterval;

    mp_void RunTaskBefore();
    mp_void RunTaskAfter();

#ifdef WIN32
    static DWORD WINAPI RunGetProgressTask(mp_void* pThis);
#else
    static mp_void* RunGetProgressTask(mp_void* pThis);
#endif
};

#endif
