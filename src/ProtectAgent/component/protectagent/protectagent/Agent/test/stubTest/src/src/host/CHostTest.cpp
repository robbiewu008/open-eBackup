/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */
#include "host/CHostTest.h"
#include <vector>
#ifdef WIN32
#include <windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "cfgmgr32.h"
#include "setupapi.h"
#endif
#include "host/host.h"
#include "common/File.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "common/CSystemExec.h"
#include "common/Uuid.h"
#include "securecom/RootCaller.h"
#include "common/ConfigXmlParse.h"
#include "common/Ip.h"
#include "host/If.h"
#include "array/disk.h"
#include "securecom/Ip.h"
#include "securecom/SecureUtils.h"
#include <iostream>
using namespace std;

namespace {
mp_void LogTest()
{}
#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)


mp_bool MockFileExist_TRUE(const mp_string& pszFilePath)
{
    return MP_TRUE;
}

mp_bool MockFileExist_FALSE(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_int32 MockReadFile(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("ENVIRONMENT_TYPE=0");
    return MP_SUCCESS;
}

mp_int32 MockReadFileEmpty(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    return MP_SUCCESS;
}

mp_int32 MockReadFile_FALSE(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("ENVIRONMENT_TYPE=0");
    return MP_FAILED;
}

mp_int32 MockReadFile_IP_FALSE(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("nas.container.kubernetes.io/ip_address=\"172.17.128.3\"");
    return MP_SUCCESS;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubWriteFileSuccess(const mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}

mp_int32 StubWriteFileFail(const mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    return MP_FAILED;
}

mp_int32 StubChmodFileSuccess(const mp_string& strFileName, mp_uint32 mode)
{
    return MP_SUCCESS;
}

mp_int32 StubChmodFileFail(const mp_string& strFileName, mp_uint32 mode)
{
    return MP_FAILED;
}

mp_int32 StubChownFileSuccess(const mp_string& strFileName, mp_int32 uid, mp_int32 gid)
{
    return MP_SUCCESS;
}

mp_int32 StubChownFileFail(const mp_string& strFileName, mp_int32 uid, mp_int32 gid)
{
    return MP_FAILED;
}

mp_int32 StubCopyHostSNSuccess(const mp_string& strSrcHostSnFile, const mp_string& strDestHostSnFile)
{
    return MP_SUCCESS;
}

mp_int32 StubCopyHostSNFail(const mp_string& strSrcHostSnFile, const mp_string& strDestHostSnFile)
{
    return MP_FAILED;
}

mp_int32 StubGetInstallSceneSuccess(void *obj, mp_string& strSceneType)
{
    strSceneType = "1";
    return MP_SUCCESS;
}

mp_int32 StubTestGetInstallSceneSuccess(mp_string& strSceneType)
{
    strSceneType = "1";
    return MP_SUCCESS;
}

mp_int32 StubGetInstallSceneFail(void *obj, mp_string& strSceneType)
{
    return MP_FAILED;
}

mp_int32 StubTestGetInstallSceneFail(mp_string& strSceneType)
{
    return MP_FAILED;
}

mp_int32 StubTestGetHostIPListSuccess(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    ipv4List.push_back("127.0.0.1");
    ipv6List.push_back("fe80::c63f:7884:eb7b:9679");
    return MP_SUCCESS;
}

mp_int32 StubTestGetListenIPAndPortSuccess(mp_string& strIP, mp_string& strPort)
{
    strIP = "127.0.0.1";
    return MP_SUCCESS;
}

mp_int32 StubTestGetListenIPAndPortFail(mp_string& strIP, mp_string& strPort)
{
    return MP_FAILED;
}

mp_string StubGetConfPath()
{
    return "test";
}

mp_int32 StubExecSystemWithoutEchoNoWin(mp_string strLogCmd, mp_string& strCommand, mp_bool bNeedRedirect)
{
    return MP_SUCCESS;
}

mp_int32 StubGetUidByUserNameSuccess(const mp_string& strUserName, mp_int32& uid, mp_int32& gid)
{
    return MP_SUCCESS;
}

mp_int32 StubGetUidByUserNameFail(const mp_string& strUserName, mp_int32& uid, mp_int32& gid)
{
    return MP_FAILED;
}

mp_int32 StubGetUuidNumberSuccess(mp_string& strUuid)
{
    strUuid = "11111";
    return MP_SUCCESS;
}

mp_int32 StubGetUuidNumberFail(mp_string& strUuid)
{
    return MP_FAILED;
}

mp_int32 StubReadHostSNInfoSuccess(void *obj, vector<mp_string>& vecMacs)
{
    vecMacs.push_back("11111");
    return MP_SUCCESS;
}

mp_int32 StubReadHostSNInfoFail(void *obj, vector<mp_string>& vecMacs)
{
    return MP_FAILED;
}

mp_int32 StubSetHostSNSuccess(void *obj, const mp_string& strHostSnFile, vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}

mp_int32 StubSetHostSNFail(void *obj, const mp_string& strHostSnFile, vector<mp_string>& vecInput)
{
    return MP_FAILED;
}

mp_int32 StubExecTestSuccess(void* obj, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    if (pvecResult) {
        pvecResult->push_back("32");
    }
    return MP_SUCCESS;
}

mp_int32 StubExecTestFail(void *obj, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_FAILED;
}

mp_int32 StubExec(void *obj, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_SUCCESS;
}

mp_int32 StubExecSystemWithEchoSuccess(
    const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect = MP_TRUE)
{
    if (mp_string::npos != strCommand.find("sys/class/fc_host/host")) {
        strEcho.push_back("0x10000090faf01674");
        strEcho.push_back("0x10000090faf01675");
    } else {
        strEcho.push_back("test");
    }

    return MP_SUCCESS;
}

mp_int32 StubExecSystemWithEchoFail(
    const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect = MP_TRUE)
{
    return MP_FAILED;
}

mp_int32 StubExecUserDefineScriptSuccess(mp_int32 iCommandID, const std::string &scriptCmd)
{
    return MP_SUCCESS;
}

mp_int32 StubExecUserDefineScriptFail(mp_int32 iCommandID, const std::string &scriptCmd)
{
    return MP_FAILED;
}

mp_int32 StubTestStringSuccess(mp_string& str)
{
    return MP_SUCCESS;
}

mp_int32 StubTestStringFail(mp_string& str)
{
    return MP_FAILED;
}

mp_int32 StubTestInt32Success(mp_int32& str)
{
    return MP_SUCCESS;
}

mp_int32 StubTestInt32Fail(mp_int32& str)
{
    return MP_FAILED;
}

mp_int32 StubGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32ReturnFail(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueStringSuccess(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubSetVauleSuccess(const mp_string& strSection, const mp_string& strKey, const mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubSetVauleFail(const mp_string& strSection, const mp_string& strKey, const mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueStringFail(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubGetListenIPAndPortSuccess(mp_string& strIP, mp_string& strPort)
{
    return MP_SUCCESS;
}

mp_int32 StubGetListenIPAndPortFail(mp_string& strIP, mp_string& strPort)
{
    return MP_FAILED;
}

mp_int32 StubGetHostOSSuccess(mp_int32& iOSType, mp_string& strOSVersion)
{
    return MP_SUCCESS;
}

mp_int32 StubGetHostOSFail(mp_int32& iOSType, mp_string& strOSVersion)
{
    return MP_FAILED;
}

mp_string StubGetStmpFilePath(mp_void*, mp_string strFileName)
{
    strFileName = "test";
    return "test";
}

mp_int32 StubGetAllIfInfoSuccess(vector<if_info_t>& ifs)
{
    return MP_SUCCESS;
}

mp_int32 StubGetAllIfInfoFail(vector<if_info_t>& ifs)
{
    return MP_FAILED;
}

mp_int32 StubGetArrayVendorAndProductSuccess(mp_void*, const mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    strvendor = "  HUAWEI";
    return MP_SUCCESS;
}


mp_int32 StubGetArrayVendorAndProductFail(mp_void*, const mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    return MP_FAILED;
}

mp_int32 StubGetAllDiskNameSuccess(vector<mp_string>& vecDiskName)
{
    vecDiskName.push_back("test1");
    return MP_SUCCESS;
}

mp_int32 StubGetAllDiskNameFail(mp_void*, vector<mp_string>& vecDiskName)
{
    return MP_FAILED;
}

mp_int32 StubGetArraySNSuccess(const mp_string& strDev, mp_string& strSN)
{
    return MP_SUCCESS;
}

mp_int32 StubGetArraySNFail(const mp_string& strDev, mp_string& strSN)
{
    return MP_FAILED;
}

mp_int32 StubGetLunInfoSuccess(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    return MP_SUCCESS;
}

mp_int32 StubGetLunInfoFail(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    return MP_FAILED;
}

mp_int32 StubUpdateSnmpV3ParamSuccess(const snmp_v3_param& stSnmpV3Param)
{
    return MP_SUCCESS;
}

mp_int32 StubUpdateSnmpV3ParamFail(const snmp_v3_param& stSnmpV3Param)
{
    return MP_FAILED;
}

mp_int32 StubUpdateAllTrapInfoSuccess(const std::vector<trap_server>& vecStServerInfo)
{
    return MP_SUCCESS;
}

mp_int32 StubUpdateAllTrapInfoFail(const std::vector<trap_server>& vecStServerInfo)
{
    return MP_FAILED;
}

mp_int32 StubGetFolderFileSuccess(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    return MP_SUCCESS;
}

mp_int32 StubGetFolderFileFail(mp_string& strFolder, vector<mp_string>& vecFileList)
{
    return MP_FAILED;
}

mp_int32 StubSysExecScriptSuccess(
    mp_string strScriptFileName, mp_string strParam, vector<mp_string> pvecResult[], mp_bool bNeedCheckSign, pFunc cb)
{
    return MP_SUCCESS;
}

mp_int32 StubSysExecScriptFail(
    mp_string strScriptFileName, mp_string strParam, vector<mp_string> pvecResult[], mp_bool bNeedCheckSign, pFunc cb)
{
    return MP_FAILED;
}

mp_int32 StubGetHostIPListSuccess(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    return MP_SUCCESS;
}

mp_int32 StubGetHostIPListFail(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    return MP_FAILED;
}

mp_int32 StubCheckHostLinkStatustestSuccess(const std::vector<mp_string>& hostIpv4List,
    const std::vector<mp_string>& hostIpv6List, std::vector<mp_string>& srcIpv4List,
    std::vector<mp_string>& srcIpv6List)
{
    return MP_SUCCESS;
}

mp_int32 StubCheckHostLinkStatustestFail(const std::vector<mp_string>& hostIpv4List,
    const std::vector<mp_string>& hostIpv6List, std::vector<mp_string>& srcIpv4List,
    std::vector<mp_string>& srcIpv6List)
{
    return MP_FAILED;
}

mp_int32 StubGetBuildINEnvironmentTypeSuccess(mp_void*, mp_string& EnvironmentType)
{
    EnvironmentType = "0";
    return MP_SUCCESS;
}

mp_string GetConfFilePathSuccess(const mp_string& strFileName)
{
    mp_string strFilePath = "/etc/profile";
    return strFilePath;
}

mp_int32 FileExistSuccess(const std::string &strFilePath)
{
    return MP_TRUE;
}

mp_int32 FileExistFailed(const std::string &strFilePath)
{
    return MP_FALSE;
}

mp_string GetConfFilePathFailed(const mp_string& strFileName)
{
    mp_string strFilePath = "";
    return strFilePath;
}

mp_int32 WriteFileSuccess(const mp_string& strFilePathTest, const vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}

mp_int32 AccumulateResourceUsageSuccess(ResourceUageInfoT& ResourceUsageInfo)
{
    return MP_SUCCESS;
}
mp_int32 AccumulateResourceUsageFailed(ResourceUageInfoT& ResourceUsageInfo)
{
    return MP_FAILED;
}

mp_int32 GetResourceExecSystemWithEchoSuccess(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("testmin 123456 2.0 2.0 test123 test test test test test test");
    return MP_SUCCESS;
}
mp_int32 CutStringFailed(string& strToCut, vector<mp_string>& strCache)
{
    return MP_FAILED;
}

mp_int32 CheckStringToDoubleFail(const mp_string& stringToDouble, double& resDouble)
{
    return MP_FAILED;
}
mp_int32 CheckStringToDoubleSuccess(const mp_string& stringToDouble, double& resDouble)
{
    return MP_SUCCESS;
}
#ifdef WIN32
mp_int32 GetPidSuccess()
{
    return MP_SUCCESS;
}
mp_int32 GetPidFailed()
{
    return MP_FAILED;
}
mp_int32 GetCpuUsageSuccess()
{
    return MP_SUCCESS;
}
mp_int32 GetCpuUsageFailed()
{
    return MP_FAILED;
}
mp_int32 GetPIDByNameSuccess(LPCTSTR szProcessName)
{
    return MP_SUCCESS;
}
mp_int32 GetPIDByNameFailed(LPCTSTR szProcessName)
{
    return MP_FAILED;
}
mp_int32 GetCpuTimeSucc(mp_uint32& processID, std::vector<mp_int64>& vecTime, std::vector<mp_int64>& vecSysTime)
{
    return MP_SUCCESS;
}
mp_int32 GetCpuTimeFail(mp_uint32& processID, std::vector<mp_int64>& vecTime, std::vector<mp_int64>& vecSysTime)
{
    return MP_FAILED;
}
mp_double GetMemUsageSuccess(mp_uint32& ProcessID)
{
    return 0.2;
}
mp_double GetMemUsageFailed(mp_uint32& ProcessID)
{
    return 0;
}
#endif

}  // namespace end

/*
 * 用例名称：读取环境配置文件失败
 * 前置条件：环境配置文件存在
 * check点：读取环境配置文件成功
 */
TEST_F(CHostTest, GetContainerIPListTest)
{
    DoLogTest();
    stub.set(&CMpFile::FileExist, MockFileExist_TRUE);
    stub.set(&CMpFile::ReadFile, MockReadFile_IP_FALSE);
    std::vector<mp_string> environment;
    auto host = CHost();
    auto iRet = host.GetContainerIPList(environment);
    for (auto item : environment) {
        cout<<item<<endl;
    }
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：保存主机SN号
 * 前置条件：
 * check点：保存主机SN号成功
 */
TEST_F(CHostTest, SetHostSN)
{
    DoLogTest();
    stub.set(&CIPCFile::WriteFile, StubWriteFileSuccess);
    stub.set(ChmodFile, StubChmodFileSuccess);
    stub.set(&CHost::CopyHostSN, StubCopyHostSNSuccess);
    
    mp_string strHostSnFile;
    vector<mp_string> vecInput;
    auto host = CHost();
    auto iRet = host.SetHostSN(strHostSnFile, vecInput);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CIPCFile::WriteFile, StubWriteFileFail);
    iRet = host.SetHostSN(strHostSnFile, vecInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CIPCFile::WriteFile, StubWriteFileSuccess);
    stub.set(ChmodFile, StubChmodFileFail);
    iRet = host.SetHostSN(strHostSnFile, vecInput);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CIPCFile::WriteFile, StubWriteFileSuccess);
    stub.set(ChmodFile, StubChmodFileSuccess);
    stub.set(&CHost::CopyHostSN, StubCopyHostSNFail);
    iRet = host.SetHostSN(strHostSnFile, vecInput);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：保存主机扩展信息
 * 前置条件：
 * check点：获取成功
 */
TEST_F(CHostTest, GetHostExtendInfo)
{
    DoLogTest();
    stub.set(&CIP::GetInstallScene, StubTestGetInstallSceneSuccess);

    Json::Value jValue;
    mp_int32 m_proxyRole;
    auto host = CHost();
    auto iRet = host.GetHostExtendInfo(jValue, m_proxyRole);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CIP::GetInstallScene, StubTestGetInstallSceneFail);
    iRet = host.GetHostExtendInfo(jValue, m_proxyRole);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CIP::GetInstallScene, StubTestGetInstallSceneSuccess);
    iRet = host.GetHostExtendInfo(jValue, m_proxyRole);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：设置agent日志级别
 * 前置条件：
 * check点：修改xml成功-设置成功
 *          修改xml失败-设置失败
 */
TEST_F(CHostTest, SetLogLevel)
{
    DoLogTest();
    auto host = CHost();

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32ReturnFail);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string& strSection, const mp_string& strKey, const mp_string& strValue))ADDR(CConfigXmlParser, SetValue),
        StubSetVauleFail);

    EXPECT_EQ(host.SetLogLevel(10000), MP_FAILED);
    EXPECT_EQ(host.SetLogLevel(0), MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32ReturnSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string& strSection, const mp_string& strKey, const mp_string& strValue))ADDR(CConfigXmlParser, SetValue),
        StubSetVauleSuccess);
    EXPECT_EQ(host.SetLogLevel(1), MP_SUCCESS);
}

