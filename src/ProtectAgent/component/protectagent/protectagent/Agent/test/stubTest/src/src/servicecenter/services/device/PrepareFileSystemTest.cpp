#include "servicecenter/services/device/PrepareFileSystemTest.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "common/ConfigXmlParse.h"
#include "securecom/RootCaller.h"
#include "message/tcp/CSocket.h"
#include "host/host.h"
#include "common/Log.h"
#include <vector>

using namespace std;
using namespace AppProtect;

namespace {
mp_int32 flag = 0;

mp_int32 StubGetValueStringFailedTwo(mp_string strSection, mp_string strKey, mp_string& strValue)
{
    if (flag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubGetInstallScene(mp_void* pThis, mp_string& strSceneType)
{
    strSceneType = "0";
}

/*
* 用例名称：挂载NAS文件系统
* 前置条件：1、无
* check点：1、挂载成功，返回MP_SUCCESS，检查返回值及挂载点成功个数
           2、获取挂载参数失败，返回MP_FAILED，检查返回值
           3、检查IP连通性失败，返回MP_FAILED，检查返回值
           4、脚本执行失败，返回MP_FAILED，检查返回值
*/
TEST_F(PrepareFileSystemTest, MountNasFileSystem)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_string> vecStorageIp;
    vector<mp_string> vecDataturboIP;
    mp_string storageFs = "/202101041911";
    vecStorageIp.push_back("8.40.99.244");
    vecStorageIp.push_back("8.40.99.245");
    MountNasParam mountNasParam = {"11111", "data", vecStorageIp, vecDataturboIP, storageFs, "vmware_type"};
    mountNasParam.isFullPath = true;
    vector<mp_string> successMountPath;
    std::set<mp_string> availStorageIp;

    mp_int32 ret;
    PrepareFileSystem om;

    // 执行挂载成功
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubPrepareFileSystemGetValueStringSuccess);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);

    
    ret = om.MountNasFileSystem(mountNasParam, successMountPath);
    EXPECT_EQ(ret, MP_SUCCESS);
    successMountPath.clear();

    // 获取挂载参数失败
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubPrepareFileSystemGetValueStringFail);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    ret = om.MountNasFileSystem(mountNasParam, successMountPath);
    EXPECT_EQ(ret, MP_FAILED);
    EXPECT_EQ(successMountPath.size(), 0);
    successMountPath.clear();

    // 脚本执行失败 
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubPrepareFileSystemGetValueStringSuccess);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    EXPECT_EQ(MP_FAILED, om.MountNasFileSystem(mountNasParam, successMountPath));
    EXPECT_EQ(0, successMountPath.size());
}

/*
* 用例名称：挂载Sanclient文件系统
* 前置条件：1、无
* check点：1、挂载成功，返回MP_SUCCESS，检查返回值及挂载点成功个数
           2、获取挂载参数失败，返回MP_FAILED，检查返回值
           3、脚本执行失败，返回MP_FAILED，检查返回值
*/
TEST_F(PrepareFileSystemTest, MountFileIoSystem)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_string> vecStorageIp;
    vector<mp_string> vecDataturboIP;
    mp_string storageFs = "/202101041911";
    vecStorageIp.push_back("8.40.99.244");
    vecStorageIp.push_back("8.40.99.245");
    MountNasParam mountNasParam = {"11111", "data", vecStorageIp, vecDataturboIP, storageFs, "vmware_type"};
    mountNasParam.isFullPath = true;
    mountNasParam.protocolType = "fc";
    mountNasParam.lunInfo = "a4813:21000024ff2f4190/150,meta,32767//";
    vector<mp_string> successMountPath;
    std::set<mp_string> availStorageIp;

    mp_int32 ret;
    PrepareFileSystem om;
    // 脚本执行失败
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    ret = om.MountFileIoSystem(mountNasParam, successMountPath,"test_mounted.lock");
    EXPECT_EQ(ret, MP_FAILED);
    EXPECT_EQ(successMountPath.size(), 0);
    successMountPath.clear();

}

