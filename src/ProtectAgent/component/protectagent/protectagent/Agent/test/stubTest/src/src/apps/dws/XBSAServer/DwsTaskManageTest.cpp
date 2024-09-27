#include "apps/dws/XBSAServer/DwsTaskManageTest.h"
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

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

/*
* 测试用例：GenStorePath接口测试
* 前置条件：无
* CHECK点：组装出的路径中无连续的'/'字符
*/
TEST_F(DwsTaskManagerTest, GenStorePathTest)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;

    BsaObjInfo objInfo;
    objInfo.objectName = "/roach/abc";
    objInfo.copyId = 1;
    dwsTask.m_cacheInfo.hostKey = "127.1.1.1";
    dwsTask.GenStorePath(objInfo);
    mp_string storePath1 = objInfo.storePath;

    objInfo.objectName = "roach/abc";
    dwsTask.GenStorePath(objInfo);
    mp_string storePath2 = objInfo.storePath;

    EXPECT_TRUE(storePath1 == storePath2);
    EXPECT_TRUE(storePath1.find("//roach/abc") == std::string::npos);
}

/*
* 测试用例：ParseFsRelation接口测试
* 前置条件：无
* CHECK点：1.归档到云副本文件不存在时无需解析返回成功
           2.备份副本、复制副本，归档到磁带副本文件不存在时解析失败
*/
TEST_F(DwsTaskManagerTest, ParseFsRelation_FileNotExist)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;
    DwsFsRelation relation;
    {
        mp_uint32 copyTypes[] = {
            CopyDataType::type::CLOUD_STORAGE_COPY
        };
        for (auto iter : copyTypes) {
            dwsTask.m_taskInfo.copyType = iter;
            EXPECT_EQ(dwsTask.ParseFsRelation(relation), MP_SUCCESS);
        }
    }
    {
        mp_uint32 copyTypes[] = {
            CopyDataType::type::FULL_COPY, CopyDataType::type::INCREMENT_COPY,
            CopyDataType::type::DIFF_COPY, 
            CopyDataType::type::REPLICATION_COPY, CopyDataType::type::TAPE_STORAGE_COPY
        };
        for (auto iter : copyTypes) {
            dwsTask.m_taskInfo.copyType = iter;
            EXPECT_EQ(dwsTask.ParseFsRelation(relation), MP_FAILED);
        }
    }

    {
        dwsTask.m_taskInfo.copyType = CopyDataType::type::CLOUD_STORAGE_COPY;
        stub.set(&CMpFile::FileExist, StubTrue);
        stub.set(&DwsTaskInfoParser::ParseFsRelation, StubSuccess);
        EXPECT_EQ(dwsTask.ParseFsRelation(relation), MP_SUCCESS);
        stub.reset(&CMpFile::FileExist);
        stub.reset(&DwsTaskInfoParser::ParseFsRelation);
    }
}

/*
* 测试用例：获取关系
* 前置条件：无
* CHECK点：检查返回值
*/
TEST_F(DwsTaskManagerTest, GetFsRelation_Test)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;
    stub.set(&DwsTaskManage::ParseFsRelation, StubTrue);
    EXPECT_EQ(dwsTask.GetFsRelation(), MP_FAILED);
    stub.reset(&DwsTaskManage::ParseFsRelation);
}

/*
* 测试用例：获取Task信息
* 前置条件：无
* CHECK点：检查返回值
*/
TEST_F(DwsTaskManagerTest, UpdateTaskWhenQueryObject_Test)
{
    DoGetJsonStringTest();
    mp_string cache_path = "/tmp";
    mp_string app_type = "dws";
    DwsTaskManage dwsTask;
    BsaQueryDescriptor objDesc;

    stub.set(&DwsTaskInfoParser::ParseCacheInfo, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_FAILED);

    dwsTask.m_taskId = "test";
    stub.set(&DwsTaskInfoParser::ParseCacheInfo, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_FAILED);

    dwsTask.m_cacheInfo.taskId = "test";
    stub.set(&DwsTaskInfoParser::ParseCacheInfo, StubSuccess);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_SUCCESS);

    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskManage::IsBackupTask, StubTrue);
    stub.set(&DwsTaskInfoParser::ParseTaskInfo, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_FAILED);

    dwsTask.m_taskId = "test";
    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskInfoParser::ParseTaskInfo, StubSuccess);
    stub.set(&DwsTaskManage::CreateBsaDb, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_FAILED);

    dwsTask.m_taskId = "test";
    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskManage::CreateBsaDb, StubSuccess);
    stub.set(&DwsTaskManage::GetDwsHosts, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_FAILED);

    dwsTask.m_taskId = "test";
    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskManage::GetDwsHosts, StubSuccess);
    stub.set(&BsaMountManager::SetRepository, StubSuccess);
    stub.set(&DwsTaskManage::UpdateDwsHosts, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_SUCCESS);

    dwsTask.m_taskId = "test";
    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskManage::UpdateDwsHosts, StubSuccess);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_SUCCESS);

    dwsTask.m_taskId = "test";
    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskManage::IsRestoreTask, StubTrue);
    stub.set(&DwsTaskManage::GetFsRelation, StubFailed);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_FAILED);

    dwsTask.m_taskId = "test";
    dwsTask.m_cacheInfo.taskId = "test_new";
    stub.set(&DwsTaskManage::GetFsRelation, StubSuccess);
    dwsTask.m_taskInfo.copyType = CopyDataType::type::REPLICATION_COPY;
    dwsTask.m_taskInfo.taskType = 1;
    stub.set(&DwsTaskManage::GetFsRelation, StubSuccess);
    EXPECT_EQ(dwsTask.UpdateTaskWhenQueryObject(objDesc), MP_SUCCESS);
}