/*
 * 用例名称：保存主机扩展信息(agentiplist)
 * 前置条件：
 * check点：获取成功
 *          获取失败
 */
TEST_F(CHostTest, GetHostAgentIplist)
{
    DoLogTest();
    stub.set(&CIP::GetHostIPList, StubTestGetHostIPListSuccess);
    stub.set(&CIP::GetListenIPAndPort, StubTestGetListenIPAndPortSuccess);
    
    Json::Value jIpList;
    auto host = CHost();
    auto iRet = host.GetHostAgentIplist(jIpList);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CIP::GetHostIPList, StubGetHostIPListFail);
    iRet = host.GetHostAgentIplist(jIpList);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CIP::GetHostIPList, StubTestGetHostIPListSuccess);
    stub.set(&CIP::GetListenIPAndPort, StubTestGetListenIPAndPortFail);
    iRet = host.GetHostAgentIplist(jIpList);
    EXPECT_EQ(iRet, MP_FAILED);
}
/*
 * 用例名称：拷贝主机扩展信息
 * 前置条件：
 * check点：拷贝成功
            拷贝失败
 */
TEST_F(CHostTest, StubCopyHostSN)
{
    DoLogTest();
    stub.set(&CPath::GetConfPath, StubGetConfPath);
    stub.set(&CSystemExec::ExecSystemWithoutEchoNoWin, StubExecSystemWithoutEchoNoWin);
    stub.set(GetUidByUserName, StubGetUidByUserNameSuccess);
    stub.set(ChownFile, StubChownFileSuccess);
    stub.set(ChmodFile, StubChmodFileSuccess);
    
    mp_string strSrcHostSnFile = "test1";
    mp_string strDestHostSnFile = "test2";
    auto host = CHost();
    auto iRet = host.CopyHostSN(strSrcHostSnFile, strDestHostSnFile);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(GetUidByUserName, StubGetUidByUserNameFail);
    iRet = host.CopyHostSN(strSrcHostSnFile, strDestHostSnFile);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(GetUidByUserName, StubGetUidByUserNameSuccess);
    stub.set(ChownFile, StubChownFileFail);
    iRet = host.CopyHostSN(strSrcHostSnFile, strDestHostSnFile);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ChownFile, StubChownFileSuccess);
    stub.set(ChmodFile, StubChmodFileFail);
    iRet = host.CopyHostSN(strSrcHostSnFile, strDestHostSnFile);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：检查Trap服务
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, CheckTrapServer)
{
    DoLogTest();
    trap_server stTrapServer;
    stTrapServer.iPort = 100;
    stTrapServer.iVersion = 2;
    stTrapServer.strServerIP = "192.168.1.1";
    auto host = CHost();
    auto iRet = host.CheckTrapServer(stTrapServer);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stTrapServer.strServerIP = "11111";
    iRet = host.CheckTrapServer(stTrapServer);
    EXPECT_NE(iRet, MP_SUCCESS);
    
    stTrapServer.strServerIP = "192.168.1.1";
    stTrapServer.iPort = 100000;
    iRet = host.CheckTrapServer(stTrapServer);
    EXPECT_NE(iRet, MP_SUCCESS);

    stTrapServer.iPort = 100;
    stTrapServer.iVersion = 5;
    iRet = host.CheckTrapServer(stTrapServer);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：读取主机SN号
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, ReadHostSNInfo)
{
    DoLogTest();
    stub.set(&CPath::GetConfPath, StubGetConfPath);
    stub.set(&CMpFile::ReadFile, MockReadFile);
    vector<mp_string> vecMacs;
    auto host = CHost();
    auto iRet = host.ReadHostSNInfo(vecMacs);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CMpFile::ReadFile, MockReadFile_FALSE);
    iRet = host.ReadHostSNInfo(vecMacs);
    EXPECT_EQ(iRet, MP_FAILED);
    vecMacs.clear();

    stub.set(&CMpFile::ReadFile, MockReadFileEmpty);
    iRet = host.ReadHostSNInfo(vecMacs);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：查询主机SN号
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostSN)
{
    DoLogTest();
    stub.set(&CHost::ReadHostSNInfo, StubReadHostSNInfoSuccess);
    mp_string strSN;
    auto host = CHost();
    auto iRet = host.GetHostSN(strSN);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(strSN, "11111");

    stub.set(&CHost::ReadHostSNInfo, StubReadHostSNInfoFail);
    stub.set(&CMpFile::FileExist, MockFileExist_FALSE);
    stub.set(&CUuidNum::GetUuidNumber, StubGetUuidNumberSuccess);
    stub.set(&CHost::SetHostSN, StubSetHostSNSuccess);
    iRet = host.GetHostSN(strSN);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(strSN, "11111");

    stub.set(&CUuidNum::GetUuidNumber, StubGetUuidNumberFail);
    iRet = host.GetHostSN(strSN);
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(&CUuidNum::GetUuidNumber, StubGetUuidNumberSuccess);
    stub.set(&CHost::SetHostSN, StubSetHostSNFail);
    iRet = host.GetHostSN(strSN);
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(&CMpFile::FileExist, MockFileExist_TRUE);
    stub.set(&CHost::CopyHostSN, StubCopyHostSNSuccess);
    stub.set(&CHost::ReadHostSNInfo, StubReadHostSNInfoFail);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(strSN, "11111");
    
    stub.set(&CHost::CopyHostSN, StubCopyHostSNFail);
    iRet = host.GetHostSN(strSN);
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(&CHost::CopyHostSN, StubCopyHostSNSuccess);
    stub.set(&CHost::ReadHostSNInfo, StubReadHostSNInfoFail);
    iRet = host.GetHostSN(strSN);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：查询Agent版本信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetAgentVersion)
{
    DoLogTest();
    mp_string strAgentVersion;
    mp_string strBuildNum;
    auto host = CHost();
    auto iRet = host.GetAgentVersion(strAgentVersion, strBuildNum);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：查询主机是32位还是64位
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostOSBit)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    mp_int32 iOSBit;
    auto host = CHost();
    auto iRet = host.GetHostOSBit(iOSBit);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(iOSBit, 32);

    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.GetHostOSBit(iOSBit);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：查询主机类型
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostOS)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    mp_int32 iOSType;
    mp_string strOSVersion;
    auto host = CHost();
    auto iRet = host.GetHostOS(iOSType, strOSVersion);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.GetHostOS(iOSType, strOSVersion);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：获取主机的虚拟机化平台
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostHypervisorType)
{
    DoLogTest();
    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoSuccess);
    mp_string hypervisorType;
    auto host = CHost();
    auto iRet = host.GetHostHypervisorType(hypervisorType);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoFail);
    iRet = host.GetHostHypervisorType(hypervisorType);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：获取主机的虚拟机化平台
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostOSExecCmd)
{
    DoLogTest();
    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoSuccess);
    mp_int32 iOSType;
    mp_string strOSVersion;
    mp_string strCmd;
    auto host = CHost();
    auto iRet = host.GetHostOSExecCmd(iOSType, strOSVersion, strCmd);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoFail);
    iRet = host.GetHostOSExecCmd(iOSType, strOSVersion, strCmd);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：查询主机信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetInfo)
{
    DoLogTest();
    stub.set(&CHost::GetHostSN, StubTestStringSuccess);
    stub.set(GetHostName, StubTestStringSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32ReturnSuccess);
    stub.set(&CIP::GetListenIPAndPort, StubGetListenIPAndPortSuccess);
    stub.set(&CHost::GetHostOS, StubGetHostOSSuccess);
    stub.set(&CHost::GetHostOSBit, StubTestInt32Success);
    stub.set(&CHost::GetHostHypervisorType, StubTestStringSuccess);
    host_info_t hostInfo;
    auto host = CHost();
    auto iRet = host.GetInfo(hostInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CHost::GetHostSN, StubTestStringFail);
    iRet = host.GetInfo(hostInfo);
    EXPECT_NE(iRet, MP_SUCCESS);
    
    stub.set(&CHost::GetHostSN, StubTestStringSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32ReturnFail);
    iRet = host.GetInfo(hostInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
       
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubGetValueInt32ReturnSuccess);
    stub.set(&CIP::GetListenIPAndPort, StubGetListenIPAndPortFail);
    iRet = host.GetInfo(hostInfo);
    EXPECT_NE(iRet, MP_SUCCESS);

    stub.set(&CIP::GetListenIPAndPort, StubGetListenIPAndPortSuccess);
    stub.set(&CHost::GetHostOS, StubGetHostOSFail);
    iRet = host.GetInfo(hostInfo);
    EXPECT_NE(iRet, MP_SUCCESS);
    
    stub.set(&CHost::GetHostOS, StubGetHostOSSuccess);
    stub.set(&CHost::GetHostOSBit, StubTestInt32Fail);
    iRet = host.GetInfo(hostInfo);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：查询agent信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetAgentInfo)
{
    DoLogTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringSuccess);
    agent_info_t agentInfo;
    auto host = CHost();
    auto iRet = host.GetAgentInfo(agentInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString),
        StubGetValueStringFail);
    iRet = host.GetAgentInfo(agentInfo);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：获取升级错误详细信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetUpgradeErrorDetails)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExec);
    stub.set(&CMpFile::FileExist, MockFileExist_TRUE);
    Json::Value jValue;
    auto host = CHost();
    auto iRet = host.GetUpgradeErrorDetails(jValue);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CMpFile::FileExist, MockFileExist_FALSE);
    iRet = host.GetUpgradeErrorDetails(jValue);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：更新升级错误详细信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, UpdateUpgradeErrorDetails)
{
    DoLogTest();
    Json::Value jValue;
    auto host = CHost();
    jValue["logDetail"] = 1;
    jValue["logInfo"] = "job_log_agent_storage_update_prepare_fail_label";
    jValue["logDetailParam"] = "Failed to download the upgrade package.";
    auto iRet = host.UpdateUpgradeErrorDetails(jValue);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：获取网络信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetNetworkInfo)
{
    DoLogTest();
    stub.set(&CIf::GetAllIfInfo, StubGetAllIfInfoSuccess);
    std::vector<mp_string> vecMacs;
    auto host = CHost();
    auto iRet = host.GetNetworkInfo(vecMacs);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CIf::GetAllIfInfo, StubGetAllIfInfoFail);
    iRet = host.GetNetworkInfo(vecMacs);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：查询主机磁盘信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetDiskInfo)
{
    DoLogTest();
    stub.set(&Disk::GetAllDiskName, StubGetAllDiskNameSuccess);
    stub.set(&Array::GetArrayVendorAndProduct, StubGetArrayVendorAndProductSuccess);
    stub.set(&Array::GetArraySN, StubGetArraySNSuccess);
    stub.set(&Array::GetLunInfo, StubGetLunInfoSuccess);
    vector<host_lun_info_t> vecLunInfo;
    auto host = CHost();
    auto iRet = host.GetDiskInfo(vecLunInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&Disk::GetAllDiskName, StubGetAllDiskNameFail);
    iRet = host.GetDiskInfo(vecLunInfo);
    EXPECT_NE(iRet, MP_SUCCESS);
    
    stub.set(&Disk::GetAllDiskName, StubGetAllDiskNameSuccess);
    stub.set(&Array::GetArrayVendorAndProduct, StubGetArrayVendorAndProductFail);
    iRet = host.GetDiskInfo(vecLunInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&Array::GetArrayVendorAndProduct, StubGetArrayVendorAndProductSuccess);
    stub.set(&Array::GetArraySN, StubGetArraySNFail);
    iRet = host.GetDiskInfo(vecLunInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&Array::GetArraySN, StubGetArraySNSuccess);
    stub.set(&Array::GetLunInfo, StubGetLunInfoFail);
    iRet = host.GetDiskInfo(vecLunInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：查询主机时区信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetTimeZone)
{
    DoLogTest();
    timezone_info_t sttimezone;
    auto host = CHost();
    auto iRet = host.GetTimeZone(sttimezone);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
 * 用例名称：查询主机启动器信息
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetInitiatorsByProtocol)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    initiator_info_t initInfo;
    mp_string protocolType = SCRIPT_INITIATOR_PARAM_ISCSI;
    auto host = CHost();
    auto iRet = host.GetInitiatorsByProtocol(initInfo, protocolType);
    EXPECT_EQ(iRet, MP_SUCCESS);

    protocolType = SCRIPT_INITIATOR_PARAM_FC;
    iRet = host.GetInitiatorsByProtocol(initInfo, protocolType);
    EXPECT_EQ(iRet, MP_SUCCESS);

    protocolType = "";
    iRet = host.GetInitiatorsByProtocol(initInfo, protocolType);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.GetInitiatorsByProtocol(initInfo, protocolType);
    EXPECT_NE(iRet, MP_FAILED);
}

