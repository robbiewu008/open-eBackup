#include "apps/xbsa/DataConversionTest.h"
#include "common/Types.h";

mp_int32 entryCount;
namespace {
mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

mp_int32 StubFailedOnTwo()
{
    if (entryCount++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_int32 StubFailedOnThree()
{
    if (entryCount++ < 2) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_int32 StubFailedOnFour()
{
    if (entryCount++ < 3) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_int32 StubFailedOnFive()
{
    if (entryCount++ < 4) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_int32 StubFailedOnSix()
{
    if (entryCount++ < 5) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_int32 StubFailedOnSeven()
{
    if (entryCount++ < 6) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
mp_void ReturnNull()
{

}
}

TEST_F(DataConversionTest, CopyStrToCharTest)
{
    std::string src = "Agent";
    char dst[100];
    uint32_t dstSize = 100;

    DataConversion::CopyStrToChar(src, dst, dstSize);
    EXPECT_STREQ(src.c_str(), dst);
}

TEST_F(DataConversionTest, ConvertStrToTimeTest)
{
    std::string src = "1490663873";
    struct tm dst;
    DataConversion::ConvertStrToTime(src, dst);

    src = "";
    stub.set(gmtime_r, ReturnNull);
    DataConversion::ConvertStrToTime(src, dst);
    stub.reset(gmtime_r);
}

TEST_F(DataConversionTest, ConvertObjectDescriptorInTest)
{
    BSA_ObjectDescriptor src;
    BsaObjectDescriptor dst;
    DataConversion::ConvertObjectDescriptorIn(&src, dst);
}

TEST_F(DataConversionTest, ConvertObjectDescriptorOutTest)
{
    BsaObjectDescriptor src;
    BSA_ObjectDescriptor dst;
    entryCount = 0;
    mp_int32 iRet;
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_TRUE, iRet);

    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailed);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailedOnTwo);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
    entryCount = 0;
    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailedOnThree);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
    entryCount = 0;
    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailedOnFour);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
    entryCount = 0;
    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailedOnFive);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
    entryCount = 0;
    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailedOnSix);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
    entryCount = 0;
    stub.set(ADDR(DataConversion, CopyStrToChar), StubFailedOnSeven);
    iRet = DataConversion::ConvertObjectDescriptorOut(src, &dst);
    EXPECT_EQ(MP_FALSE, iRet);
}

TEST_F(DataConversionTest, ConvertdataBlockOutTest)
{
    BsaDataBlock32 src;
    BSA_DataBlock32 dst;
    DataConversion::ConvertdataBlockOut(src, &dst);
}

TEST_F(DataConversionTest, ConvertQueryObjectInTest)
{
    BSA_QueryDescriptor src;
    BsaQueryDescriptor dst;
    DataConversion::ConvertQueryObjectIn(&src, dst);
}

TEST_F(DataConversionTest, ConvertdataBlockInTest)
{
    BSA_DataBlock32 src;
    BsaDataBlock32 dst;
    DataConversion::ConvertdataBlockIn(&src, dst);
}

TEST_F(DataConversionTest, U64ToBsaU64Test)
{
    unsigned long long u64;
    BsaUInt64 b64;
    DataConversion::U64ToBsaU64(u64, b64);
}
