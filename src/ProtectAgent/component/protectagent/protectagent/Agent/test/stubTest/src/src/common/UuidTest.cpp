#include "common/UuidTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(CUuidNumTest,GetUuidNumber){
    mp_string strUuid;
    stub.set(&CLogger::Log, StubCLoggerLog);
    CUuidNum::GetUuidNumber(strUuid);
    strUuid = "aabb";
    CUuidNum::GetUuidNumber(strUuid);
}

TEST_F(CUuidNumTest,GetUuidStr)
{
    CUuidNum work;
    mp_int32 iRet;
    mp_string uuid = "test";
    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.GetUuidStr(uuid);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CUuidNumTest,GetUuidStandardStr)
{
    CUuidNum work;
    mp_int32 iRet;
    mp_string uuid = "test";
    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.GetUuidStandardStr(uuid);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CUuidNumTest,CovertStrToUuid)
{
    CUuidNum work;
    mp_int32 iRet;
    mp_string uuidStr = "test";
    mp_uuid uuid;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.CovertStrToUuid(uuidStr, uuid);
    EXPECT_EQ(iRet, MP_FAILED);
    uuidStr = "12345678901234567890123456789011";
    iRet = work.CovertStrToUuid(uuidStr, uuid);
}

TEST_F(CUuidNumTest,ConvertUuidToStr)
{
    CUuidNum work;
    mp_int32 iRet;
    mp_string uuidStr;
    mp_uuid uuid;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.ConvertUuidToStr(uuid, uuidStr);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CUuidNumTest,CovertStandrdStrToUuid)
{
    CUuidNum work;
    mp_int32 iRet;
    mp_string uuidStr = "test";
    mp_uuid uuid;

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.CovertStandrdStrToUuid(uuidStr, uuid);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CUuidNumTest,ConvertStrUUIToArray)
{
    CUuidNum work;
    mp_int32 iRet;
    mp_string uuidStr = "test";
    mp_char pszCharArray[] = "test";

    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = work.ConvertStrUUIToArray(uuidStr, pszCharArray, sizeof(pszCharArray));
    EXPECT_EQ(iRet, MP_FAILED);
}