TEST_F(CHostTest, GetInitiators)
{
    DoLogTest();
    stub.set(ADDR(CHost, GetInitiatorsByProtocol), StubSuccess);
    initiator_info_t initInfo;
    auto host = CHost();
    auto iRet = host.GetInitiators(initInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(ADDR(CHost, GetInitiatorsByProtocol), StubFailed);
    iRet = host.GetInitiators(initInfo);
    EXPECT_NE(iRet, MP_SUCCESS);
}

mp_int32 StubGetInfoSuccess(mp_void* pThis, host_info_t& hostInfo)
{
    hostInfo.sn = "strSN_1234";
    hostInfo.strOS = "AIX";
    hostInfo.subType = 5;
    return MP_SUCCESS;
}

mp_int32 StubGetInfoFailed(mp_void* pThis, host_info_t& hostInfo)
{
    hostInfo.sn = "strSN_1234";
    hostInfo.strOS = "AIX";
    hostInfo.subType = 5;
    return MP_FAILED;
}

/*
 * 用例名称：查询主机的sn和subType
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostInfo)
{
    DoLogTest();
    stub.set(&CHost::GetInfo, StubGetInfoSuccess);
    auto host = CHost();
    Json::Value jValue1;
    auto iRet = host.GetHostInfo(jValue1);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CHost::GetInfo, StubGetInfoFailed);
    Json::Value jValue2;
    iRet = host.GetHostInfo(jValue2);
    EXPECT_EQ(iRet, MP_FAILED);
}

mp_int32 StubExecTestIqnSuccess(void* obj, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    if (pvecResult) {
        pvecResult->push_back("xxxxxxxx");
    }
    return MP_SUCCESS;
}

/*
 * 用例名称：查询主机iscsi卡的所有iqn
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, QueryIqns)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    auto host = CHost();
    std::map<mp_string, mp_string> mapIqns;
    auto iRet = host.QueryIqns(mapIqns);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.QueryIqns(mapIqns);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：查询主机FC卡的所有wwpn及状态
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, QueryWwpnsV2)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    auto host = CHost();
    std::map<mp_string, mp_string> mapWwpns;
    auto iRet = host.QueryWwpnsV2(mapWwpns);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.QueryWwpnsV2(mapWwpns);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：连接iscsi启动器
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, LinkiScsiTarget)
{
    DoLogTest();
    Json::Value scsiTargets;
    scsiTargets["storageIps"] = Json::arrayValue;
    scsiTargets["storageIps"].append("192.168.1.1");
    scsiTargets["storageIps"].append("192.168.1.2");
    {
        scsiTargets["storagePort"] = 8080;
        scsiTargets["authUser"] = "66666";
        scsiTargets["chapPwd"] = "555";
    }
    stub.set(ADDR(CRootCaller, Exec), StubExec);
    auto host = CHost();
    auto iRet = host.LinkiScsiTarget(scsiTargets);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.LinkiScsiTarget(scsiTargets);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：查询主机所有的wwpn号成功
 * 前置条件：1、查询命令执行成功，并且有wwpn信息
 * check点：检查返回值为MP_SUCCESS
 */
TEST_F(CHostTest, QueryWwpnsSuccess)
{
    DoLogTest();
    CHost host;

    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubExecSystemWithEchoSuccess);
    std::vector<mp_string> vecWwpns;
    auto iRet = host.QueryWwpns(vecWwpns);
    stub.reset(ADDR(CSystemExec, ExecSystemWithEcho));
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(vecWwpns[0], "10000090faf01674");
    EXPECT_EQ(vecWwpns[1], "10000090faf01675");
}

