#include "taskmanager/TaskStepScanDisk.h"
#include "taskmanager/TaskStepScanDiskTest.h"
#include "host/host.h"
#include "common/Log.h"
namespace {
mp_int32 StubCConfigXmlParserGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32Return); \
} while (0)

mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)
}

mp_int32 ScanDiskTest()
{
    return MP_FAILED;
}

mp_int32 ScanDiskSuccTest()
{
    return MP_SUCCESS;
}

TEST_F(TaskStepScanDiskTest, InitTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    Json::Value param;

    DoGetJsonStringTest();
    EXPECT_EQ(MP_SUCCESS, disk.Init(param));
    EXPECT_EQ(MP_SUCCESS, disk.Init(param));
}

TEST_F(TaskStepScanDiskTest, RunTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    DoGetJsonStringTest();
    stub.set(ADDR(CHost, ScanDisk), ScanDiskTest);
    EXPECT_EQ(MP_FAILED, disk.Run());
    stub.set(ADDR(CHost, ScanDisk), ScanDiskSuccTest);
    EXPECT_EQ(MP_SUCCESS, disk.Run());
}

TEST_F(TaskStepScanDiskTest, CancelTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    DoGetJsonStringTest();
    EXPECT_EQ(MP_SUCCESS, disk.Cancel());
}

TEST_F(TaskStepScanDiskTest, StopTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    DoGetJsonStringTest();
    Json::Value param;
    EXPECT_EQ(MP_SUCCESS, disk.Stop(param));
}

TEST_F(TaskStepScanDiskTest, RedoTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    DoGetJsonStringTest();
    mp_string param;
    EXPECT_EQ(MP_SUCCESS, disk.Redo(param));
}

TEST_F(TaskStepScanDiskTest, UpdateTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    DoGetJsonStringTest();
    Json::Value param;
    EXPECT_EQ(MP_SUCCESS, disk.Update(param));
}

TEST_F(TaskStepScanDiskTest, FinishTest)
{
    mp_string id;
    mp_string taskId;
    const mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepScanDisk disk(id, taskId, name, ratio, order);
    DoGetJsonStringTest();
    Json::Value param;
    EXPECT_EQ(MP_SUCCESS, disk.Finish(param));
}