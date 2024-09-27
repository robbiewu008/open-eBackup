#ifndef _AGENT_BACKUP_STEP_RMAN_TEST_H_
#define _AGENT_BACKUP_STEP_RMAN_TEST_H_

#define protected public
#define private public

#include "apps/oraclenative/TaskStepOracleNativeBackup.h"
#include "apps/oraclenative/TaskStepOracleNativeTest.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/Task.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepOracleNativeBackupTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        init_cunit_data();
    }

    static void TearDownTestCase(void)
    {
        destroy_cunit_data();
    }
    Stub stub;
protected:
    virtual void SetUp()
    {
        reset_cunit_counter();
    }

    virtual void TearDown()
    {
        ;
    }
};

static mp_int32 StubCJsonUtilsGetJsonString(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    strValue = "test";
    return MP_SUCCESS;
}

static mp_int32 StubCJsonUtilsGetJsonInt32(const Json::Value& jsValue, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 1;
    return MP_SUCCESS;
}

mp_int32 StubCIPCFileReadResult(const mp_string& strFileName, std::vector<mp_string>& vecRlt)
{
    vecRlt.push_back("test = 1");
    vecRlt.push_back("test2 = 2");
    return MP_SUCCESS;
}

mp_int32 StubRemoveParam(const mp_string& paramID)
{
    return MP_SUCCESS;
}

mp_int32 StubInitialDBInfo(const Json::Value& param)
{
    return MP_SUCCESS;
}

#endif