/*
 * 用例名称：查询主机所有的wwpn号失败
 * 前置条件：1、查询命令执行失败，或者查询结果无wwpn信息
 * check点：检查返回值MP_FAILED
 */
TEST_F(CHostTest, QueryWwpnsFailed)
{
    DoLogTest();
    CHost host;

    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubExecSystemWithEchoFail);
    std::vector<mp_string> vecWwpns;
    auto iRet = host.QueryWwpns(vecWwpns);
    stub.reset(ADDR(CSystemExec, ExecSystemWithEcho));
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：主机执行Dataturbo扫盘脚本成功
 * 前置条件：1、执行shell命令成功
 * check点：检查返回值MP_SUCCESS
 */
TEST_F(CHostTest, DataturboRescanSuccess)
{
    DoLogTest();
#ifdef LINUX
    CHost host;

    stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecUserDefineScriptSuccess);
    auto iRet = host.DataturboRescan();
    stub.reset(ADDR(CRootCaller, ExecUserDefineScript));
    EXPECT_EQ(iRet, MP_SUCCESS);
#endif
}

/*
 * 用例名称：主机执行Dataturbo扫盘脚本失败
 * 前置条件：1、执行shell命令失败
 * check点：检查返回值MP_FAILED
 */
TEST_F(CHostTest, DataturboRescanFailed)
{
    DoLogTest();
#ifdef LINUX
    CHost host;

    stub.set(ADDR(CRootCaller, ExecUserDefineScript), StubExecUserDefineScriptFail);
    auto iRet = host.DataturboRescan();
    stub.reset(ADDR(CRootCaller, ExecUserDefineScript));
    EXPECT_EQ(iRet, ERROR_DISK_SCAN_FAILED);
#endif
}

