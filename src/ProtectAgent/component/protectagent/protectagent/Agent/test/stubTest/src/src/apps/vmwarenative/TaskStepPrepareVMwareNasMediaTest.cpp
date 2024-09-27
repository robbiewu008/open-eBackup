#include "apps/vmwarenative/TaskStepPrepareVMwareNasMediaTest.h"
#include "apps/vmwarenative/TaskStepPrepareVMwareNasMedia.h"
#include "host/host.h"
#include "gtest/gtest.h"
#include "stub.h"

// Run interface testing
mp_int32 MountNasMediaTest(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst)
{
    vecRst.push_back("/home1/");
    vecRst.push_back("/home2/");
    vecRst.push_back("/home3/");
    return MP_SUCCESS;
}

mp_int32 MountNasMediaTest_FAILED(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst)
{
    return MP_FAILED;
}

mp_int32 MountDataturboMediaTest(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst, DataturboMountParam &param)
{
    vecRst.push_back("/home1/");
    vecRst.push_back("/home2/");
    vecRst.push_back("/home3/");
    return MP_SUCCESS;    
}

mp_int32 MountDataturboMediaTest_FAILED(mp_void* pThis, const mp_string& scriptParam, vector<mp_string>& vecRst, DataturboMountParam &param)
{
    return MP_FAILED;    
}

mp_int32 GetHostSN_Stub(mp_string& strSN)
{
    strSN="12345";
    return MP_SUCCESS;
}

/*
* 用例描述：检验挂载文件系统流程
* 观测点：(1) Dataturbo挂载成功，NFS挂载成功，返回成功
*        (2) Dataturbo挂载失败，NFS挂载成功，返回成功
*        (2) Dataturbo挂载失败，NFS挂载失败，返回失败
*/
TEST_F(TaskStepPrepareVMwareNasMediaTest, RUN_success)
{
    DoGetJsonStringTest();

    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;

    TaskStepPrepareVMwareNasMedia vmwaremedia(id, taskId, name, ratio , order);
    vmwaremedia.Init(param);

    stub.set(ADDR(CHost, GetHostSN), GetHostSN_Stub);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia),  MountNasMediaTest_FAILED);
    mp_int32 iRet = vmwaremedia.Run();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia), MountDataturboMediaTest);
    iRet = vmwaremedia.Run();
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest_FAILED);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia), MountDataturboMediaTest);
    iRet = vmwaremedia.Run();
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia), MountDataturboMediaTest_FAILED);
    stub.set(ADDR(TaskStepPrepareNasMedia, MountNasMedia),  MountNasMediaTest_FAILED);
    iRet = vmwaremedia.Run();
    EXPECT_EQ(MP_FAILED, iRet); 

    stub.reset(ADDR(CHost, GetHostSN));
    stub.reset(ADDR(TaskStepPrepareNasMedia, MountDataturboMedia));
    stub.reset(ADDR(TaskStepPrepareNasMedia, MountNasMedia));
}