/*
* 测试用例：归档到云副本恢复获取archive服务器IP列表
* 前置条件：无
* CHECK点：
    1.ip列表格式:ip1,ip2,ip3
*/
TEST_F(DwsTaskManagerTest, GetArchiveServerIp_Test)
{
    DoGetJsonStringTest();
    ArchiveFileServerInfo fileServer;
    fileServer.ip = "192.168.1.1";
    fileServer.port = 30066;
    DwsTaskManage dwsTask;
    dwsTask.m_taskInfo.fileServers.push_back(fileServer);
    EXPECT_EQ(dwsTask.GetArchiveServerIp() == "192.168.1.1", true);

    fileServer.ip = "192.168.1.2";
    fileServer.port = 30066;
    dwsTask.m_taskInfo.fileServers.push_back(fileServer);
    EXPECT_EQ(dwsTask.GetArchiveServerIp() == "192.168.1.1,192.168.1.2", true);
}

/*
* 测试用例：归档到云副本恢复填充查询对象结果
* 前置条件：无
* CHECK点：
    1.有映射关系时,归档到云副本需要转换文件系统映射关系(复制归档副本)
*/
TEST_F(DwsTaskManagerTest, FillQuryRsp_CloudCopy)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;

    mp_long bsaHandle = 100;
    BsaObjInfo queryReslt;
    queryReslt.fsId = "123";
    queryReslt.fsName = "Database_fs_0";
    queryReslt.fsDeviceId = "esn11";
    QueryObjectResult rsp;

    FsKeyInfo oldFs;
    oldFs.fsId = "123";
    oldFs.fsName = "Database_fs_0";
    oldFs.fsDeviceId = "esn11";
    FsKeyInfo newFs;
    newFs.fsId = "12345";
    newFs.fsName = "rep_Database_fs_0";
    newFs.fsDeviceId = "esn22";

    dwsTask.m_fsRelationMap[oldFs] = newFs;

    dwsTask.m_taskInfo.copyType = CopyDataType::type::CLOUD_STORAGE_COPY;
    ArchiveFileServerInfo fileServer;
    fileServer.ip = "192.168.1.1";
    fileServer.port = 30066;
    dwsTask.m_taskInfo.fileServers.push_back(fileServer); 
    EXPECT_EQ(dwsTask.FillQuryRsp(bsaHandle, queryReslt, rsp), true);
    EXPECT_EQ(rsp.fsID == string(newFs.fsDeviceId + "_" + newFs.fsId), true);
}

mp_string GetMountPath_Stub_succ(const mp_string &deviceSN, const mp_string &fsName)
{
    return "123";
}

mp_string GetMountPath_Stub_fail(const mp_string &deviceSN, const mp_string &fsName)
{
    return "";
}

