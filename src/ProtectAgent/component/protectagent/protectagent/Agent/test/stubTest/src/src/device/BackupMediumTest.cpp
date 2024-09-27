#include "device/BackupMediumTest.h"

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 StubExec(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

TEST_F(BackupMediumTest, CreateFsMedium)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), StubExec);
    BackupMedium work;
    work.mountPath = "mountPath";
    work.fsType = "fsType";
    work.vgName = "vgName";
    work.lvName = "lvName";
    EXPECT_EQ(MP_SUCCESS, work.CreateFsMedium("diskList"));
}