/*
 * 用例名称：更新TrapServer
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, UpdateTrapServer)
{
    DoLogTest();
    trap_server stTrapServer;
    stTrapServer.iPort = 100;
    stTrapServer.iVersion = 2;
    stTrapServer.strServerIP = "192.168.1.1";
    stub.set(&CAlarmConfig::UpdateSnmpV3Param, StubUpdateSnmpV3ParamSuccess);
    stub.set(&AlarmDB::UpdateAllTrapInfo, StubUpdateAllTrapInfoSuccess);
    std::vector<trap_server> vecTrapServer;
    vecTrapServer.push_back(stTrapServer);
    snmp_v3_param stParam;
    auto host = CHost();
    auto iRet = host.UpdateTrapServer(vecTrapServer, stParam);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CAlarmConfig::UpdateSnmpV3Param, StubUpdateSnmpV3ParamFail);
    iRet = host.UpdateTrapServer(vecTrapServer, stParam);
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(&CAlarmConfig::UpdateSnmpV3Param, StubUpdateSnmpV3ParamSuccess);
    stub.set(&AlarmDB::UpdateAllTrapInfo, StubUpdateAllTrapInfoFail);
    iRet = host.UpdateTrapServer(vecTrapServer, stParam);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：校验SNMP参数
 * 前置条件：
 * check点：校验成功
 */
