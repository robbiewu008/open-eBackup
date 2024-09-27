#include "apps/dws/XBSAServer/BsaMountManagerTest.h"
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

/*
* 测试用例：SetRepository和AddIntoMountedList接口测试
* 前置条件：无
* CHECK点：插入结果与预期相符.
*/
TEST_F(BsaMountManagerTest, AddIntoMountedList_Test)
{
    DoGetJsonStringTest();
    BsaMountManager mounter;

    DwsFsInfo fsInfo1;
    fsInfo1.id = "111";
    fsInfo1.name = "fs1_0";
    fsInfo1.mountPath.push_back("/tmp");

    DwsFsInfo fsInfo2;
    fsInfo2.id = "111";
    fsInfo2.name = "fs1_0";
    fsInfo2.mountPath.push_back("/tmp");
    fsInfo2.mountPath.push_back("/tmp2");

    std::vector<DwsRepository> repos;
    DwsRepository repo1;
    repo1.deviceSN = "esn1";
    repo1.filesystems.push_back(fsInfo1);
    repos.push_back(repo1);

    DwsRepository repo2;
    repo2.deviceSN = "esn2";
    repo2.filesystems.push_back(fsInfo2);
    repos.push_back(repo2);
    
    mp_string taskId = "1";
    mounter.SetRepository(taskId, repos);

    EXPECT_EQ(mounter.m_mountedFsList[taskId].size(), 2);
    for (auto &fs : mounter.m_mountedFsList[taskId]) {
        if (fs.deviceSN == repo1.deviceSN && fs.fsName == fsInfo1.name) {
            EXPECT_EQ(fs.mountPathList.size(), 1);
        }
        if (fs.deviceSN == repo2.deviceSN && fs.fsName == fsInfo2.name) {
            EXPECT_EQ(fs.mountPathList.size(), 2);
        }
    }

    EXPECT_EQ(mounter.IsFsMounted(taskId, "esn1", "111", "fs1_0"), true);
}

/*
* 测试用例：IsFsMounted接口测试
* 前置条件：无
* CHECK点：fsId，esn和fsName都相同的才是同一个文件系统.
*/
TEST_F(BsaMountManagerTest, IsFsMounted_Test)
{
    DoGetJsonStringTest();
    BsaMountManager mounter;
    mp_string taskId = "1";

    EXPECT_EQ(mounter.IsFsMounted(taskId, "esn1", "111", "fs1_0"), false);

    mp_string esn1 = "esn1";
    DwsFsInfo fsInfo1;
    fsInfo1.id = "111";
    fsInfo1.name = "fs1_0";
    fsInfo1.mountPath.push_back("/tmp");

    std::vector<FsMountInfo> output;
    mounter.AddIntoMountedList(taskId, esn1, fsInfo1, output);
    EXPECT_EQ(output.size(), 1);
}

/*
* 测试用例：AllocFilesystem接口测试
* 前置条件：文件系统已经挂载成功
* CHECK点：顺序分配文件系统.
*/
TEST_F(BsaMountManagerTest, AllocFilesystem_Test)
{
    DoGetJsonStringTest();
    BsaMountManager mounter;
    DwsFsInfo fsInfo1;
    fsInfo1.id = "111";
    fsInfo1.name = "fs1_0";
    fsInfo1.mountPath.push_back("/tmp");

    DwsFsInfo fsInfo2;
    fsInfo2.id = "222";
    fsInfo2.name = "fs1_1";
    fsInfo2.mountPath.push_back("/tmp");
    fsInfo2.mountPath.push_back("/tmp2");

    std::vector<DwsRepository> repos;
    DwsRepository repo1;
    repo1.deviceSN = "esn1";
    repo1.filesystems.push_back(fsInfo1);
    repos.push_back(repo1);

    DwsRepository repo2;
    repo2.deviceSN = "esn2";
    repo2.filesystems.push_back(fsInfo2);
    repos.push_back(repo2);
    
    mp_string taskId = "1";
    mounter.SetRepository(taskId, repos);

    mp_string deviceSN;
    mp_string fsId;
    mp_string fsName;
    mounter.AllocFilesystem(taskId, deviceSN, fsId, fsName);

    EXPECT_EQ(deviceSN == "esn2", true);
    EXPECT_EQ(fsId == "222", true);
    EXPECT_EQ(fsName == "fs1_1", true);

    mounter.AllocFilesystem(taskId, deviceSN, fsId, fsName);
    EXPECT_EQ(deviceSN == "esn1", true);
    EXPECT_EQ(fsId == "111", true);
    EXPECT_EQ(fsName == "fs1_0", true);

    mounter.AllocFilesystem(taskId, deviceSN, fsId, fsName);
    EXPECT_EQ(deviceSN == "esn2", true);
    EXPECT_EQ(fsId == "222", true);
    EXPECT_EQ(fsName == "fs1_1", true);
}

/*
* 测试用例：GetMountPath接口测试
* 前置条件：文件系统已经挂载成功
* CHECK点：同一个文件系统有多个挂载点时，顺序分配挂载点.
*/
TEST_F(BsaMountManagerTest, GetMountPath_Test)
{
    DoGetJsonStringTest();
    BsaMountManager mounter;
    DwsFsInfo fsInfo1;
    fsInfo1.id = "111";
    fsInfo1.name = "fs1_0";
    fsInfo1.mountPath.push_back("/tmp");

    DwsFsInfo fsInfo2;
    fsInfo2.id = "111";
    fsInfo2.name = "fs1_0";
    fsInfo2.mountPath.push_back("/tmp");
    fsInfo2.mountPath.push_back("/tmp2");

    std::vector<DwsRepository> repos;
    DwsRepository repo1;
    repo1.deviceSN = "esn1";
    repo1.filesystems.push_back(fsInfo1);
    repos.push_back(repo1);

    DwsRepository repo2;
    repo2.deviceSN = "esn2";
    repo2.filesystems.push_back(fsInfo2);
    repos.push_back(repo2);
    
    mp_string taskId = "1";
    mounter.SetRepository(taskId, repos);

    {
        mp_string ret1 = mounter.GetMountPath(taskId, "esn1", fsInfo1.name);
        EXPECT_EQ(ret1 == "/tmp", true);
        mp_string ret2 = mounter.GetMountPath(taskId, "esn1", fsInfo1.name);
        EXPECT_EQ(ret2 == "/tmp", true);
    }

    {
        mp_string ret1 = mounter.GetMountPath(taskId, "esn2", fsInfo2.name);
        mp_string ret2 = mounter.GetMountPath(taskId, "esn2", fsInfo2.name);
        EXPECT_EQ(ret2 != ret1, true);

        mp_string ret3 = mounter.GetMountPath(taskId, "esn2", fsInfo2.name);
        EXPECT_EQ(ret3 == ret1, true);

        mp_string ret4 = mounter.GetMountPath(taskId, "esn2", fsInfo2.name);
        EXPECT_EQ(ret4 == ret2, true);
    }
}