/*
* 用例名称：检查或创建Dataturbo链接
* 前置条件：1、无
* check点：DataturboIp为空
*/
TEST_F(PrepareFileSystemTest, CheckAndCreateDataturboLink)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string storageName;
    MountNasParam mountNasParam;

    const mp_int32 ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT = 0x5E025C02;
    PrepareFileSystem om;
    mp_int32 iRet = om.CheckAndCreateDataturboLink(storageName, mountNasParam);
    EXPECT_EQ(iRet, ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT);
}

/*
* 用例名称：检查或创建Dataturbo链接，通过FC建链
* 前置条件：1、无
* check点：返回成功
*/
TEST_F(PrepareFileSystemTest, CheckAndCreateDataturboLinkByFC)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string storageName;
    MountNasParam mountNasParam;
    mountNasParam.isFcOn = MP_TRUE;

    const mp_int32 ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT = 0x5E025C02;
    PrepareFileSystem om;
    mp_int32 iRet = om.CheckAndCreateDataturboLink(storageName, mountNasParam);
    EXPECT_EQ(iRet, ERR_CREATE_DATA_TURBO_LINK);
}

/*
* 用例名称：挂载NAS文件系统
* 前置条件：1、无
* check点：1、一个文件系统多个IP，部分成功，返回成功
           1、一个文件系统多个IP，全部失败，返回失败
*/
TEST_F(PrepareFileSystemTest, MountNasFileSystemParitalFailed)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_string> vecStorageIp;
    vector<mp_string> vecDataturboIP;
    mp_string storageFs = "/202101041911";
    vecStorageIp.push_back("8.40.99.244");
    vecStorageIp.push_back("8.40.99.245");
    MountNasParam mountNasParam = {"11111", "data", vecStorageIp, vecDataturboIP, storageFs, "vmware_type"};
    mountNasParam.isFullPath = true;
    vector<mp_string> successMountPath;
    std::set<mp_string> availStorageIp;

    mp_int32 ret;
    PrepareFileSystem om;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubPrepareFileSystemGetValueStringSuccess);
    execNum = 0;

    // 一个文件系统多个IP部分IP成功，返回成功
    stub.set(ADDR(CRootCaller, Exec), StubExecTest1fstSuccessOtherFailed);
    ret = om.MountNasFileSystem(mountNasParam, successMountPath);
    EXPECT_EQ(ret, MP_SUCCESS);

    successMountPath.clear();
    // 一个文件系统多个IP所有IP失败，返回失败
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    ret = om.MountNasFileSystem(mountNasParam, successMountPath);
    EXPECT_EQ(ret, MP_FAILED);
    EXPECT_EQ(successMountPath.size(), 0);
}

/*
* 用例名称：卸载NAS文件系统
* 前置条件：1、无
* check点：1、卸载成功，返回MP_SUCCESS，检查返回值
           2、卸载失败，返回MP_FAILED，检查返回值
*/
TEST_F(PrepareFileSystemTest, UmountNasFileSystem)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_string> successMountPath;
    successMountPath.push_back("/tmp/huawei/databackup/dws/111111/meta/202111041910/8.42.99.244");
    successMountPath.push_back("/tmp/huawei/databackup/dws/111111/meta/202111041911/8.40.99.245");

    mp_int32 ret;
    PrepareFileSystem om;

    // 脚本执行成功
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    ret = om.UmountNasFileSystem(successMountPath);
    EXPECT_EQ(ret, MP_SUCCESS);

    // 脚本执行失败, 无错误挂载路径
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    ret = om.UmountNasFileSystem(successMountPath);
    EXPECT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：本地创建文件夹