TEST_F(CHostTest, VerifySnmp)
{
    DoLogTest();
    snmp_v3_param stParam;
    stParam.iAuthProtocol = AUTH_PROTOCOL_NONE;
    auto host = CHost();
    auto iRet = host.VerifySnmp(stParam);
    EXPECT_NE(iRet, MP_SUCCESS);

    stParam.iPrivProtocol = PRIVATE_PROTOCOL_NONE;
    iRet = host.VerifySnmp(stParam);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：查询第三方脚本
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, QueryThirdPartyScripts)
{
    DoLogTest();
    stub.set(&CMpFile::GetFolderFile, StubGetFolderFileSuccess);
    vector<mp_string> vectFileList;
    auto host = CHost();
    auto iRet = host.QueryThirdPartyScripts(vectFileList);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CMpFile::GetFolderFile, StubGetFolderFileFail);
    iRet = host.QueryThirdPartyScripts(vectFileList);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：执行第三方脚本
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, ExecThirdPartyScript)
{
    DoLogTest();
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    mp_string fileName = "test";
    mp_string paramValues = "test";
    vector<mp_string> vecResult;
    auto host = CHost();
    auto iRet = host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    iRet = host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    EXPECT_NE(iRet, MP_SUCCESS);
}

/*
 * 用例名称：获取主机IP列表
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetHostIPList)
{
    DoLogTest();
    stub.set(&CIP::GetHostIPList, StubGetHostIPListSuccess);
    stub.set(&CIP::GetInstallScene, StubGetInstallSceneSuccess);
    stub.set(&CIP::GetBuildINEnvironmentType, StubGetBuildINEnvironmentTypeSuccess);
    std::vector<mp_string> hostIpv4List;
    std::vector<mp_string> hostIpv6List;
    std::vector<mp_string> ipv4List;
    std::vector<mp_string> ipv6List;
    auto host = CHost();
    auto iRet = host.GetHostIPList(hostIpv4List, hostIpv6List, ipv4List, ipv6List);
    EXPECT_EQ(iRet, MP_SUCCESS);
    
    stub.set(&CIP::GetHostIPList, StubGetHostIPListFail);
    iRet = host.GetHostIPList(hostIpv4List, hostIpv6List, ipv4List, ipv6List);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：获取主机安装模式
 * 前置条件：参数文件存在，写入文件成功，参数文件不含有查找的字符串
 * check点：失败
 */
