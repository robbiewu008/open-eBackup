#ifndef AGENT_ORACLE_NATIVE_REDO_TASK_REGISTER
#define AGENT_ORACLE_NATIVE_REDO_TASK_REGISTER

#include "taskmanager/TaskRedoFuncContainer.h"
#include "apps/oraclenative/OracleNativeBackupTask.h"
#include "apps/oraclenative/OracleNativeExpireCopyTask.h"

class OracleNativeRedoTaskRegister {
public:
    OracleNativeRedoTaskRegister()
    {
        TaskRedoFuncContainer::GetInstance().RegisterNewFunc(
            "OracleNativeBackupTask", OracleNativeBackupTask::CreateRedoTask);
        TaskRedoFuncContainer::GetInstance().RegisterNewFunc(
            "OracleNativeExpireCopyTask", OracleNativeExpireCopyTask::CreateRedoTask);
    }
    ~OracleNativeRedoTaskRegister()
    {}
};

#endif