#include "apps/dws/XBSAServer/InformixTaskManageTest.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParser.h"
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "apps/dws/XBSAServer/BsaDb.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "securecom/RootCaller.h"

namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)
}

mp_bool IifStubTrue()
{
    return MP_TRUE;
}

mp_bool IifStubFalse()
{
    return MP_FALSE;
}

mp_int32 IifStubFailed()
{
    return MP_FAILED;
}

mp_int32 IifStubSuccess()
{
    return MP_SUCCESS;
}

/*
* 测试用例：创建备份文件接口测试
* 前置条件：无
* CHECK点：
*/
TEST_F(InformixTaskManagerTest, UpdateTaskWhenCreateObject)
{
    DoGetJsonStringTest();
    InformixTaskManage iifTask;
    BsaObjectDescriptor objDesc;
    stub.set(&InformixTaskManage::GetInstanceName, IifStubFailed);
    stub.set(&DwsTaskInfoParser::ParseCacheInfo, IifStubFailed);
    mp_int32 iRet = iifTask.UpdateTaskWhenCreateObject(objDesc);
    EXPECT_EQ(iRet, -1);

}

/*
* 测试用例：查询副本文件接口测试
* 前置条件：无
* CHECK点：
*/
TEST_F(InformixTaskManagerTest, UpdateTaskWhenQueryObject)
{
    DoGetJsonStringTest();
    InformixTaskManage iifTask;
    BsaQueryDescriptor objDesc;
    objDesc.objectName.pathName = "/orcl";
    mp_int32 iRet = iifTask.UpdateTaskWhenQueryObject(objDesc);
    EXPECT_EQ(iRet, -1);
}

/*
* 测试用例：分配文件系统
* 前置条件：无
* CHECK点：检查返回值
*/
TEST_F(InformixTaskManagerTest, AllocFilesystem)
{
    DoGetJsonStringTest();
    DwsFsInfo fs;
    fs.id = "id1";
    fs.name = "fs1";
    fs.mountPath.push_back("/tmp/123");
    std::vector<DwsRepository> repos;
    DwsRepository repo1;
    repo1.deviceSN = "esn1";
    repo1.filesystems.push_back(fs);
    repos.push_back(repo1);

    mp_string taskId = "1";
    BsaMountManager::GetInstance().SetRepository(taskId, repos);

    mp_string deviceSN;
    mp_string fsId;
    mp_string fsName;

    InformixTaskManage iifTask;
    BsaObjInfo objInfo;
    iifTask.m_taskId = taskId;
    iifTask.AllocFilesystem(objInfo);
    EXPECT_EQ(objInfo.fsDeviceId == "esn1", true);
    EXPECT_EQ(objInfo.fsId == "id1", true);
    EXPECT_EQ(objInfo.fsName == "fs1", true);
}

/*
* 测试用例：获取Task信息
* 前置条件：无
* CHECK点：检查返回值
*/
TEST_F(InformixTaskManagerTest, GetTaskInfoLockedInner)
{
    DoGetJsonStringTest();
    InformixTaskManage iifTask;
    stub.set(&DwsTaskInfoParser::ParseTaskInfo, IifStubFailed);
    EXPECT_EQ(iifTask.GetTaskInfoLockedInner(), MP_FAILED);

    stub.set(&DwsTaskInfoParser::ParseTaskInfo, IifStubSuccess);
    stub.set(&InformixTaskManage::IsBackupTask, IifStubTrue);
    stub.set(&InformixTaskManage::CreateBsaDb, IifStubFailed);
    EXPECT_EQ(iifTask.GetTaskInfoLockedInner(), MP_FAILED);

    stub.set(&InformixTaskManage::IsBackupTask, IifStubFalse);
    stub.set(&BsaMountManager::SetRepository, IifStubSuccess);
    EXPECT_EQ(iifTask.GetTaskInfoLockedInner(), MP_SUCCESS);
}

/*
* 测试用例：获取数据库实例名
* 前置条件：无
* CHECK点：
*/
TEST_F(InformixTaskManagerTest, GetInstanceName)
{
    DoGetJsonStringTest();
    InformixTaskManage iifTask;
    mp_string instanceName;
    iifTask.GetInstanceName("/orcl/rootdbs/0", instanceName);
    EXPECT_EQ(instanceName == "orcl", true);
    std::cout<<"instanceName: "<<instanceName <<"\n";

    instanceName = "";
    iifTask.GetInstanceName("orcl", instanceName);
    std::cout<<"instanceName: "<<instanceName <<"\n";
    EXPECT_EQ(instanceName == "", true);
}