TEST_F(CHostTest, CheckAgentInstallModeSucc)
{
    DoLogTest();
    mp_string strFileName = "profile";
    std::vector<mp_string> vecInput = {"test"};
    mp_string strFilePathTest = "test";
    auto host = CHost();
    stub.set(&CPath::GetConfFilePath, GetConfFilePathSuccess);
    stub.set(&CMpFile::FileExist, FileExistSuccess);
    stub.set(& CIPCFile::WriteFile, WriteFileSuccess);
    auto iRet = host.CheckAgentInstallMode();
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
 * 用例名称：获取主机安装模式
 * 前置条件：参数文件不存在
 * check点：失败
 */
TEST_F(CHostTest, CheckAgentInstallModeFail)
{
    DoLogTest();
    mp_string strFileName = "profile";
    std::vector<mp_string> vecInput = {"test"};
    mp_string strFilePathTest = "test";
    auto host = CHost();
    stub.set(&CPath::GetConfFilePath, GetConfFilePathSuccess);
    stub.set(&CMpFile::FileExist,  FileExistFailed);
    auto iRet = host.CheckAgentInstallMode();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CPath::GetConfFilePath,  GetConfFilePathFailed);
    iRet = host.CheckAgentInstallMode();
    EXPECT_EQ(iRet, MP_FAILED);
}