/*
* 测试用例：备份副本恢复填充查询对象结果
* 前置条件：无
* CHECK点：
    1.备份、复制、磁带归档副本必须转换文件系统，没有映射关系时，返回失败
    2.转换文件系统映射关系成功且获取挂载点成功，返回成功
*/
TEST_F(DwsTaskManagerTest, FillQuryRsp_FullCopy)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;

    mp_long bsaHandle = 100;
    BsaObjInfo queryReslt;
    queryReslt.fsId = "123";
    queryReslt.fsName = "Database_fs_0";
    queryReslt.fsDeviceId = "esn11";
    QueryObjectResult rsp;

    mp_uint32 copyTypes[] = {
        CopyDataType::type::FULL_COPY, CopyDataType::type::INCREMENT_COPY, CopyDataType::type::DIFF_COPY,
        CopyDataType::type::REPLICATION_COPY, CopyDataType::type::TAPE_STORAGE_COPY
    };
    for (auto iter : copyTypes) {
        dwsTask.m_fsRelationMap.clear();
        dwsTask.m_taskInfo.copyType = iter;
        EXPECT_EQ(dwsTask.FillQuryRsp(bsaHandle, queryReslt, rsp), false);

        FsKeyInfo oldFs;
        oldFs.fsId = "123";
        oldFs.fsName = "Database_fs_0";
        oldFs.fsDeviceId = "esn11";
        FsKeyInfo newFs;
        newFs.fsId = "12345";
        newFs.fsName = "rep_Database_fs_0";
        newFs.fsDeviceId = "esn22";

        dwsTask.m_fsRelationMap[oldFs] = newFs;
        stub.set(ADDR(BsaMountManager, GetMountPath), GetMountPath_Stub_fail);
        EXPECT_EQ(dwsTask.FillQuryRsp(bsaHandle, queryReslt, rsp), false);

        stub.set(ADDR(BsaMountManager, GetMountPath), GetMountPath_Stub_succ);
        EXPECT_EQ(dwsTask.FillQuryRsp(bsaHandle, queryReslt, rsp), true);
    }
}

mp_string GetDwsHostDbFilePath_Stub()
{
    cout << "GetDwsHostDbFilePath_Stub" << endl;
    return "/tmp/dwsHosts.db";
}

/*
* 测试用例：GetDwsHosts接口测试
* 前置条件：无
* CHECK点：
    1.查询数据库失败时，返回失败
    2.查询数据库成功时，返回成功
*/
TEST_F(DwsTaskManagerTest, GetDwsHosts_Test)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;

    EXPECT_EQ(dwsTask.GetDwsHosts(), MP_FAILED); // DB文件不存在时返回失败

    BsaDb db("/tmp/dwsHosts.db");
    system("touch /tmp/dwsHosts.db");
    EXPECT_EQ(db.CreateDwsHostFilesystemTable(), MP_SUCCESS); // 首次创建表成功
    DwsHostInfo hostInfo1("host1", "id1", "fs1", "esn1");
    EXPECT_EQ(db.InsertDwsHost(hostInfo1), MP_SUCCESS); // 插入host1成功
    DwsHostInfo hostInfo2("host2", "id2", "fs2", "esn1");
    EXPECT_EQ(db.InsertDwsHost(hostInfo2), MP_SUCCESS); // 插入host2成功

    stub.set(ADDR(DwsTaskManage, GetDwsHostDbFilePath), GetDwsHostDbFilePath_Stub);
    EXPECT_EQ(dwsTask.GetDwsHosts(), MP_SUCCESS); // 查询成功
    EXPECT_EQ(dwsTask.m_dwsHostMap.size() == 2, true); // 查询成功

    EXPECT_EQ(dwsTask.m_dwsHostMap["host1"].fsId == "id1", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host1"].fsName == "fs1", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host1"].fsDeviceId == "esn1", true);

    EXPECT_EQ(dwsTask.m_dwsHostMap["host2"].fsId == "id2", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host2"].fsName == "fs2", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host2"].fsDeviceId == "esn1", true);

    system("rm -f /tmp/dwsHosts.db"); // 清理资源
}

mp_bool IsFsMounted_Stub(const mp_string &deviceSN, const mp_string &fsId, const mp_string &fsName)
{
    return true;
}

/*
* 测试用例：UpdateDwsHosts接口测试
* 前置条件：无
* CHECK点：
    1.文件系统未挂载成功则删除记录
*/
TEST_F(DwsTaskManagerTest, UpdateDwsHosts_Test)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;

    FsKeyInfo fsInfo("id2", "fs2", "esn1");
    dwsTask.m_dwsHostMap["host1"] = fsInfo;
    dwsTask.UpdateDwsHosts();
    EXPECT_EQ(dwsTask.m_dwsHostMap.size() == 0, true);

    stub.set(ADDR(BsaMountManager, IsFsMounted), IsFsMounted_Stub);
    dwsTask.m_dwsHostMap["host1"] = fsInfo;
    dwsTask.UpdateDwsHosts();
    EXPECT_EQ(dwsTask.m_dwsHostMap.size() == 1, true);
}

