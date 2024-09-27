#include "dataprocess/DiskStreamTest.h"
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubDiskStreamGetValueInt32Return); \
} while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}
TEST_F(DiskStreamTest, StreamWrite) {
    mp_void *ctx;
    mp_char buff[] = "abc";
    mp_int32 iBuffLen = 3;
    DiskStream om;
    mp_int32 iRet = 0;

    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DataContext, GetDiskFdByName), StubGetDiskFdByName);
    iRet = om.StreamWrite(ctx, buff, iBuffLen);
    EXPECT_EQ(MP_SUCCESS, iRet);
    stub.reset(ADDR(DataContext, GetDiskFdByName));
}

TEST_F(DiskStreamTest, StreamRead) {
    mp_void *ctx;
    mp_char buff[] = "abc";
    mp_int32 iBuffLen = 3;
    mp_uint32 uiRecvLen = 0;
    DiskStream om;
    mp_int32 iRet = 0;

    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DataContext, GetDiskFdByName), StubGetDiskFdByName);
    iRet = om.StreamRead(ctx, buff, iBuffLen, uiRecvLen);
    EXPECT_EQ(MP_SUCCESS, iRet);
    stub.reset(ADDR(DataContext, GetDiskFdByName));
}