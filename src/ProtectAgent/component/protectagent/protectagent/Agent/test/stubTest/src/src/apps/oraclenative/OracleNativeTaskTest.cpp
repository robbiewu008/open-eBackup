#include "apps/oraclenative/OracleNativeTaskTest.h"
using namespace std;
namespace {
mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

};


class MyOracleNativeTaskTest : public OracleNativeTask
{
public:
    MyOracleNativeTaskTest(const mp_string& taskID): OracleNativeTask(taskID) {}
    mp_void CreateTaskStep(){}
    mp_void RunTaskBefore(){}
    mp_void RunTaskAfter(){}
};

TEST_F(OracleNativeTaskTest, GetStatusFlagStub)
{
    mp_string taskID = "1";
    MyOracleNativeTaskTest task11(taskID);
    task11.GetStatusFlag();
}

TEST_F(OracleNativeTaskTest, RunTaskBeforeStub)
{
    mp_string taskID = "1";
    MyOracleNativeTaskTest task11(taskID);
    task11.RunTaskBefore();
}

TEST_F(OracleNativeTaskTest, RunTaskAfterStub)
{
    mp_string taskID = "1";
    MyOracleNativeTaskTest task11(taskID);
    task11.RunTaskAfter();
}


TEST_F(OracleNativeTaskTest, RunGetProgressTaskaStub)
{
    mp_string taskID = "1";
    MyOracleNativeTaskTest task11(taskID);
    mp_void* pThis = nullptr;
    task11.RunGetProgressTask(pThis);
    MyOracleNativeTaskTest task22(taskID);
    pThis = &task22;
    // task11.RunGetProgressTask(pThis);
}