mp_int32 GetInfoSuccess(mp_void* pThis, host_info_t& hostInfo)
{
    hostInfo.sn = "strSN_1234";
    hostInfo.strOS = "AIX";
    hostInfo.subType = 1;
    return MP_SUCCESS;
}
/*
 * 用例名称：获取资源占用
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetAgentResourceUsage)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    stub.set(&CHost::GetInfo, StubGetInfoFailed);
    auto iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, ERROR_HOST_GETINFO_FAILED);

    stub.set(&CHost::GetInfo, GetInfoSuccess);
#ifdef WIN32
    stub.set(&CHost::AccumulateResourceUsageWin, AccumulateResourceUsageSuccess);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
#endif
#ifdef LINUX
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageSuccess);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
#endif
#ifdef _AIX
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageSuccess);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
#endif
#ifdef SOLARIS
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageSuccess);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
#endif

#ifdef WIN32
    stub.set(&CHost::AccumulateResourceUsageWin, AccumulateResourceUsageFailed);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);
#endif
#ifdef LINUX
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageFailed);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);
#endif
#ifdef _AIX
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageFailed);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);
#endif
#ifdef SOLARIS
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageFailed);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);
#endif
}

/*
 * 用例名称：获取资源占用sanclient
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetAgentResourceUsageSanclient)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    stub.set(&CHost::GetInfo, StubGetInfoSuccess);

#ifdef LINUX
    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageSuccess);
    auto iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CHost::AccumulateResourceUsageLinux, AccumulateResourceUsageFailed);
    iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);
#endif
}

#ifdef WIN32
/*
 * 用例名称：Windows获取主机资源占用
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, AccumulateResourceUsageWin)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    stub.set(&CHost::GetPid, GetPidFailed);
    auto iRet = host.AccumulateResourceUsageWin(ResourceUageInfoT& ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CHost::GetPid, GetPidSuccess);
    host.m_vecProcessID = {"123", "234"};
    stub.set(&CHost::GetCpuUsage, GetCpuUsageFailed);
    iRet = host.AccumulateResourceUsageWin(ResourceUageInfoT& ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CHost::GetCpuUsage, GetCpuUsageSuccess);
    stub.set(&CHost::GetMemUsage, GetMemUsageSuccess);
    iRet = host.AccumulateResourceUsageWin(ResourceUageInfoT& ResourceUsageInfo);
    auto iResMem = ResourceUsageInfo.memUsage;
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(iResMem, 0.4);
}

/*
 * 用例名称：windows 获取PID
 * 前置条件：
 * check点：检查返回值
 */
TEST_F(CHostTest, GetPid)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    stub.set(&CHost::GetPIDByName, GetPIDByNameFailed);
    auto iRet = host.GetPid();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CHost::GetPIDByName, GetPIDByNameSuccess);
    iRet = host.GetPid();
    EXPECT_EQ(iRet, MP_SUCCESS);
}
#endif

#ifdef LINUX
/*
 * 用例名称：Linux获取资源占用量
 * 前置条件：ps命令不存在或者字符串分割函数异常
 * check点：失败
 */
TEST_F(CHostTest, AccumulateResourceUsageLinuxFail)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    stub.set(&CSystemExec::ExecSystemWithEcho, StubExecSystemWithEchoFail);
    auto iRet = host.AccumulateResourceUsageLinux(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&CSystemExec::ExecSystemWithEcho, GetResourceExecSystemWithEchoSuccess);
    stub.set(&CHost::CutString, CutStringFailed);
    iRet = host.AccumulateResourceUsageLinux(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_FAILED);
}
/*
 * 用例名称：Linux获取资源占用量
 * 前置条件：
 * check点：成功
 */
TEST_F(CHostTest, AccumulateResourceUsageLinux)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    stub.set(&CSystemExec::ExecSystemWithEcho, GetResourceExecSystemWithEchoSuccess);
    stub.set(&CHost::CheckStringToDouble, CheckStringToDoubleFail);
    auto iRet = host.AccumulateResourceUsageLinux(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(&CHost::CheckStringToDouble, CheckStringToDoubleSuccess);
    iRet = host.AccumulateResourceUsageLinux(ResourceUsageInfo);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
#endif

/*
 * 用例名称：字符串转化为double型
 * 前置条件：
 * check点：验证结果
 */
TEST_F(CHostTest, CheckStringToDouble)
{
    DoLogTest();
    ResourceUageInfoT ResourceUsageInfo;
    auto host = CHost();
    mp_double resDouble = 0;
    mp_string stringToDoubletest = "123";
    auto iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = "";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = "xs123";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = "1.";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = ".12";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = "1.23.";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = "1.2.3";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = ".";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_FAILED);
    EXPECT_EQ(resDouble, 0);

    stringToDoubletest = "7.2";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(resDouble, 7.2);

    stringToDoubletest = "2";
    iRet = host.CheckStringToDouble(stringToDoubletest, resDouble);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(resDouble, 2);
}