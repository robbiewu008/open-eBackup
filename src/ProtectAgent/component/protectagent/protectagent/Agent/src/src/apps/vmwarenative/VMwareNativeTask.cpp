#include <algorithm>
#include <vector>
#include "apps/vmwarenative/VMwareNativeTask.h"

#include "common/Utils.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/MessageHandler.h"
#include "taskmanager/TaskContext.h"

using namespace std;

VMwareNativeTask::VMwareNativeTask(const mp_string& taskID) : Task(taskID)
{
    m_statusFlag = MP_FALSE;
}

VMwareNativeTask::~VMwareNativeTask()
{
    m_statusFlag = MP_FALSE;
}

mp_void VMwareNativeTask::RunTaskBefore()
{}

mp_void VMwareNativeTask::RunTaskAfter()
{}

#ifdef WIN32
DWORD WINAPI VMwareNativeTask::RunGetProgressTask(mp_void* pThis)
#else
mp_void* VMwareNativeTask::RunGetProgressTask(mp_void* pThis)
#endif
{
#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

mp_bool VMwareNativeTask::GetStatusFlag()
{
    return m_statusFlag;
}

mp_int32 VMwareNativeTask::ReportTaskStatus(VMwareNativeTask* task)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}
