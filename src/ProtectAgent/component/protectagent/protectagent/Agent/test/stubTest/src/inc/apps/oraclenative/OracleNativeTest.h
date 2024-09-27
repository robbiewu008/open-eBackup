#ifndef __ORACLETEST_H__
#define __ORACLETEST_H__

#define private public
#define protected public
#include "common/JsonUtils.h"
#include "cunitpub/publicInc.h"
#include "apps/oraclenative/OracleNativeBackup.h"
#include "securecom/RootCaller.h"
#include "message/tcp/CDppMessage.h"

using namespace std;

static mp_int32 StubCRootCallerExecGetDBType1(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return 0;
}

static mp_int32 StubCRootCallerExecGetDBType2(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    pvecResult->push_back("3");
    return 0;
}

static mp_int32 StubOracleNativeBackupReturnSuccess(void)
{
    return MP_SUCCESS;
}

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

static mp_int32 StubGetTaskidByReq(const mp_string& msgBody, mp_string& taskid)
{
    taskid = "test";
    return MP_SUCCESS;
}


class COracleNativeTest : public testing::Test {
public:
    static void SetUpTestCase(void)
    {
        init_cunit_data();
    }

    static void TearDownTestCase(void)
    {
        destroy_cunit_data();
    }

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

class COracleNativeBackupTest : public testing::Test {
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


#endif