/*
* 测试用例：ParseDwsHostname接口测试
* 前置条件：无
* CHECK点：
    1.按roach对象路径格式解析.rch文件对应的hostname
*/
TEST_F(DwsTaskManagerTest, ParseDwsHostname_Test)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;

    EXPECT_EQ(dwsTask.ParseDwsHostname("") == "", true);
    EXPECT_EQ(dwsTask.ParseDwsHostname("a.txt") == "", true);

    string fname = "/roach/20220506_172055/roach_master_updated.metadata";
    EXPECT_EQ(dwsTask.ParseDwsHostname(fname) == "", true);

    fname = "/roach/metadata/20220506_172055/gauss200-211/metadata.tar.gz";
    EXPECT_EQ(dwsTask.ParseDwsHostname(fname) == "", true);

    fname = "/roach/20220723_183709/gauss200-211/dn_6001/data_colstore/file_0.rch";
    EXPECT_EQ(dwsTask.ParseDwsHostname(fname) == "gauss200-211", true);

    fname = "/roach/20220809_120715_test_schema1.test_table2/dws-150/dn_6002/file_0.rch";
    EXPECT_EQ(dwsTask.ParseDwsHostname(fname) == "dws-150", true);
}

/*
* 测试用例：AllocFilesystem接口测试
* 前置条件：已挂载成功一个文件系统
* CHECK点：
    1.无法解析对象所属的主机时按顺序分配一个已挂载的文件系统
    2.解析对象所属的主机成功，按映射表中指定的关系分配文件系统
    3.解析对象所属的主机成功，但映射表中未指定的关系，按顺序分配文件系统
*/
TEST_F(DwsTaskManagerTest, AllocFilesystem_Test)
{
    DoGetJsonStringTest();
    DwsTaskManage dwsTask;
    // 存在一个挂载文件系统
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

    // 无法解析对象所属的主机时按顺序分配一个已挂载的文件系统
    BsaObjInfo objInfo;
    dwsTask.m_taskId = taskId;
    dwsTask.AllocFilesystem(objInfo);
    EXPECT_EQ(objInfo.fsDeviceId == "esn1", true);
    EXPECT_EQ(objInfo.fsId == "id1", true);
    EXPECT_EQ(objInfo.fsName == "fs1", true);

    // 解析对象所属的主机成功，按映射表中指定的关系分配文件系统
    FsKeyInfo fsInfo("id2", "fs2", "esn1");
    dwsTask.m_dwsHostMap["host1"] = fsInfo;
    objInfo.objectName = "/roach/20220723_183709/host1/dn_6001/data_colstore/file_0.rch";
    dwsTask.AllocFilesystem(objInfo);
    EXPECT_EQ(objInfo.fsDeviceId == "esn1", true);
    EXPECT_EQ(objInfo.fsId == "id2", true);
    EXPECT_EQ(objInfo.fsName == "fs2", true);

    // 解析对象所属的主机成功，但映射表中未指定的关系，按顺序分配文件系统
    objInfo.objectName = "/roach/20220723_183709/host2/dn_6001/data_colstore/file_0.rch";
    dwsTask.AllocFilesystem(objInfo);
    EXPECT_EQ(objInfo.fsDeviceId == "esn1", true);
    EXPECT_EQ(objInfo.fsId == "id1", true);
    EXPECT_EQ(objInfo.fsName == "fs1", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap.count("host2") > 0, true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host2"].fsId == "id1", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host2"].fsName == "fs1", true);
    EXPECT_EQ(dwsTask.m_dwsHostMap["host2"].fsDeviceId == "esn1", true);

    // 清理资源
    BsaMountManager::GetInstance().m_mountedFsList.clear();
    BsaMountManager::GetInstance().m_allocFsIndex.clear();
}

mp_string nullstr(){
    mp_string str;
    return str;
}

TEST_F(DwsTaskManagerTest, CreateBsaDb_Test)
{
    DwsTaskManage dwsTask;
    EXPECT_EQ(dwsTask.CreateBsaDb(), MP_FAILED);
    stub.set(&CMpFile::CreateFile, StubSuccess);
    EXPECT_EQ(dwsTask.CreateBsaDb(), MP_FAILED);
    stub.set(&BsaDb::CreateBsaObjTable, StubSuccess);
    EXPECT_EQ(dwsTask.CreateBsaDb(), MP_SUCCESS);
    stub.set(&DwsTaskManage::GetBsaDbFilePath, nullstr);
    EXPECT_EQ(dwsTask.CreateBsaDb(), MP_FAILED);
}