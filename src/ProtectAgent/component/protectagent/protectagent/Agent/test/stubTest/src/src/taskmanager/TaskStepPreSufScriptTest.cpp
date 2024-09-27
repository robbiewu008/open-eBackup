#include "taskmanager/TaskStepPreSufScript.h"
#include "taskmanager/TaskStepPreSufScriptTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(TaskStepPreScriptTest, TaskStepPreScriptTestInitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepPreScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
    param["preScript"] = "preScript";
    script.Init(param);
}

TEST_F(TaskStepPreScriptTest, TaskStepPostScriptInitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepPostScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
    param["postScript"] = "postScript";
    script.Init(param);
}

TEST_F(TaskStepPreScriptTest, TaskStepFailPostScriptInitTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepFailPostScript script(id, taskId, name, ratio, order);
    Json::Value param;
    script.Init(param);
    param["failPostScript"] = "failPostScript";
    script.Init(param);
}