* 前置条件：1、无
* check点：1、创建文件夹成功， 返回MP_SUCCESS
*          2、创建文件夹失败，返回MP_FAILED
*/
TEST_F(PrepareFileSystemTest, CreateLocalFileDir)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string localPath = "/home/lxl/test1";

    mp_int32 ret;
    PrepareFileSystem om;

    // 创建文件夹成功
    stub.set(ADDR(CMpFile, DirExist), StubDirExistSuccess);
    stub.set(ADDR(CMpFile, CreateDir), StubCreateDirSuccess);
    ret = om.CreateLocalFileDir(localPath);
    EXPECT_EQ(ret, MP_SUCCESS);

    // 创建文件夹失败
    stub.set(ADDR(CMpFile, DirExist), StubDirExistFail);
    stub.set(ADDR(CMpFile, CreateDir), StubCreateDirFail);
    ret = om.CreateLocalFileDir(localPath);
    EXPECT_EQ(ret, MP_FAILED);

    // localPath为空
    localPath = "";
    ret = om.CreateLocalFileDir(localPath);
    EXPECT_EQ(ret, MP_FAILED);

    // localPath长度大于MAX_PATH_LEN
    mp_string tempStr = "0123456789012345678901234567890123456789012345678901234567890123456789";
    localPath = tempStr + tempStr + tempStr + tempStr;
    ret = om.CreateLocalFileDir(localPath);
    EXPECT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：挂载Dataturbo文件系统
* 前置条件：1、无
* check点：1、挂载成功， 返回MP_SUCCESS
*          2、挂载失败，返回错误码 或MP_FAILED
*/
TEST_F(PrepareFileSystemTest, MountDataturboFileSystemTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<mp_string> vecDataturboIP;
    vector<mp_string> vecStorageIp;
    mp_string storageFs = "/202101041911";
    vecDataturboIP.push_back("8.40.99.244");
    MountNasParam mountNasParam = {"11111", "data", vecStorageIp, vecDataturboIP, storageFs, "vmware_type"};
    mountNasParam.authKey = "test";
    mountNasParam.authPwd = "12345";
    mountNasParam.isFullPath = true;
    vector<mp_string> successMountPath;
    vector<mp_string> dtbMountPath;
    std::set<mp_string> availStorageIp;

    mp_int32 ret;
    PrepareFileSystem om;

    const mp_int32 ERR_CREATE_DATA_TURBO_LINK = 0x5E006702;
    const mp_int32 ERR_MOUNT_DATA_TURBO_FILE_SYSTEM = 0x5E025C01;
    const mp_int32 ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT = 0x5E025C02;

    // 获取使用HostSN失败
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNFailed);
    ret = om.MountDataturboFileSystem(mountNasParam, successMountPath, dtbMountPath);
    EXPECT_EQ(ret, MP_FAILED);

    // 执行脚本失败 创建dataturbo链路失败
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSN);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    ret = om.MountDataturboFileSystem(mountNasParam, successMountPath, dtbMountPath);
    EXPECT_EQ(ret, ERR_CREATE_DATA_TURBO_LINK);

    //执行挂载成功
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSN);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    ret = om.MountDataturboFileSystem(mountNasParam, successMountPath, dtbMountPath);
    EXPECT_EQ(ret, MP_SUCCESS);
    successMountPath.clear();

    // 下发的链路datatubro ip为空 未配置dataturbo逻辑端口 
    mountNasParam.vecDataturboIP.clear();
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSN);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    ret = om.MountDataturboFileSystem(mountNasParam, successMountPath, dtbMountPath);
    EXPECT_EQ(ret, ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT);
}

/*
* 用例名称：获取挂载路径
* 前置条件：无
* check点：获取挂载路径成功
*/
TEST_F(PrepareFileSystemTest, GetMountPath)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MountNasParam mountNasParam;
    mp_string iterIp;
    mp_string mountPath;
    PrepareFileSystem om;

    mountNasParam.isFullPath = false;
    om.GetMountPath(mountNasParam, iterIp, mountPath);
}

/*
* 用例名称：获取挂载参数
* 前置条件：无
* check点：获取挂载路径成功
*/
TEST_F(PrepareFileSystemTest, GetMountParam)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MountNasParam mountNasParam;
    mp_string mountOption;
    mp_string mountProtocol;
    PrepareFileSystem om;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&)) \
        ADDR(CConfigXmlParser, GetValueString), StubGetValueStringFailedTwo);
    mp_int32 iRet = om.GetMountParam(mountNasParam, mountOption, mountProtocol);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&)) \
        ADDR(CConfigXmlParser, GetValueString), StubPrepareFileSystemGetValueStringSuccess);
    iRet = om.GetMountParam(mountNasParam, mountOption, mountProtocol);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
