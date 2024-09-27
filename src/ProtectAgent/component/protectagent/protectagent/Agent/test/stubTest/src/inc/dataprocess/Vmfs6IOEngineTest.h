#ifndef __VMFS_IO_ENGINE_TEST_H__
#define __VMFS_IO_ENGINE_TEST_H__

#define private public

#include "dataprocess/ioscheduler/Vmfs6IOEngine.h"
#include "common/ConfigXmlParse.h"
#include "common/File.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"

class vmfs6IOEngineTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

public:
    Stub stub;
};

void vmfs6IOEngineTest::SetUp()
{}

void vmfs6IOEngineTest::TearDown()
{}

void vmfs6IOEngineTest::SetUpTestCase()
{}

void vmfs6IOEngineTest::TearDownTestCase()
{}

VmfsFsT StubReturnVmfsFsT()
{
    VmfsFsT* vft;
    vft->debugLevel = 0;
    vft->fs_info = NULL;
    vft->dev = NULL;
    return vft;
}

VmfsDirT StubReturnVmfsDirT()
{
    VmfsDirT* vdt;
    vdt->pos = 0;
    vdt->dir = NULL;
    vdt->dirent = NULL;
    return vdt;
}

VmfsDirentT StubReturnVmfsDirentT()
{
    VmfsDirentT* vdt;
    vdt->type = 0;
    vdt->blockId = 0;
    vdt->recordId = 0;
    return vdt;
}

mp_int32 StubSetDiskPathFail()
{
    return MP_FAILED;
}

mp_int32 StubSetDiskPathSucc()
{
    return MP_SUCCESS;
}
mp_int32 StubVmfsIOLookupFail()
{
    return MP_FAILED;
}

mp_int32 StubVmfsIOLookupSucc()
{
    return MP_SUCCESS;
}

#endif