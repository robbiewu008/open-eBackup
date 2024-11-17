/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "host/host.h"
#include <sstream>
#include <fstream>
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Uuid.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "securecom/UniqueId.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "common/JsonHelper.h"
#include "securecom/CryptAlg.h"
#include "common/JsonHelper.h"
#include "securecom/Ip.h"
#include "securecom/SecureUtils.h"
#include "host/If.h"
#include "array/disk.h"
#include "array/array.h"
#include "securec.h"

using namespace std;

namespace {
const std::vector<mp_string> PROCESS_NAME_OF_AGENT = {
    "rdagent", "nginx", "monitor"
};
const std::vector<mp_string> PROCESS_NAME_OF_PLUGIN = {
    "NasPlugin", "FusionComputePlugin", "GeneralDBPlugin",
    "FilePlugin", "ElasticSearchPlugin", "VirtualizationPlugin", "HadoopPlugin", "LunOpTool"
};
const std::vector<mp_string> PROCESS_NAME_OF_AGENT_WIN = {
    "rdagent.exe", "rdnginx.exe", "monitor.exe", "AgentPlugin.exe"
};
const mp_double MIN_PERCENT = 0;
const mp_double MAX_PERCENT = 100;
const mp_int32 MIN_VALID_COLUMN = 5;
const mp_int32 CPU_COLUMN = 2;
const mp_int32 MEM_COLUMN = 3;
const mp_uint32 ONE_SECOND = 1000;
const mp_uint32 PERCENT = 100;
const mp_string PARAM_KEY_LOG_DETAIL = "logDetail";             // Error Code
const mp_string PARAM_KEY_LOG_DETAIL_INFO = "logDetailInfo";    // Error Details
const mp_string PARAM_KEY_LOG_DETAIL_PARAM = "logDetailParam";  // Placeholder: tool Parameters
const mp_string PARAM_KEY_LOG_INFO = "logInfo";                 // lable
const mp_string PARAM_KEY_LOG_INFO_PARAM = "logInfoParam";      // Placeholder: Version
const mp_string PARAM_KEY_EXTEND_SCENARIO = "scenario";
const mp_string PARAM_KEY_EXTEND_INTSALL_PATH = "install_path";
const mp_string PARAM_KEY_EXTEND_AGENTIPLIST = "agentIpList" ;
const mp_string PARAM_KEY_EXTEND_AGENTPORT = "agentNginxPort" ;
const mp_string CONTAINER_NETWORK_IP_INFO = "nas.container.kubernetes.io/ip_address";
const mp_string STORAGE_NETWORK_IP_INFO = "nas.storage.kubernetes.io/ip_address";
const mp_string CONTAINER_NETWORK_INFO_FILE_PATH = "/opt/podinfo/annotations";
const int CONTAINER_NETWORK_SIZE = 2;
const int CONTAINER_NETWORK_INDEX = 1;
const mp_string IN_AGENT_TYPE = "1";
const mp_string OUT_AGENT_TYPE = "0";
const mp_string ENVIRONMENT_TYPE = "ENVIRONMENT_TYPE";
const mp_string STRING_0X = "0x";
const int ENVIRONMENT_TYPE_MINISIZE = 2;
const mp_string DORADO_ENVIRONMENT = "0";
const mp_string VIRTUAL_DORADO_ENVIRONMENT = "1";
const mp_string PARAM_KEY_PM_IP = "PM_IP";
const mp_int32 BACKUP_ROLE_GENERA = 4;
const mp_int32 BACKUP_ROLE_SANCLIENT = 5;
const mp_string HOST_ENV_INSTALL_PATH = "DATA_BACKUP_AGENT_HOME";
const mp_string HOST_ENV_SANCLIENT_PATH = "DATA_BACKUP_SANCLIENT_HOME";
const mp_string MEM_FILE_NAME_LINUX = "/proc/meminfo";
const mp_string CPU_FILE_NAME_LINUX = "/proc/stat";    // linux系统存放cpu时间信息的文件
const mp_int32 VALID_TIME_MUNBER = 7;      // "/proc/stat"文件中，第一行的前7个数据为有效数据，内容分割后数据个数需不小于7
const mp_int32 LOCATION_IDLE_TIME = 3;     // 所需要的"idletime"的位置为4
const mp_int32 SIZE_OF_MEMTOTAL = 9;       // 需丢弃 "memtotal:" 部分长度为9
const mp_int32 SIZE_OF_MEMAVAILABLE = 13;  // 需丢弃 "memavailable:" 部分长度为9
const mp_int32 SIZE_OF_CPU = 5;            // 需丢弃 "cpu  "部分长度为5
const mp_int32 MEM_INFO_FILE_VALID_COUNT = 5;    // meminfo文件中，最多只取文件的前5行
const mp_int32 SIZE_MB = 1024 * 1024;
const mp_int32 SIZE_KB = 1024;

}  // namespace
CHost::CHost()
{
    CMpThread::InitLock(&m_pMutex);
}

CHost::~CHost()
{
    CMpThread::DestroyLock(&m_pMutex);
}

/* ------------------------------------------------------------
Description  : 保存主机SN号
Input        : vecInput -- SN号
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::SetHostSN(const mp_string& strHostSnFile, vector<mp_string>& vecInput)
{
    mp_int32 iRet;
    mp_string tmpHostSnFile = strHostSnFile;
    iRet = CIPCFile::WriteFile(tmpHostSnFile, vecInput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetHostSN: Write hostsn into hostsn file failed [%d].", iRet);
        return iRet;
    }

    // 修改权限
#ifdef WIN32
    mp_string strCommand = "cmd.exe /c echo Y | cacls.exe \"" + strHostSnFile + "\" /E /R Users";
    // cmd命令调用
    iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "cacls hostsn file failed %d.", iRet);

        return iRet;
    }
#else
    if (ChmodFile(strHostSnFile, S_IRUSR | S_IWUSR) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, file %s.", strHostSnFile.c_str());
        return MP_FAILED;
    }
#endif
    iRet = CopyHostSN(strHostSnFile, m_hostsnFile);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "copy hostsn  file failed [%d].", iRet);

        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Write file succ.");

    return MP_SUCCESS;
}

mp_int32 CHost::SetLogLevel(const mp_int32 level)
{
    if (level < OS_LOG_DEBUG || level > OS_LOG_CRI) {
        return MP_FAILED;
    }

    mp_int32 oldLevel = -1;
    auto iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_LEVEL, oldLevel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Get old log level failed.");
    }

    COMMLOG(OS_LOG_INFO, "Log level before update: %d, update log level to: %d", oldLevel, level);
    mp_string strLevel = std::to_string(level);
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_LOG_LEVEL, strLevel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get old log level failed.");
        return iRet;
    }

    return CLogger::GetInstance().SetLogLevel(level);
}

mp_int32 CHost::GetHostExtendInfo(Json::Value& jValue, mp_int32 m_proxyRole)
{
    std::string sceneType;
    mp_int32 iRet = CIP::GetInstallScene(sceneType);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    jValue[PARAM_KEY_EXTEND_SCENARIO] = sceneType;

    mp_string customInstallPath;
    if (m_proxyRole == BACKUP_ROLE_SANCLIENT) {
        CIP::GetHostEnv(HOST_ENV_SANCLIENT_PATH, customInstallPath);
    } else {
        CIP::GetHostEnv(HOST_ENV_INSTALL_PATH, customInstallPath);
        if (customInstallPath.empty()) {
#ifdef WIN32
            customInstallPath = GetSystemDiskChangedPathInWin(AGENT_DEFAULT_INSTALL_DIR);
#else
            customInstallPath = AGENT_DEFAULT_INSTALL_DIR;
#endif
        }
    }
    jValue[PARAM_KEY_EXTEND_INTSALL_PATH] = customInstallPath;
    INFOLOG("Custom installation path is %s.", customInstallPath.c_str());

    return MP_SUCCESS;
}

mp_int32 CHost::GetHostAgentIplist(Json::Value& jIpList)
{
    std::vector<mp_string> ipv4List;
    std::vector<mp_string> ipv6List;
    mp_int32 iRet = CIP::GetHostIPList(ipv4List, ipv6List);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get host iplist failed.");
        return MP_FAILED;
    }
    std::string strPort;
    std::string strIp;
    if (CIP::GetListenIPAndPort(strIp, strPort) != MP_SUCCESS) {
        ERRLOG("Get Agent listen IP and port failed.");
        return MP_FAILED;
    }
    mp_string ipList = strIp;
    for (const auto& iter : ipv4List) {
        if (iter.compare(strIp) != 0) {
            ipList += ',';
            ipList += iter;
        }
    }
    for (const auto& iter : ipv6List) {
        if (iter.compare(strIp) != 0) {
            ipList += ',';
            ipList += iter;
        }
    }
    jIpList[PARAM_KEY_EXTEND_AGENTIPLIST] = ipList;
    jIpList[PARAM_KEY_EXTEND_AGENTPORT] = strPort;
    return MP_SUCCESS;
}

mp_int32 CHost::CopyHostSN(const mp_string& strSrcHostSnFile, const mp_string& strDestHostSnFile)
{
    mp_string strDestDir = CPath::GetInstance().GetConfPath();
    // 路径校验
    strDestDir = CMpString::BlankComma(strDestDir);
#ifdef WIN32
    mp_string strCommand = "cmd.exe /c copy /y " + strSrcHostSnFile + " " + strDestDir + "\\ >nul";
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCommand));

    strCommand = "cmd.exe /c echo Y | cacls.exe \"" + strDestHostSnFile + "\" /E /R Users";
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCommand));
#else
    mp_string strCommand = "cp -f " + strSrcHostSnFile + " " + strDestDir;
    CHECK_FAIL_EX(CheckCmdDelimiter(strCommand));
    COMMLOG(OS_LOG_DEBUG, "copy command:%s", strCommand.c_str());
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCommand));
    COMMLOG(OS_LOG_DEBUG, "copy src hostSN: %s to dest dir: %s succ", strSrcHostSnFile.c_str(), strDestDir.c_str());
    // 获取rdadmin用户的uid
    mp_int32 uid(-1);
    mp_int32 gid(-1);
    int iRet = GetUidByUserName(AGENT_RUNNING_USER, uid, gid);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Get user(%s) uid and gid failed.", AGENT_RUNNING_USER.c_str());
        return MP_FAILED;
    }

    // 设置rdadmin的uid和gid
    iRet = ChownFile(strDestHostSnFile, uid, gid);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Chown file failed, file %s.", strDestHostSnFile.c_str());
        return MP_FAILED;
    }
    if (ChmodFile(strDestHostSnFile, S_IRUSR | S_IWUSR) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "chmod file failed, file %s.", strDestHostSnFile.c_str());
        return MP_FAILED;
    }
#endif
    return MP_SUCCESS;
}

mp_int32 CHost::CheckTrapServer(const trap_server& stTrapServer)
{
    // 参数合法性判断
    // 检查ip地址
    if (!CIP::CheckIsIPv6OrIPv4(stTrapServer.strServerIP)) {
        COMMLOG(OS_LOG_ERROR, "SNMP server IP \"%s\" is invalid", stTrapServer.strServerIP.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    // 检查端口
    if (stTrapServer.iPort > MAX_PORT_NUM || stTrapServer.iPort < 0) {
        COMMLOG(OS_LOG_ERROR, "Port number %d is invalid", stTrapServer.iPort);
        return ERROR_COMMON_INVALID_PARAM;
    }

    // 检查协议
    if (stTrapServer.iVersion > SNMP_V3 || stTrapServer.iVersion < SNMP_V1) {
        COMMLOG(OS_LOG_ERROR, "Protocol version %d is invalid", stTrapServer.iVersion);
        return ERROR_COMMON_INVALID_PARAM;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 读取主机SN号
Output       : vecMacs -- SN号
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::ReadHostSNInfo(vector<mp_string>& vecMacs)
{
    m_hostsnFile = CPath::GetInstance().GetConfFilePath(HOSTSN_FILE);

    mp_int32 iRet = CMpFile::ReadFile(m_hostsnFile, vecMacs);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "GetHostSN: Read host sn file failed [%d].", iRet);
        return iRet;
    }

    if (vecMacs.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "GetHostSN: HostSN file is NULL.");
        return MP_FAILED;
    }

    return iRet;
}

/* ------------------------------------------------------------
Description  :查询主机SN号
Output       : strSN -- SN号
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetHostSN(mp_string& strSN)
{
    LOGGUARD("");
    vector<mp_string> vecMacs;

    CThreadAutoLock cLock(&m_pMutex);
    mp_int32 iRet = CHost::ReadHostSNInfo(vecMacs);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Get host uuid from conf file failed, begin to get uuid.");

#ifdef WIN32
        mp_string hostsnFile = GetSystemDiskChangedPathInWin(HOSTSN_DIR) + HOSTSN_FILE;
#else
        mp_string hostsnFile = HOSTSN_DIR + HOSTSN_FILE;
#endif
        if (!CMpFile::FileExist(hostsnFile)) {
            iRet = CUuidNum::GetUuidNumber(strSN);
            if (iRet != MP_SUCCESS || strSN == "") {
                COMMLOG(OS_LOG_ERROR, "Get uuid failed, iRet %d or strUuid is empty.", iRet);

                return iRet;
            }

            COMMLOG(OS_LOG_DEBUG, "Get uuid of this host succ, uuid %s.", strSN.c_str());

            // 写入配置文件
            vecMacs.push_back(strSN);
            iRet = CHost::SetHostSN(hostsnFile, vecMacs);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Write hostsn into hostsn file failed [%d].", iRet);

                return iRet;
            }
        } else {
            iRet = CopyHostSN(hostsnFile, m_hostsnFile);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "copy hostsn  file failed [%d].", iRet);
                return iRet;
            }
            iRet = CHost::ReadHostSNInfo(vecMacs);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Get uuid failed, iRet %d or strUuid is empty.", iRet);
                return iRet;
            }
        }
    }

    if (vecMacs.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get uuid of this host failed, HostSN file is empty.");
        return MP_FAILED;
    }

    strSN = vecMacs.front();
    COMMLOG(OS_LOG_DEBUG, "Get uuid of this host succ, uuid %s.", strSN.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询Agent 版本信息
Output       : strAgentVersion -- Agent 版本信息
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetAgentVersion(mp_string& strAgentVersion, mp_string& strBuildNum)
{
    LOGGUARD("");
    strAgentVersion = AGENT_VERSION;
    strBuildNum = AGENT_BUILD_NUM;
    if (strAgentVersion != "" && strBuildNum != "") {
        COMMLOG(OS_LOG_DEBUG, "Agent version is %s, build number is %s.", strAgentVersion.c_str(), strBuildNum.c_str());

        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_ERROR, "Agent version or build number is empty.");

    return MP_FAILED;
}

#ifdef WIN32
mp_int32 CHost::GetWindowsOs(mp_int32& iOSType, mp_string& strOSVersion)
{
    mp_char cVerTmp[MAX_SYS_VERSION_LENGTH] = {0};
    mp_int32 dwVersion = GetVersion();

    mp_int32 dwMajorVer = (mp_int32)(LOBYTE(LOWORD(dwVersion)));
    mp_int32 dwMinorVer = (mp_int32)(HIBYTE(LOWORD(dwVersion)));

    CHECK_FAIL(sprintf_s(cVerTmp, sizeof(cVerTmp), "%d.%d", dwMajorVer, dwMinorVer));

    strOSVersion = cVerTmp;
    iOSType = HOST_TYPE_WINDOWS;
    COMMLOG(OS_LOG_DEBUG, "Get host version[%s] and ostype[%d].", strOSVersion.c_str(), iOSType);
    return MP_SUCCESS;
}
#endif
/* ------------------------------------------------------------
Description  : 查询主机是32位还是64位
Output       : iOSBit -- 主机位数
Create By    : z00455045
------------------------------------------------------------- */
mp_int32 CHost::GetHostOSBit(mp_int32& iOSBit)
{
#if defined WIN32
    iOSBit = 0;  // 暂时不支持windows获取主机操作系统位数
#elif defined LINUX
    // cmd: getconf LONG_BIT | sed 's/ //'
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_GETCONF, " LONG_BIT", &vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Inquire host os bit failed.");
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "Inquire host os bit failed, result is null.");
        return MP_FAILED;
    }

    stringstream ss(CMpString::TotallyTrim(vecResult.front()));
    ss >> iOSBit;
    if (iOSBit != HOST_OS_BIT_32 && iOSBit != HOST_OS_BIT_64) {
        COMMLOG(OS_LOG_ERROR, "Inquire host os bit %d, not is 32 or 64 bits.", iOSBit);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Host os bit is %d.", iOSBit);
#else
    iOSBit = 0;  // 不支持小机
#endif
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 查询主机类型
Output       : iOSType -- 主机类型
               strOSVersion-- 系统版本
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetHostOS(mp_int32& iOSType, mp_string& strOSVersion)
{
#ifdef WIN32
    return GetWindowsOs(iOSType, strOSVersion);
#else
    // sh执行该命令，错误的场景下仍然返回0，因此不能使用返回值判断，只能通过输出参数判断
    mp_string strParam;
    vector<mp_string> vecResult;
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_GETHOSTOS, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host version failed");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Get host version %s or ostype %s", vecResult.front().c_str(), vecResult.back().c_str());

    mp_uint64 tmp;
    CMpString::StringToUInteger(vecResult.front(), tmp);
    iOSType = static_cast<mp_int32>(tmp);
    strOSVersion = vecResult.back();
    return MP_SUCCESS;
#endif
}

/* ------------------------------------------------------------
Description  : 获取主机的虚拟机化平台，通过lscpu的回显捕获
Output       : hypervisorType -- 虚拟化类型，KVM/XEN/VMWARE/PHY等
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : z00455045
------------------------------------------------------------- */
mp_int32 CHost::GetHostHypervisorType(mp_string& hypervisorType)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to get host hypervisor type.");
#ifdef LINUX
    const mp_string strCmd = "lscpu | grep -i \"Hypervisor vendor\" | cut -d ':' -f 2 | sed 's/^ *//g'";

    vector<mp_string> vecRlt;
    mp_uint32 echoSize = 1;

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet == MP_SUCCESS && vecRlt.size() == echoSize) {
        hypervisorType = vecRlt.front();
        COMMLOG(OS_LOG_DEBUG, "Get host hypervisor type [%s] succ.", hypervisorType.c_str());
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_WARN, "Get host hypervisor type failed, iRet = %d.", iRet);
    return MP_FAILED;
#else
    hypervisorType = "null";
#endif
    return MP_SUCCESS;
}

// 未调用，strCmd入参
mp_int32 CHost::GetHostOSExecCmd(mp_int32& iOSType, mp_string& strOSVersion, const mp_string& strCmd)
{
    // Linux 非SuSE下返回1个结果,SuSE返回2个结果
#ifdef SUSE
    mp_int32 iRltCount = HOST_NUM_2;
#else
    mp_int32 iRltCount = HOST_NUM_1;
#endif

    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS || iRltCount != vecRlt.size()) {
        COMMLOG(OS_LOG_ERROR,
            "Get host version failed, iRet is %d or vecRlt size must be %d, but %d.",
            iRet,
            iRltCount,
            vecRlt.size());
        return MP_FAILED;
    }
    strOSVersion = vecRlt.front();

#ifdef SUSE
    strOSVersion += ".";
    strOSVersion += vecRlt.back();
#endif

    COMMLOG(OS_LOG_DEBUG, "Get host version[%s] and ostype[%d].", strOSVersion.c_str(), iOSType);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机信息
Output       : hostInfo -- 主机信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetInfo(host_info_t& hostInfo)
{
    mp_int32 iRet = GetHostSN(hostInfo.sn);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host sn failed, errCode:%d.", iRet);
        return ERROR_HOST_GETINFO_FAILED;
    }

    iRet = GetHostName(hostInfo.hostName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host name failed, errCode:%d.", iRet);
        return ERROR_HOST_GETINFO_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Get host name %s.", hostInfo.hostName.c_str());

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_ROLE, hostInfo.subType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup role config failed, set default value %d.", hostInfo.subType);
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_CLIENT_VERSION, hostInfo.version);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup role config failed, set agent version :%d.", hostInfo.version);
    }

    mp_string strPort;
    if (CIP::GetListenIPAndPort(hostInfo.endPoint, strPort) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Agent listen IP and port failed.");
        return ERROR_HOST_GETINFO_FAILED;
    }
    hostInfo.port = atoi(strPort.c_str());

    iRet = GetHostOS(hostInfo.nOS, hostInfo.osVersion);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host OsType and version failed, errCode:%d.", iRet);
        return ERROR_HOST_GETINFO_FAILED;
    }
    hostInfo.TrOS();
    COMMLOG(OS_LOG_DEBUG,
        "Get host os %s and version %s,agent version is :%s.",
        hostInfo.strOS.c_str(),
        hostInfo.osVersion.c_str(),
        hostInfo.version.c_str());
    if (GetHostOSBit(hostInfo.osBit) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host os bit failed.");
        return ERROR_HOST_GETINFO_FAILED;
    }

    iRet = GetHostHypervisorType(hostInfo.virtualType);  // try to get hypervisor type by lscpu (may be not exist)
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Get host hypervisor type failed, errCode:%d.", iRet);
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询agent信息
Output       : agentInfo -- agent信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : machenglin mwx1011302
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 CHost::GetAgentInfo(agent_info_t& agentInfo)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(
        CFG_SYSTEM_SECTION, CFG_CLIENT_VERSION, agentInfo.curVersion);
    if (iRet != MP_SUCCESS) {
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(
        CFG_SYSTEM_SECTION, CFG_VERSION_TIME_STAMP, agentInfo.versionTimeStamp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent version timestamp info failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取agent升级/修改应用类型任务的状态
Output       : strTaskStatus -- 任务的结果状态
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : machenglin mwx1011302
------------------------------------------------------------- */
mp_int32 CHost::GetTaskStatus(mp_string& taskType, mp_string& strTaskStatus)
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_bool iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "The upgrade status file does not exist.");
        return MP_FAILED;
    }

    std::ifstream stream;
    stream.open(strFilePath.c_str(), std::ifstream::in);
    mp_string line;
    mp_string strUpgradeText;

    mp_string strText = taskType;
    if (stream.is_open()) {
        while (getline(stream, line)) {
            if (line.find(strText.c_str()) != std::string::npos) {
                strUpgradeText = line;
                break;
            }
        }
    } else {
        return MP_FAILED;
    }

    std::size_t start = strUpgradeText.find("=", 0) + 1;
    strTaskStatus = strUpgradeText.substr(start);

    if (stream.is_open()) {
        stream.close();
    }

    return MP_SUCCESS;
}

mp_int32 CHost::GetUpgradeErrorDetails(Json::Value& jValue)
{
    mp_string strFilePath = CPath::GetInstance().GetStmpFilePath(CMD_RUNNING_UPGRADE_ERROR_MSG);
    mp_bool iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        COMMLOG(OS_LOG_ERROR, "The error file does not exist.");
        strFilePath = CPath::GetInstance().GetTmpFilePath(CMD_RUNNING_UPGRADE_ERROR_MSG);
        iRet = CMpFile::FileExist(strFilePath);
        if (iRet != MP_TRUE) {
            COMMLOG(OS_LOG_ERROR, "The error file does not exist.");
            return MP_FAILED;
        }
    }

    std::string logDetailStr;
    std::string logInfoStr;
    std::string logDetailParamsStr;

    CMpFile::GetProfileSection(strFilePath, PARAM_KEY_LOG_DETAIL, logDetailStr);
    CMpFile::GetProfileSection(strFilePath, PARAM_KEY_LOG_INFO, logInfoStr);
    CMpFile::GetProfileSection(strFilePath, PARAM_KEY_LOG_DETAIL_PARAM, logDetailParamsStr);

    COMMLOG(OS_LOG_DEBUG, "Error info, logDetail=%s, logInfo=%s, logDetailParam=%s",
        logDetailStr.c_str(), logInfoStr.c_str(), logDetailParamsStr.c_str());

    jValue[PARAM_KEY_LOG_DETAIL] = logDetailStr;
    jValue[PARAM_KEY_LOG_INFO] = logInfoStr;
    jValue[PARAM_KEY_LOG_DETAIL_PARAM].append(logDetailParamsStr);

#ifdef WIN32
    CMpFile::DelFile(strFilePath);
#else
    if (strFilePath.find(AGENT_STMP_DIR) == std::string::npos) {
        CMpFile::DelFile(strFilePath);
    } else {
        CRootCaller rootCaller;
        rootCaller.RemoveFile(CMD_RUNNING_UPGRADE_ERROR_MSG);
    }
#endif
    return MP_SUCCESS;
}

mp_int32 CHost::UpdateUpgradeErrorDetails(const Json::Value& jValue)
{
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(CMD_RUNNING_UPGRADE_ERROR_MSG);
    std::ofstream streamout;
    streamout.open(strFilePath.c_str(), std::ifstream::out);
    if (streamout.is_open()) {
        mp_int32 errCode = jValue[PARAM_KEY_LOG_DETAIL].asInt();
        mp_string errLable = jValue[PARAM_KEY_LOG_INFO].asString();
        mp_string errDetail = jValue[PARAM_KEY_LOG_DETAIL_PARAM].asString();
        streamout << PARAM_KEY_LOG_DETAIL << STR_EQUAL << CMpString::to_string(errCode) << STR_CODE_WARP;
        streamout << PARAM_KEY_LOG_INFO << STR_EQUAL << errLable << STR_CODE_WARP;
        streamout << PARAM_KEY_LOG_DETAIL_PARAM << STR_EQUAL << errDetail << STR_CODE_WARP;
        streamout.close();
    } else {
        ERRLOG("Failed to open the upgrade error details file.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取agent安装的模式
Output       : strInstallMode -- agent安装的模式
Return       : MP_SUCCESS -- agent安装模式为手动安装
               非MP_SUCCESS -- agent安装模式为推送安装或写入文件失败
------------------------------------------------------------- */
mp_int32 CHost::CheckAgentInstallMode()
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_bool bRet = CMpFile::FileExist(strFilePath);
    if (bRet != MP_TRUE) {
        ERRLOG("The Agent install mode file does not exist.");
        return MP_FAILED;
    }
    std::ifstream stream;
    stream.open(strFilePath.c_str(), std::ifstream::in);
    if (!stream.is_open()) {
        ERRLOG("Open the agent install mode file failed.");
        return MP_FAILED;
    }
    mp_string line;
    mp_int32 iReasult = MP_FAILED;
    mp_string strText = AGENT_INSTALL_MODE;
    std::vector<mp_string> vecInput = {AGENT_INSTALL_MODE};
    while (getline(stream, line)) {
        if (line.find(strText.c_str()) != std::string::npos) {
            DBGLOG("The installation mode is push installation.");
            mp_string strAgentModePath = CPath::GetInstance().GetLogFilePath(AGENT_PUSH_MODEFILE_PATH);
            mp_int32 iRet = CIPCFile::WriteFile(strAgentModePath.c_str(), vecInput);
            if (MP_SUCCESS != iRet) {
                ERRLOG("Write agent install mode file failed, file %s.", strAgentModePath.c_str());
            }
            iReasult = MP_SUCCESS;
            break;
        }
    }
    stream.close();
    return iReasult;
}

mp_int32 CHost::GetNetworkInfo(std::vector<mp_string>& vecMacs)
{
    LOGGUARD("");
    vector<if_info_t> ifs;
    vector<if_info_t>::iterator iter;

    COMMLOG(OS_LOG_DEBUG, "Begin Get network info.");
    mp_int32 iRet = CIf::GetAllIfInfo(ifs);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_DEBUG, "Get nics info failed, iRet %d.", iRet);
        return iRet;
    }

    for (iter = ifs.begin(); iter != ifs.end(); ++iter) {
        vecMacs.push_back(iter->strMac);
    }

    COMMLOG(OS_LOG_DEBUG, "Get network info succ.");
    return MP_SUCCESS;
}

mp_int32 CHost::Prepare4HP()
{
#ifdef HP_UX_IA
    // HP-UX需要清理无用dsf,防止出现无法发现设备情况，具体请看删除实现说明
    mp_int32 iRet = Disk::ClearInvalidDisk();
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_DISK_INFO_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ClearInvalidDisk failed.");
        return iRet;
    }

    // 清理磁盘后，扫描磁盘
    iRet = ScanDisk();
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_SCAN_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Execute script(ioscan -fnC disk) failed.");
        return iRet;
    }
#endif
    return MP_SUCCESS;
}

mp_int32 CHost::AfterHandle4HP(mp_string& strDevName, vector<mp_string>::iterator& iter, host_lun_info_t& structLunInfo,
    vector<host_lun_info_t>& vecLunInfo)
{
#ifdef HP_UX_IA
    mp_string strSecDevName;
    mp_int32 iRet = Disk::GetPersistentDSFByLegacyDSF(*iter, strSecDevName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get persistent DSF failed, device name %s, ret %d.", strDevName.c_str(), iRet);
        return iRet;
    }
    structLunInfo.deviceName = strSecDevName;
    vecLunInfo.push_back(structLunInfo);
#else
    (mp_void) iter;
    (mp_void) strDevName;
    (mp_void) structLunInfo;
    (mp_void) vecLunInfo;
#endif
    return MP_SUCCESS;
}

mp_int32 CHost::GetVendorAndProduct(mp_string& strDevName, mp_string& strVendor, mp_string& strProduct)
{
    mp_int32 iRet = Array::GetArrayVendorAndProduct(strDevName, strVendor, strProduct);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "The disk(%s) get array vendor and product failed.", strDevName.c_str());
        return MP_FAILED;
    }

    strVendor = CMpString::Trim(strVendor);
    strProduct = CMpString::Trim(strProduct);
    // 排除掉非华为的产品
    mp_bool bRet = (strcmp(strVendor.c_str(), ARRAY_VENDER_HUAWEI.c_str()) != 0 &&
                    strcmp(strVendor.c_str(), VENDOR_ULTRAPATH_HUAWEI.c_str()) != 0 &&
                    strcmp(strVendor.c_str(), ARRAY_VENDER_HUASY.c_str()) != 0 &&
                    strcmp(strVendor.c_str(), ARRAY_VENDER_FUSION_STORAGE.c_str()) != 0);
    if (bRet) {
        COMMLOG(OS_LOG_WARN, "The disk(%s) is not huawei LUN, Vendor:%s.", strDevName.c_str(), strVendor.c_str());

        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机磁盘信息(HP一个LUN返回2条设备信息，/dev/dsk/和/dev/disk/)
Output       : vecLunInfo -- 主机磁盘信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetDiskInfo(vector<host_lun_info_t>& vecLunInfo)
{
    LOGGUARD("");
    mp_string strDevName;

    mp_int32 iRet = Prepare4HP();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    vector<mp_string> vecDiskName;
    iRet = Disk::GetAllDiskName(vecDiskName);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_DISK_INFO_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get bolck devs failed, iRet %d.", iRet);
        return iRet;
    }

    for (vector<mp_string>::iterator iter = vecDiskName.begin(); iter != vecDiskName.end(); ++iter) {
        host_lun_info_t structLunInfo;

        iRet = GetSingleDiskInfo(*iter, structLunInfo);
        if (iRet == MP_FAILED) {
            continue;
        }

        if (iRet != MP_SUCCESS) {
            return iRet;
        }

        COMMLOG(OS_LOG_INFO,
            "host_lun_info_t::arrayVendor=%s,lunId=%s,wwn=%s,arraySn=%s,"
            "arrayVersion=%s,arrayModel=%s,deviceName=%s,diskNumber=%s,",
            structLunInfo.arrayVendor.c_str(),
            structLunInfo.lunId.c_str(),
            structLunInfo.wwn.c_str(),
            structLunInfo.arraySn.c_str(),
            structLunInfo.arrayVersion.c_str(),
            structLunInfo.arrayModel.c_str(),
            structLunInfo.deviceName.c_str(),
            structLunInfo.diskNumber.c_str());

        vecLunInfo.push_back(structLunInfo);

        iRet = AfterHandle4HP(strDevName, iter, structLunInfo, vecLunInfo);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    return MP_SUCCESS;
}

mp_int32 CHost::GetSingleDiskInfo(const mp_string& diskName, host_lun_info_t& structLunInfo)
{
    mp_string strDevName;
    mp_int32 iRet;
#if defined(LINUX) || defined(AIX)
    // 拼装全路径
    strDevName = mp_string("/dev/") + diskName;
#elif defined WIN32
    strDevName = diskName;
#elif defined HP_UX_IA
    iRet = Disk::GetHPRawDiskName(diskName, strDevName);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get full disk name of disk(%s) failed, ret %d.", diskName.c_str(), iRet);
        return ERROR_DISK_GET_RAW_DEVICE_NAME_FAILED;
    }
#elif defined SOLARIS
    strDevName = mp_string("/dev/rdsk/") + diskName + mp_string("s0");
#endif
    // 厂商和型号
    if (GetVendorAndProduct(strDevName, structLunInfo.arrayVendor, structLunInfo.arrayModel) != MP_SUCCESS) {
        return MP_FAILED;
    }

    // 获取SN
    iRet = Array::GetArraySN(strDevName, structLunInfo.arraySn);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "The disk(%s) get array SN failed.", strDevName.c_str());
        return MP_FAILED;
    }

    iRet = Array::GetLunInfo(strDevName, structLunInfo.wwn, structLunInfo.lunId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "The disk(%s) get lun wwn and lun id failed.", strDevName.c_str());
        return MP_FAILED;
    }

#ifdef WIN32
    // 获取disknumber
    mp_int32 iDiskNum;
    iRet = Disk::GetDiskNum(strDevName, iDiskNum);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "The disk(%s) get disk number failed.", strDevName.c_str());
        return MP_FAILED;
    }

    mp_char cDiskNum[NAME_PATH_LEN] = {0};
    ITOA(iDiskNum, cDiskNum, NAME_PATH_LEN, AGENT_DECIMAL);
    structLunInfo.diskNumber = cDiskNum;
#endif

#ifdef HP_UX_IA
    structLunInfo.deviceName = diskName;
#elif defined SOLARIS
    structLunInfo.deviceName = "/dev/dsk/" + diskName;
#else
    structLunInfo.deviceName = std::move(strDevName);
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机时区信息
Output       : vecLunInfo -- 主机磁盘信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetTimeZone(timezone_info_t& sttimezone)
{
    mp_char ctzBias[HOST_TIMEZONE_LENGTH] = {0};
    COMMLOG(OS_LOG_DEBUG, "Begin get host time zone info.");

    sttimezone.iIsDST = HOST_ISNOTDST;

#ifdef WIN32
    mp_long ltotalBias;
    TIME_ZONE_INFORMATION tzi;

    mp_int32 iRet = GetTimeZoneInformation(&tzi);
    // 处理夏令时偏移量对时区的影响
    if (iRet == TIME_ZONE_ID_DAYLIGHT) {
        sttimezone.iIsDST = HOST_ISDST;
        ltotalBias = tzi.Bias + tzi.DaylightBias;
    } else {
        ltotalBias = tzi.Bias;
    }
    mp_long lBiasHour = ltotalBias / (HOST_TZ_CONVER_UNIT);
    mp_long lBiasMin = abs(ltotalBias) % (HOST_TZ_CONVER_UNIT);

    CHECK_FAIL(sprintf_s(ctzBias, sizeof(ctzBias), "%+03d%02d", lBiasHour, lBiasMin));

#else
    time_t tTime;
    tm tLoctime;

    (mp_void) time(&tTime);
    (mp_void) localtime_r(&tTime, &tLoctime);

    sttimezone.iIsDST = (tLoctime.tm_isdst > 0) ? HOST_ISDST : HOST_ISNOTDST;

#ifdef AIX
    tm tUtctime;
    (mp_void) gmtime_r(&tTime, &tUtctime);

    // tm_isdst置于0,防止夏令时偏移量有效时,
    // 影响Agent接口偏移量的准确性
    tLoctime.tm_isdst = 0;
    // CodeDex误报，Missing Check against Null
    time_t tLocal_t = mktime(&tLoctime);
    time_t tUtc_t = mktime(&tUtctime);

    mp_double dDifftime_sec = difftime(tLocal_t, tUtc_t);
    mp_int32 iDifftime_hour = (mp_int32)(dDifftime_sec / (HOST_TZ_CONVER_UNIT * HOST_TZ_CONVER_UNIT));
    mp_int32 iDifftime_min = (mp_int32)(dDifftime_sec / HOST_TZ_CONVER_UNIT) % (HOST_TZ_CONVER_UNIT);

    CHECK_FAIL(sprintf_s(ctzBias, sizeof(ctzBias), "%+03d%02d", iDifftime_hour, abs(iDifftime_min)));

#else
    // 返回的是%z格式化后的字符串长度,如+0800长度为5
    std::size_t slen = strftime(ctzBias, sizeof(ctzBias), "%z", &tLoctime);
    if (HOST_TIMEZONE_LENGTH - 1 != slen && HOST_TIMEZONE_LENGTH - HOST_NUM_2 != slen) {
        COMMLOG(OS_LOG_ERROR, "Get UTC Bias failed, return len is not 5 or 4, len is %d.", slen);
        return MP_FAILED;
    }

    // HP-UX 下Africa/Casablanca 为0000
    if (HOST_TIMEZONE_LENGTH - HOST_NUM_2 == slen) {
        sttimezone.strTzBias = "+";
    }

#endif
#endif

    sttimezone.strTzBias += ctzBias;
    COMMLOG(OS_LOG_DEBUG, "Get time zone succ, tzBias %s, isDst %d", sttimezone.strTzBias.c_str(), sttimezone.iIsDST);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询windows主机启动器信息
Output       : initInfo -- 主机启动器信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
#ifdef WIN32
mp_int32 CHost::GetInitiators(initiator_info_t& initInfo)
{
    vector<mp_string> vecResult;
    mp_int32 iRetIscsi = SecureCom::SysExecScript(SCRIPT_INITIATOR_WIN, SCRIPT_INITIATOR_PARAM_ISCSI, &vecResult);
    TRANSFORM_RETURN_CODE(iRetIscsi, ERROR_HOST_GET_INIATOR_FAILED);
    if (iRetIscsi != MP_SUCCESS) {
        mp_int32 iRettmp = ErrorCode::GetInstance().GetErrorCode(iRetIscsi);
        COMMLOG(OS_LOG_ERROR, "Exec script failed, return code %d, tranformed return code is %d", iRetIscsi, iRettmp);
        iRetIscsi = iRettmp;
    } else {
        for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
            COMMLOG(OS_LOG_DEBUG, "Get iscsi iqn ");
            *iter = CMpString::Trim(*iter);
            initInfo.iscsis.push_back(*iter);
            ClearString(*iter);
        }
    }
    vecResult.clear();
    mp_int32 iRetFc = m_CHBA.GetHBAInfo(vecResult);
    TRANSFORM_RETURN_CODE(iRetFc, ERROR_HOST_GET_INIATOR_FAILED);
    if (iRetFc != MP_SUCCESS) {
        mp_int32 iRettmp = ErrorCode::GetInstance().GetErrorCode(iRetFc);
        COMMLOG(OS_LOG_ERROR, "Exec script failed, return code is %d, tranformed return code is %d", iRetFc, iRettmp);
        iRetFc = iRettmp;
    } else {
        for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
            COMMLOG(OS_LOG_DEBUG, "Get fc wwn %s.", iter->c_str());
            *iter = CMpString::Trim(*iter);
            initInfo.fcs.push_back(*iter);
        }
    }
    // 获取启动器需要满足的条件
    mp_bool bBothFailed = ((iRetIscsi != MP_SUCCESS) && (iRetFc != MP_SUCCESS));
    mp_bool bBothFileExist = (iRetIscsi != ERROR_COMMON_SCRIPT_FILE_NOT_EXIST &&
                              iRetFc != ERROR_COMMON_SCRIPT_FILE_NOT_EXIST);
    mp_bool bBothSuccess = (iRetIscsi != ERROR_COMMON_SCRIPT_EXEC_FAILED && iRetFc != ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (bBothFailed) {
        if (bBothFileExist) {
            if (bBothSuccess) {
                COMMLOG(OS_LOG_ERROR, "Iscsi and fc initiators are not exist.");
                return ERROR_HOST_GET_INIATOR_FAILED;
            }
            COMMLOG(OS_LOG_ERROR, "%s exec failed.", SCRIPT_INITIATOR_WIN.c_str());
            return ERROR_COMMON_SCRIPT_EXEC_FAILED;
        }
        COMMLOG(OS_LOG_ERROR, "%s is not exist.", SCRIPT_INITIATOR_WIN.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }
    return MP_SUCCESS;
}
#else
/* ------------------------------------------------------------
Description  : 查询主机启动器信息
Output       : initInfo -- 主机启动器信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetInitiatorsByProtocol(initiator_info_t& initInfo, const mp_string protocolType)
{
    LOGGUARD("");
    vector<mp_string> vecResult;
    std::ostringstream scriptParam;
    scriptParam << "queryType=" << protocolType << NODE_COLON;
    CRootCaller rootCaller;
    // Linux下查询iscsci信息时需要读取initiatorname.iscsi文件信息，该文件在Suse10/Suse11下只有root有读权限
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_INIT, scriptParam.str(), &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_GET_INIATOR_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Rootcaller exec failed. %s , iRet %d.", protocolType.c_str(), iRet);
        return iRet;
    }
    for (auto iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        *iter = CMpString::Trim(*iter);
        if (protocolType == SCRIPT_INITIATOR_PARAM_FC) {
            std::string fcInitor = iter->c_str();
            int pos = fcInitor.find(STRING_0X);
            if (pos != std::string::npos) {
                fcInitor.erase(pos, STRING_0X.size());
            }
            COMMLOG(OS_LOG_DEBUG, "Get fc wwn %s.", fcInitor.c_str());
            initInfo.fcs.push_back(fcInitor);
        } else if (protocolType == SCRIPT_INITIATOR_PARAM_ISCSI) {
            COMMLOG(OS_LOG_DEBUG, "Get iscsi iqn");
            initInfo.iscsis.push_back(*iter);
        } else {
            COMMLOG(OS_LOG_ERROR, "un known protocol: %s.", protocolType.c_str());
            return MP_FAILED;
        }
        ClearString(*iter);
    }
    return MP_SUCCESS;
}

mp_int32 CHost::GetInitiators(initiator_info_t& initInfo)
{
    LOGGUARD("");
    mp_int32 iRetIscsi = GetInitiatorsByProtocol(initInfo, SCRIPT_INITIATOR_PARAM_ISCSI);
    mp_int32 iRetFc = GetInitiatorsByProtocol(initInfo, SCRIPT_INITIATOR_PARAM_FC);
    // 获取启动器需要满足的条件
    mp_bool bBothFailed = ((iRetIscsi != MP_SUCCESS) && (iRetFc != MP_SUCCESS));
    if (bBothFailed) {
        mp_bool bBothFileExist = (iRetIscsi != ERROR_COMMON_SCRIPT_FILE_NOT_EXIST &&
            iRetFc != ERROR_COMMON_SCRIPT_FILE_NOT_EXIST);
        if (bBothFileExist) {
            mp_bool bothSuccess = (iRetIscsi != ERROR_COMMON_SCRIPT_EXEC_FAILED &&
                iRetFc != ERROR_COMMON_SCRIPT_EXEC_FAILED);
            if (bothSuccess) {
                COMMLOG(OS_LOG_ERROR, "Iscsi and fc initiators are not exist.");
                return ERROR_HOST_GET_INIATOR_FAILED;
            }
            COMMLOG(OS_LOG_ERROR, "%d exec failed.", ROOT_COMMAND_SCRIPT_INIT);
            return ERROR_COMMON_SCRIPT_EXEC_FAILED;
        }
        COMMLOG(OS_LOG_ERROR, "%d is not exist.", ROOT_COMMAND_SCRIPT_INIT);
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }
    return MP_SUCCESS;
}
#endif

mp_string CHost::BuildLinkTargetParam(const mp_string& targetIp, mp_uint32 targetPort, const mp_string& chapUser)
{
    static const mp_string keyTargetIp = "targetIp=";
    static const mp_string keyTargetPort = "targetPort=";
    static const mp_string keyChapName = "chapName=";

    ostringstream oss;
    oss << keyTargetIp << targetIp << NODE_COLON << keyTargetPort << targetPort << NODE_COLON << keyChapName
        << chapUser;
    return oss.str();
}

// 连接iscsi启动器
mp_int32 CHost::LinkiScsiTarget(const Json::Value& scsiTargets)
{
    static const mp_string keyStorageIps = "storageIps";
    static const mp_string keyStoragePort = "storagePort";
    static const mp_string keyChapName = "authUser";

    mp_int32 iRet = MP_FAILED;
    mp_int32 vecCnt = 0;

    mp_uint32 storagePort;
    mp_string chapUser;
    GET_JSON_UINT32(scsiTargets, keyStoragePort, storagePort);
    GET_JSON_STRING_OPTION(scsiTargets, keyChapName, chapUser);

    vector<mp_string> vecStorageIps;
    iRet = CJsonUtils::GetJsonArrayString(scsiTargets, keyStorageIps, vecStorageIps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get json array value failed, key %s.", keyStorageIps.c_str());
        return iRet;
    }

    // check if have connected iscsi

    for (vector<mp_string>::const_iterator iter = vecStorageIps.begin(); iter != vecStorageIps.end(); ++iter) {
        ++vecCnt;
        mp_string storageIp = *iter;
        mp_string scriptParam = BuildLinkTargetParam(storageIp, storagePort, chapUser);
#ifdef WIN32
        iRet = SecureCom::SysExecScript(SCRIPT_LINK_INITIATOR, scriptParam, NULL);
        TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_LOG_LINK_ISCSITARGET);
        if (iRet == MP_SUCCESS) {
            return MP_SUCCESS;
        } else if (vecCnt == vecStorageIps.size()) {
            mp_int32 iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
            COMMLOG(OS_LOG_ERROR, "Exec script failed, return code %d, tranformed return code is %d", iRet, iRettmp);
            iRet = iRettmp;
            return iRet;
        }
#else
        CRootCaller rootCaller;
        iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_LINK_ISCISITARGET, scriptParam, NULL);
        TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_LOG_LINK_ISCSITARGET);
        if (iRet == MP_SUCCESS) {
            return MP_SUCCESS;
        } else if (vecCnt == vecStorageIps.size()) {
            COMMLOG(OS_LOG_ERROR, "link iscsi target failed, ret %d.", iRet);
            return iRet;
        }
#endif
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机FC卡的所有wwpn
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangfan zwx1151941
------------------------------------------------------------- */
mp_int32 CHost::QueryWwpns(std::vector<mp_string>& vecWwpns)
{
    LOGGUARD("");
#ifndef WIN32
    const mp_string strCmd = "cat /sys/class/fc_host/host*/port_name";
    vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec sys cmd failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    for (mp_string& item : vecRlt) {
        DBGLOG("Wwpn: %s.", item.c_str());
        // 需要去掉0x前缀
        const mp_int16 startPos = 2;
        if (item.length() <= startPos) {
            continue;
        }
        vecWwpns.push_back(item.substr(startPos));
    }

#else
    ERRLOG("Unsupport query wwpns.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机sn和subType
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : luozhao lwx1154006
------------------------------------------------------------- */
mp_int32 CHost::GetHostInfo(Json::Value& jValue)
{
    LOGGUARD("");
#ifndef WIN32
    host_info_t hostInfo;
    mp_int32 iRet = GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get host info failed, errCode:%d.", iRet);
        return iRet;
    }

    jValue["uuid"] = hostInfo.sn;
    jValue["type"] = (hostInfo.subType == BACKUP_ROLE_SANCLIENT ? "sanclient" : "general");

    return MP_SUCCESS;
#else
    ERRLOG("Unsupport query iqns.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif
}

/* ------------------------------------------------------------
Description  : 查询主机iscsi卡的所有iqn
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : luozhao lwx1154006
------------------------------------------------------------- */
mp_int32 CHost::QueryIqns(std::map<mp_string, mp_string>& mapIqns)
{
    LOGGUARD("");
#ifndef WIN32
    mp_string strParam = SCRIPT_INITIATOR_PARAM_ISCSI;
    vector<mp_string> vecIqnsResult;
    vector<mp_string> vecIqns;
    CRootCaller rootCaller;
    ostringstream scriptParam;
    scriptParam << "queryType=" << strParam << NODE_COLON;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_INIT, scriptParam.str(), &vecIqnsResult);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Rootcaller exec failed[iqns], iRet %d.", iRet);
        return iRet;
    }
    for (vector<mp_string>::iterator iter = vecIqnsResult.begin(); iter != vecIqnsResult.end(); ++iter) {
        *iter = CMpString::Trim(*iter);
        vecIqns.push_back(*iter);
        ClearString(*iter);
    }

    for (int i = 0; i < vecIqns.size(); ++i) {
        mp_string iqnState = HBA_STATUS_ONLINE;
        mapIqns.insert(std::make_pair<mp_string, mp_string>(std::move(vecIqns[i]), std::move(iqnState)));
    }

    return mapIqns.empty() ? MP_FAILED : MP_SUCCESS;
#else
    ERRLOG("Unsupport query iqns.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif
}

/* ------------------------------------------------------------
Description  : 查询主机FC卡的所有wwpn及状态
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : luozhao lwx1154006
------------------------------------------------------------- */
mp_int32 CHost::QueryWwpnsV2(std::map<mp_string, mp_string>& mapWwpns)
{
    LOGGUARD("");
#ifndef WIN32
    mp_string strParam = SCRIPT_INITIATOR_PARAM_FC;
    vector<mp_string> vecFCsResult;
    vector<mp_string> vecFCs;
    CRootCaller rootCaller;
    ostringstream scriptParam;
    scriptParam << "queryType=" << strParam;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_INIT, scriptParam.str(), &vecFCsResult);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Rootcaller exec failed[fc_port_name], iRet %d.", iRet);
        return iRet;
    }
    for (vector<mp_string>::iterator iter = vecFCsResult.begin(); iter != vecFCsResult.end(); ++iter) {
        *iter = CMpString::Trim(*iter);
        vecFCs.push_back(*iter);
        ClearString(*iter);
    }

    strParam = SCRIPT_INITIATOR_PARAM_FC_STATE;
    vector<mp_string> vecFCStateResult;
    vector<mp_string> vecFCState;
    scriptParam.str("");
    scriptParam << "queryType=" << strParam;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_INIT, scriptParam.str(), &vecFCStateResult);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Rootcaller exec failed[fc_port_state], iRet %d.", iRet);
        return iRet;
    }
    for (vector<mp_string>::iterator iter = vecFCStateResult.begin(); iter != vecFCStateResult.end(); ++iter) {
        *iter = CMpString::Trim(*iter);
        vecFCState.push_back(*iter);
        ClearString(*iter);
    }

    mp_string FCState;
    for (int i = 0; i < vecFCs.size(); ++i) {
        FCState = (vecFCState[i] == "Online" ? HBA_STATUS_ONLINE : HBA_STATUS_OFFLINE);
        mapWwpns.insert(std::make_pair<mp_string, mp_string>(std::move(vecFCs[i]), std::move(FCState)));
    }

    return mapWwpns.empty() ? MP_FAILED : MP_SUCCESS;
#else
    ERRLOG("Unsupport query wwpns.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif
}

/* ------------------------------------------------------------
Description  : 执行Dataturbo的重新扫描脚本
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangfan zwx1151941
------------------------------------------------------------- */
mp_int32 CHost::DataturboRescan()
{
    LOGGUARD("");
#ifndef WIN32
    const mp_string strCmd = "/opt/oceanstor/dataturbo/script/dataturbo_rescan";
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.ExecUserDefineScript((mp_int32)ROOT_COMMAND_USER_DEFINED, strCmd);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec sys cmd failed, iRet = %d.", iRet);
        return ERROR_DISK_SCAN_FAILED;
    }
#else
    ERRLOG("Unsupport dataturbo scan.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 扫盘
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::ScanDisk()
{
    COMMLOG(OS_LOG_DEBUG, "Begin scan disk.");
    mp_int32 iRet;
#ifdef WIN32
    DEVINST devInst;
    CONFIGRET status;

    // 得到设备树根节点
    status = CM_Locate_DevNode(&devInst, NULL, CM_LOCATE_DEVNODE_NORMAL);
    if (status != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CM_Locate_DevNode failed: %x ", status);
        return ERROR_DISK_SCAN_FAILED;
    }

    // 强制刷新设备树
    status = CM_Reenumerate_DevNode(devInst, 0);
    if (status != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CM_Reenumerate_DevNode when scan device tree failed on windows: %x ", status);
        return ERROR_DISK_SCAN_FAILED;
    }
    iRet = MP_SUCCESS;
#elif defined(LINUX)
    // hot_add
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_SCANDISK, "", NULL);
#elif defined(HP_UX_IA)
    // ioscan -fnC disk
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_IOSCANFNC, "", NULL);
#elif defined(AIX)
    // cfgmgr -v
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_CFGMGR, "", NULL);
#elif defined(SOLARIS)
    // cfgadm -al;devfsadm
    CRootCaller rootCaller;
    mp_int32 iRet1 = rootCaller.Exec((mp_int32)ROOT_COMMAND_CFGADM, "", NULL);
    mp_int32 iRet2 = rootCaller.Exec((mp_int32)ROOT_COMMAND_DEVFSADM, "", NULL);
    if ((MP_SUCCESS == iRet1) && (MP_SUCCESS == iRet2)) {
        iRet = MP_SUCCESS;
    } else {
        iRet = MP_FAILED;
    }
#else
    COMMLOG(OS_LOG_ERROR, "Unsupport scan disk.");
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Scan disk failed in this system, iRet %d.", iRet);
        return ERROR_DISK_SCAN_FAILED;
    }

#ifdef LINUX
    // hot_add执行以后到块设备文件和/proc/partitions中的信息生成不是同步的，需要等待
    // 一般时间较短，SUSE工程师建议等待30秒较为可靠
    DoSleep(WAIT_AFTER_HOT_ADD);
#endif

    COMMLOG(OS_LOG_DEBUG, "Scan disk end.");
    return MP_SUCCESS;
}

mp_int32 CHost::UpdateTrapServer(const std::vector<trap_server>& vecTrapServer, const snmp_v3_param& stParam)
{
    LOGGUARD("");
    std::vector<trap_server> vecTmp;
    for (int i = 0; i < vecTrapServer.size(); ++i) {
        if (MP_SUCCESS == CheckTrapServer(vecTrapServer.at(i))) {
            vecTmp.push_back(vecTrapServer.at(i));
        }
    }
    if (vecTmp.empty()) {
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_int32 iRet = CAlarmConfig::UpdateSnmpV3Param(stParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "UpdateSnmpV3Param failed,iRet = %d.", iRet);
        return iRet;
    }

    iRet = AlarmDB::UpdateAllTrapInfo(vecTmp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "UpdateAllTrapInfo failed,iRet = %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 注册Trap
Input        : stTrapServer 待注册的trap server
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 CHost::RegTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    mp_int32 ret = CheckTrapServer(stTrapServer);
    if (MP_SUCCESS != ret) {
        return ret;
    }

    MP_RETURN(AlarmDB::InsertTrapServer(stTrapServer), ERROR_HOST_REG_TRAPSERVER_FAILED);
}
/* ------------------------------------------------------------
Description  : 解除Trap
Input        : stTrapServer 待注册的trap server
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 CHost::UnRegTrapServer(trap_server& stTrapServer)
{
    LOGGUARD("");
    mp_int32 ret = CheckTrapServer(stTrapServer);
    if (MP_SUCCESS != ret) {
        return ret;
    }

    MP_RETURN(AlarmDB::DeleteTrapServer(stTrapServer), ERROR_HOST_UNREG_TRAPSERVER_FAILED);
}
/* ------------------------------------------------------------
Description  : 校验SNMP参数
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 CHost::VerifySnmp(snmp_v3_param& stParam)
{
    LOGGUARD("");
    // 从配置文件中读取相关信息
    snmp_v3_param stLocalParam;
    CAlarmConfig::GetSnmpV3Param(stLocalParam);
    mp_bool bRet = MP_FALSE;

    // 当Server下发的AuthProtocol为NONE时，对应的授权认证协议密钥为空。
    // 下发的PrivProtocol为NONE时，对应的数据加密协议密钥为空。
    // 所以针对以上两种情况，本地不校验密钥。
    if (stParam.iAuthProtocol == AUTH_PROTOCOL_NONE) {
        // 下发的Auth为NONE时，PrivProtocol也必须为NONE
        bRet = (PRIVATE_PROTOCOL_NONE == stLocalParam.iPrivProtocol &&
                stParam.iAuthProtocol == stLocalParam.iAuthProtocol &&
                stParam.iPrivProtocol == stLocalParam.iPrivProtocol &&
                stParam.strSecurityName == stLocalParam.strSecurityName);
        if (bRet) {
            stLocalParam.strAuthPassword.replace(0, stLocalParam.strAuthPassword.length(), "");
            stLocalParam.strPrivPassword.replace(0, stLocalParam.strPrivPassword.length(), "");
            return MP_SUCCESS;
        }
    }
    if (stParam.iPrivProtocol == PRIVATE_PROTOCOL_NONE) {
        bRet = (stParam.strAuthPassword == stLocalParam.strAuthPassword &&
                stParam.iAuthProtocol == stLocalParam.iAuthProtocol &&
                stParam.iPrivProtocol == stLocalParam.iPrivProtocol &&
                stParam.strSecurityName == stLocalParam.strSecurityName);
        if (bRet) {
            stLocalParam.strAuthPassword.replace(0, stLocalParam.strAuthPassword.length(), "");
            stLocalParam.strPrivPassword.replace(0, stLocalParam.strPrivPassword.length(), "");
            return MP_SUCCESS;
        }
    }

    bRet = ((stParam.strAuthPassword != stLocalParam.strAuthPassword) ||
            (stParam.strPrivPassword != stLocalParam.strPrivPassword) ||
            (stParam.strSecurityName != stLocalParam.strSecurityName) ||
            (stParam.iAuthProtocol != stLocalParam.iAuthProtocol) ||
            (stParam.iPrivProtocol != stLocalParam.iPrivProtocol));
    if (bRet) {
        COMMLOG(OS_LOG_INFO, "SNMP V3 paramters does not match.");
        return ERROR_HOST_VERIFY_SNMP_FAILED;
    }
    stLocalParam.strAuthPassword.replace(0, stLocalParam.strAuthPassword.length(), "");
    stLocalParam.strPrivPassword.replace(0, stLocalParam.strPrivPassword.length(), "");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询第三方脚本
Output       : vectFileList -- 三方脚本
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::QueryThirdPartyScripts(vector<mp_string>& vectFileList)
{
    LOGGUARD("");
    mp_string scriptPath = CPath::GetInstance().GetSBinPath() + mp_string(PATH_SEPARATOR) + AGENT_THIRDPARTY_DIR;
    mp_int32 iRet = CMpFile::GetFolderFile(scriptPath, vectFileList);
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_THIRDPARTY_GETFILE_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query thridparty script failed, iRet %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询块客户端IP
Output       : strFusionStorageIP -- 块客户端IP
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回空
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::QueryFusionStorageIP(vector<mp_string>& strFusionStorageIP)
{
    ERRLOG("Function is forbidden");
    return MP_FAILED;
}

mp_int32 CHost::IsSafeDirectory(const mp_string& strInput)
{
    // 检查是否包含 ".." 或者以 "/" 开头
    if (strInput.find("..") != std::string::npos || strInput.find('/') != std::string::npos
        || strInput.find('\\') != std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "IsSafeDirectory failed");
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_INFO, "IsSafeDirectory succeed");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 执行第三方脚本
Input        : fileName -- 脚本名称
                paramValues -- 脚本参数
Output       : vecResult -- 结果
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::ExecThirdPartyScript(const mp_string& fileName,
    const mp_string& paramValues, vector<mp_string>& vecResult)
{
    LOGGUARD("");
    mp_string strFileName = CMpString::Trim(fileName);
    if (strFileName == "") {
        COMMLOG(OS_LOG_ERROR, "Input Parameter file name is null.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_int32 iRet;
    mp_string strParam = CMpString::Trim(paramValues);
#ifndef WIN32
    mp_string strInput = strFileName + NODE_COLON + "1" + NODE_COLON + strParam;
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_THIRDPARTY, strInput, &vecResult);
#else
    mp_int32 iRettmp = MP_SUCCESS;
    // SecureCom::SysExecScript只获取到bin目录下，需组装至thridparty目录
    mp_string strInput = mp_string(AGENT_THIRDPARTY_DIR) + PATH_SEPARATOR + strFileName;
    // 校验是否存在跨目录
    iRet = IsSafeDirectory(strInput);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Input Parameter file name exists directory traversal and script execution risk.");
        return iRet;
    }
    // 调用时，默认参数设为FALSE，不验证脚本签名
    iRet = SecureCom::SysExecScript(strInput, strParam, &vecResult, MP_FALSE);
    if (iRet != MP_SUCCESS) {
        iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(
            OS_LOG_ERROR, "Exec script failed, initial return code is %d, tranformed return code is %d", iRet, iRettmp);
        iRet = iRettmp;
    }
#endif
    TRANSFORM_RETURN_CODE(iRet, ERROR_HOST_THIRDPARTY_EXEC_FAILED);
    if (iRet == ERROR_COMMON_SCRIPT_FILE_NOT_EXIST) {
        COMMLOG(OS_LOG_WARN, "%s is not exist.", strFileName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    if (iRet != MP_SUCCESS) {
        // 对脚本不存在错误码做特殊处理
        iRet = (iRet == INTER_ERROR_SRCIPT_FILE_NOT_EXIST) ? ERROR_COMMON_SCRIPT_FILE_NOT_EXIST : iRet;
        COMMLOG(OS_LOG_ERROR, "Rootcaller thirdparty script exec failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Exec thirdparty script succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : CollectLog
Input        : strLogName -- 收集的日志名称
Output       : 无
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : xuchong 00300551
------------------------------------------------------------- */
mp_int32 CHost::CollectLog()
{
    mp_int32 iRet = m_logCollector.CollectLog();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CollectLog failed, iRet = %d", iRet);
    }
    return iRet;
}

#ifdef WIN32
// windows 2003平台上对应磁盘没有offline选项
/* ------------------------------------------------------------
Description  : 磁盘上线
Input        : strDiskNum --磁盘编号
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 CHost::DeviceOnline(mp_string& strDiskNum)
{
    COMMLOG(OS_LOG_DEBUG, "Begin device online, disk num %s.", strDiskNum.c_str());
    CHECK_FAIL_EX(CheckCmdDelimiter(strDiskNum));
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(SCRIPT_ONLINE_WIN);
    strCmd = CMpString::BlankComma(strCmd);
    strCmd += " ";
    strCmd += strDiskNum;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_ONLINE_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec online cmd failed, cmd %s.", strCmd.c_str());
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Device online succ.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询磁盘分区
Output       : partisioninfos -- 磁盘分区信息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CHost::GetPartisions(vector<partitisions_info_t>& partisioninfos)
{
    LOGGUARD("");
    mp_int32 iRet;
    COMMLOG(OS_LOG_DEBUG, "Get disk Partisions on windows.");
    vector<sub_area_Info_t> rvecSubareaInfo;
    vector<sub_area_Info_t>::iterator iter;

    iRet = Disk::GetSubareaInfoList(rvecSubareaInfo);
    TRANSFORM_RETURN_CODE(iRet, ERROR_DISK_GET_PARTITION_INFO_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get disk subares on windows failed.");

        return iRet;
    }

    for (iter = rvecSubareaInfo.begin(); iter != rvecSubareaInfo.end(); ++iter) {
        partitisions_info_t partisioninfo;

        mp_char lbaTmp[FILESYS_NAME_LEN] = {0};
        mp_char numTmp[FILESYS_NAME_LEN] = {0};
        partisioninfo.lCapacity = iter->ullTotalCapacity;
        partisioninfo.strVolName = iter->acVolName;
        partisioninfo.strDiskNumber = iter->iDiskNum;
        partisioninfo.strPartitionName = iter->acDeviceName;
        I64ITOA(iter->llOffset, lbaTmp, FILESYS_NAME_LEN, AGENT_DECIMAL);
        ITOA(iter->iDiskNum, numTmp, FILESYS_NAME_LEN, AGENT_DECIMAL);
        partisioninfo.strLba = lbaTmp;
        partisioninfo.strDiskNumber = numTmp;

        partisioninfos.push_back(partisioninfo);
        COMMLOG(OS_LOG_DEBUG,
            "Partitisons info: capacity %lld, volname %s, partision name %s, lba %s, disk num %d.",
            partisioninfo.lCapacity,
            partisioninfo.strVolName.c_str(),
            partisioninfo.strPartitionName.c_str(),
            partisioninfo.strLba.c_str(),
            partisioninfo.strDiskNumber.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, "End Get disk Partisions on windows.");

    return iRet;
}

#endif

mp_int32 CHost::GetHostIPList(const std::vector<mp_string>& hostIpv4List, const std::vector<mp_string>& hostIpv6List,
    std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    auto iRet = CIP::GetHostIPList(ipv4List, ipv6List);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Host ip  failed");
        return iRet;
    }
    iRet = SecureCom::CIP::CheckHostLinkStatus(hostIpv4List, hostIpv6List, ipv4List, ipv6List);
    if (iRet != MP_SUCCESS) {
        ERRLOG("CheckHostLinkStatus failed, iRet = %d", iRet);
        return iRet;
    }
    mp_bool isDorado = false;
    iRet = CIP::CheckIsDoradoEnvironment(isDorado);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get DoradoEnvironment type failed");
        return iRet;
    }
    if (isDorado) {
        std::vector<mp_string> dipv4List;
        std::vector<mp_string> dipv6List;
        if (GetContainerIPList(dipv4List, dipv6List) != MP_SUCCESS) {
            ERRLOG("Get container host ip failed, iRet = %d", iRet);
        } else {
            ipv4List.insert(ipv4List.end(), dipv4List.begin(), dipv4List.end());
            ipv6List.insert(ipv6List.end(), dipv6List.begin(), dipv6List.end());
        }
    }
    return iRet;
}

mp_int32 CHost::GetContainerIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    mp_bool iRet = CMpFile::FileExist(CONTAINER_NETWORK_INFO_FILE_PATH);
    if (iRet != MP_TRUE) {
        ERRLOG("The backup netplane file does not exist.");
        return MP_FAILED;
    }
    std::vector<mp_string> networkInfoVec;
    mp_string path = CONTAINER_NETWORK_INFO_FILE_PATH;
    iRet = CMpFile::ReadFile(path, networkInfoVec);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The container network file does not exist.");
        return MP_FAILED;
    }
    mp_string nodeName = getenv("NODE_MANE");
    Json::Value networkInfo;
    // /opt/network-conf/backup_net_plane为一行json字符串
    if (networkInfoVec.empty() || !JsonHelper::JsonStringToJsonValue(networkInfoVec[0], networkInfo)) {
        return MP_FAILED;
    }
    std::vector<Json::Value> vecValues;
    if (!networkInfo.isArray()) {
        DBGLOG("NetworkInfo is not array");
        return MP_FAILED;
    }
    GET_ARRAY_JSON(networkInfo, vecValues);
    for (const auto& item : vecValues) {
        if (!item.isObject() || !item.isMember("nodeId") || !item["nodeId"].isString()) {
            DBGLOG("nodeId is not string");
            return MP_FAILED;
        }

        if (item["nodeId"].asString() != nodeName) {
            continue;
        }

        if (!item.isMember("ips") || !item["ips"].isArray()) {
            DBGLOG("ips is not array");
            return MP_FAILED;
        }
        std::vector<mp_string> ipInfos;
        GET_ARRAY_STRING_WITHOUT_BRACES(item["ips"], ipInfos);
        for (const auto& ipInfo : ipInfos) {
            if (CIP::IsIPV4(ipInfo)) {
                ipv4List.push_back(ipInfo);
            }
            
            if (CIP::IsIPv6(ipInfo)) {
                ipv6List.push_back(ipInfo);
            }
        }
    }
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: GetHBAInfo
Description  : 获取win下HB卡上的WWN号
Modification : zwx373611 2016/09/14
Others       :------------------------------------------------------------- */
#ifdef WIN32
CHBA::CHBA()
{
    m_hbadll = NULL;
}

CHBA::~CHBA()
{
    FreeHBAdllModule();
}

mp_bool CHBA::InitDll()
{
    if (m_hbadll != NULL) {
        COMMLOG(OS_LOG_DEBUG, "%s", "hbaapidll handle is exists.");
        return MP_TRUE;
    }
    mp_char strSystemPath[MAX_PATH_LEN] = {0};
    mp_int32 lenPath = GetSystemDirectory(strSystemPath, MAX_PATH_LEN);
    if (lenPath == 0) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[HBA_NUM_256] = {0};
        COMMLOG(OS_LOG_ERROR,
            "Get system directory failed, errno [%d]: %s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FALSE;
    }

    mp_string tmpSystemPath = strSystemPath;
    mp_string ntdllPath = tmpSystemPath + "\\hbaapi.dll";
    m_hbadll = LoadLibrary(TEXT(ntdllPath.c_str()));
    if (m_hbadll == NULL) {
        COMMLOG(OS_LOG_ERROR, "LoadHbaApidllModule failed [%ld]", GetLastError());
        return MP_FALSE;
    }
    return MP_TRUE;
}
mp_bool CHBA::LoadHBAdllModule()
{
    if (MP_FALSE == InitDll()) {
        return MP_FALSE;
    }

    Hbafun_GetNumber = (Hba_GetNumberOfAdapters)GetProcAddress(m_hbadll, "HBA_GetNumberOfAdapters");
    if (Hbafun_GetNumber == NULL) {
        COMMLOG(OS_LOG_ERROR, "Hbafun_GetNumber null, errorcode(%d).", GetLastError());
        FreeHBAdllModule();
        return MP_FALSE;
    }

    Hbafun_GetAdapterName = (Hba_GetAdapterName)GetProcAddress(m_hbadll, "HBA_GetAdapterName");
    if (Hbafun_GetAdapterName == NULL) {
        COMMLOG(OS_LOG_ERROR, "Hbafun_GetAdapterName null, errorcode(%d).", GetLastError());
        FreeHBAdllModule();
        return MP_FALSE;
    }

    Hbafun_OpenAdapter = (Hba_OpenAdapter)GetProcAddress(m_hbadll, "HBA_OpenAdapter");
    if (Hbafun_OpenAdapter == NULL) {
        COMMLOG(OS_LOG_ERROR, "Hbafun_OpenAdapter null, errorcode(%d).", GetLastError());
        FreeHBAdllModule();
        return MP_FALSE;
    }

    Hbafun_CloseAdapter = (Hba_CloseAdapter)GetProcAddress(m_hbadll, "HBA_CloseAdapter");
    if (Hbafun_CloseAdapter == NULL) {
        COMMLOG(OS_LOG_ERROR, "Hbafun_CloseAdapter null, errorcode(%d).", GetLastError());
        FreeHBAdllModule();
        return MP_FALSE;
    }

    Hbafun_GetAdapterAttributes = (Hba_GetAdapterAttributes)GetProcAddress(m_hbadll, "HBA_GetAdapterAttributes");
    if (Hbafun_GetAdapterAttributes == NULL) {
        COMMLOG(OS_LOG_ERROR, "Hbafun_GetAdapterAttributes null, errorcode(%d).", GetLastError());
        FreeHBAdllModule();
        return MP_FALSE;
    }

    Hbafun_GetAdapterPortAttributes = (Hba_GetAdapterPortAttributes)GetProcAddress(
        m_hbadll, "HBA_GetAdapterPortAttributes");
    if (Hbafun_GetAdapterPortAttributes == NULL) {
        COMMLOG(OS_LOG_ERROR, "Hbafun_GetAdapterPortAttributes null, errorcode(%d).", GetLastError());
        FreeHBAdllModule();
        return MP_FALSE;
    }

    return MP_TRUE;
}

mp_void CHBA::FreeHBAdllModule()
{
    if (m_hbadll) {
        FreeLibrary(m_hbadll);
        m_hbadll = NULL;
    }
}

mp_void CHBA::HandleHBA(vector<mp_string>& pvecResult, mp_int32 hbaCount)
{
    mp_string stradaptername;

    if (Hbafun_GetAdapterName(hbaCount, const_cast<char*>(stradaptername.c_str())) != 0) {
        return;
    }
    HANDLE hba_handle = Hbafun_OpenAdapter(const_cast<char*>(stradaptername.c_str()));
    if (hba_handle == 0) {
        return;
    }

    HBA_ADAPTERATTRIBUTES strHbaAdapterAttributes;
    if (Hbafun_GetAdapterAttributes(hba_handle, &strHbaAdapterAttributes) != 0) {
        Hbafun_CloseAdapter(hba_handle);
        return;
    }

    mp_int32 hbaPort;
    mp_string PortWWN;
    mp_char buf[HBA_NUM_32];
    mp_bool bIsUnit = MP_FALSE;

    for (hbaPort = 0; hbaPort < strHbaAdapterAttributes.NumberOfPorts; ++hbaPort) {
        HBA_PORTATTRIBUTES hbaPortAttrs;
        if ((Hbafun_GetAdapterPortAttributes(hba_handle, hbaPort, &hbaPortAttrs)) != 0) {
            continue;
        }
        for (mp_int32 i = 0; i < HBA_NUM_8; ++i) {
            bIsUnit = hbaPortAttrs.PortWWN.wwn[i] >= 0x00 && hbaPortAttrs.PortWWN.wwn[i] <= 0x0F;
            if (bIsUnit) {
                PortWWN += "0";
                ITOA(hbaPortAttrs.PortWWN.wwn[i], buf, sizeof(buf), HBA_NUM_16);
                PortWWN += buf;
            } else {
                ITOA(hbaPortAttrs.PortWWN.wwn[i], buf, sizeof(buf), HBA_NUM_16);
                PortWWN += buf;
            }
        }
        pvecResult.push_back(PortWWN.c_str());
    }

    Hbafun_CloseAdapter(hba_handle);
}

mp_int32 CHBA::GetHBAInfo(vector<mp_string>& pvecResult)
{
    VARIANT vtMfgDomain;
    VARIANT vtModel;
    VARIANT vtPortWWN;
    VariantInit(&vtMfgDomain);
    VariantInit(&vtModel);
    VariantInit(&vtPortWWN);
    mp_int32 startSize = pvecResult.size();

    if (!LoadHBAdllModule()) {
        COMMLOG(OS_LOG_ERROR, "LoadHbaApidllModule failed.");
        return MP_FAILED;
    }

    mp_int32 numberOfAdapters = Hbafun_GetNumber();
    mp_int32 hbaCount;

    for (hbaCount = 0; hbaCount < numberOfAdapters; ++hbaCount) {
        HandleHBA(pvecResult, hbaCount);
    }

    FreeHBAdllModule();

    if (pvecResult.size() <= startSize) {
        COMMLOG(OS_LOG_ERROR, "HbaAdapter is not exists.");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
#endif

/*------------------------------------------------------------
Description  : 更新PM的业务ip
Input        : ip列表
Output       : 无
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangbangwei zwx1059021
-------------------------------------------------------------*/
mp_int32 CHost::UpdateManagerServer(const std::vector<mp_string>& vecManagerServer)
{
    INFOLOG("Begin update manager server.");
    mp_string ipStr;
    // 对ip列表去重
    std::set<mp_string> tmpManagerServer(vecManagerServer.begin(), vecManagerServer.end());
    std::set<mp_string>::const_iterator iter = tmpManagerServer.begin();
    for (; iter != tmpManagerServer.end(); ++iter) {
        mp_string tempIp = *iter;
        if ((CheckIpAddressValid(tempIp) == MP_SUCCESS)) {
            if (ipStr.empty()) {
                ipStr.append(*iter);
            } else {
                ipStr.append(",").append(*iter);
            }
        }
    }
    if (ipStr.empty()) {
        ERRLOG("No valid ip.");
        return MP_FAILED;
    }
    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, ipStr);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Write configure failed.");
        return iRet;
    }

    mp_string filePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    iRet = ModifyLineData(filePath, PARAM_KEY_PM_IP, ipStr);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to modify the configuration.");
        return iRet;
    }
    return iRet;
}

mp_int32 CHost::GetEipInfo(mp_string& eip)
{
    mp_int32 iRet = CIP::GetHostEip(eip);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}


mp_int32 CHost::GetFloatingIp(mp_string& floatingIp)
{
    mp_int32 iRet = CIP::GetFloatingIp(floatingIp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CHost::GetAvailableZone(mp_string& az)
{
    mp_int32 iRet = CIP::GetAvailableZone(az);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CHost::GetIsSharedAgent(mp_string& isShared)
{
    mp_int32 iRet = CIP::GetIsSharedAgent(isShared);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    INFOLOG("Get is shared agent success, isShared=%s", isShared.c_str());
    return MP_SUCCESS;
}

mp_int32 CHost::GetInstallMode(mp_string& mode)
{
    mp_int32 iRet = CIP::GetInstallMode(mode);
    if (iRet != MP_SUCCESS) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    INFOLOG("Get installation mode success, mode=%s", mode.c_str());
    return MP_SUCCESS;
}

/* ---------------------------------------------------------------------------
Function Name: CollectLog
Description  : 日志收集处理函数
Others       :------------------------------------------------------------- */
mp_int32 CLogCollector::CollectLog()
{
    LOGGUARD("");
    // 判断状态
    if (GetStatus() == LOG_COLLECTING) {
        COMMLOG(OS_LOG_INFO, "Logcollector is processing, status is %d.", m_status);
        // 日志收集失败，返回特性错误码
        return ERROR_HOST_LOG_IS_BEENING_COLLECTED;
    }

    SetStatus(LOG_COLLECTING);
    SetLogName(GenerateLogName());
    m_isCollect = true;

    // 创建处理线程
    if (m_hThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_hThread, NULL);
    }
    (mp_void) memset_s(&m_hThread, sizeof(m_hThread), 0, sizeof(m_hThread));
    return CMpThread::Create(&m_hThread, LogCollectThread, this);
}

mp_int32 CLogCollector::CleanCollectedLogFile(const mp_string &logId)
{
    LOGGUARD("");
    if (GetStatus() == LOG_COLLECTING) {
        COMMLOG(OS_LOG_INFO, "Logcollector is processing, status is %d.", m_status);
        return ERROR_HOST_LOG_IS_BEENING_COLLECTED;
    }

    SetStatus(LOG_COLLECTING);
    SetLogName(FormatLogNameFromLogId(logId));
    m_isCollect = false;

    // 创建处理线程
    if (m_hThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_hThread, NULL);
    }
    (mp_void) memset_s(&m_hThread, sizeof(m_hThread), 0, sizeof(m_hThread));
    return CMpThread::Create(&m_hThread, LogCollectThread, this);
}

mp_string CLogCollector::GenerateLogName()
{
    // 根据时间戳生成日志名称
    mp_time nowTime;
    CMpTime::Now(nowTime);
    mp_string strNowTime = CMpTime::GetTimeString(nowTime);
    mp_string strTime;
    for (mp_uint32 i = 0; i < strNowTime.length(); ++i) {
        mp_bool bFlag = strNowTime[i] >= '0' && strNowTime[i] <= '9';
        if (bFlag) {
            strTime.push_back(strNowTime[i]);
        }
    }
    /* LOG FILE FORMAT:
     * WIN: AGENTLOG_{UUID}_{TIMESTAMP}.zip
     * LINUX: AGENTLOG_{UUID}_{TIMESTAMP}.tar.gz
     */
    mp_string uuid;
    if (CUuidNum::GetUuidStandardStr(uuid) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Generate uuid failed.");
        return mp_string();
    }
    mp_string logId = uuid + SIGN_UNDERLINE + strTime;
    COMMLOG(OS_LOG_INFO, "Set log id: %s", logId.c_str());
    SetLogId(logId);
    return FormatLogNameFromLogId(logId);
}

mp_string CLogCollector::FormatLogNameFromLogId(const mp_string &logId)
{
    mp_string m_HostIp;
    mp_string listenPort;
    CIP::GetListenIPAndPort(m_HostIp, listenPort);
    mp_string strLogName = mp_string(AGENT_LOG_ZIP_NAME) + SIGN_UNDERLINE + logId
        + SIGN_UNDERLINE + m_HostIp + ZIP_SUFFIX;
    COMMLOG(OS_LOG_INFO, "Log File: %s", strLogName.c_str());
    return strLogName;
}

/* ---------------------------------------------------------------------------
Function Name: LogCollectThread
Description  : 日志收集线程处理函数
Others       :------------------------------------------------------------- */
#ifdef WIN32
DWORD WINAPI CLogCollector::LogCollectThread(LPVOID param)
#else
mp_void* CLogCollector::LogCollectThread(mp_void* param)
#endif
{
    LOGGUARD("");
    CLogCollector* pLogCollector = static_cast<CLogCollector*>(param);
    mp_int32 iRet = SecureCom::PackageLog(pLogCollector->GetLogName(), pLogCollector->IsCollect());
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "PackageLog failed, iRet = %d.", iRet);
        pLogCollector->SetStatus(LOG_FAILED);
        pLogCollector->SetErrCode(iRet);
    }
    pLogCollector->SetStatus(LOG_COMPLETED);
    mp_string logMsg = pLogCollector->IsCollect() ? "Collect" : "Clean";
    COMMLOG(OS_LOG_INFO, "%s log success.", logMsg.c_str());

#ifdef WIN32
    return MP_SUCCESS;
#else
    return NULL;
#endif
}

/* ------------------------------------------------------------
Description  : Agent资源占用量查询
Output       : hostInfo -- 主机信息结构体
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : gongqinhua 30046491
------------------------------------------------------------- */
mp_int32 CHost::GetAgentResourceUsage(ResourceUageInfoT& ResourceUsageInfo)
{
    host_info_t hostInfo;
    mp_int32 iRet = GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host Info failed, errCode:%d.", iRet);
        return ERROR_HOST_GETINFO_FAILED;
    }
    ResourceUsageInfo.sn = hostInfo.sn;
    if (hostInfo.subType == BACKUP_ROLE_SANCLIENT) {
        m_userName = "sancli";
        ResourceUsageInfo.sn += "_sanclient";
    }

#if defined(WIN32)
    iRet = AccumulateResourceUsageWin(ResourceUsageInfo);
#elif defined(LINUX)
    iRet = AccumulateResourceUsageLinux(ResourceUsageInfo);
#elif defined(_AIX)
    iRet = AccumulateResourceUsageLinux(ResourceUsageInfo);
#elif defined(SOLARIS)
    iRet = AccumulateResourceUsageLinux(ResourceUsageInfo);
#else
    COMMLOG(OS_LOG_INFO, "Agent resource usage not supported.");
    return MP_FAILED;
#endif

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get resource usage failed, errCode:%d.", iRet);
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "Get resource usage success");
    return MP_SUCCESS;
}

mp_int32 CHost::CutString(const mp_string& strToCut, std::vector<mp_string>& strCache)
{
    stringstream strStream(strToCut);
    mp_string strCut;
    while (strStream >> strCut) {
        strCache.emplace_back(strCut);
    }
    return MP_SUCCESS;
}

mp_int32 CHost::CheckStringToDouble(const mp_string& stringToDouble, double& resDouble)
{
    if (stringToDouble.size() == 0) {
        ERRLOG("convert string to double failed, the string is empty.");
        return MP_FAILED;
    }
    if (stringToDouble[0] == '.' || stringToDouble[stringToDouble.size() - 1] == '.') {
        ERRLOG("convert string to double failed, the fromat is wrong.");
        return MP_FAILED;
    }
    mp_int32 pointNum = 0;
    for (auto i : stringToDouble) {
        if (i >= '0' && i <= '9') {
            continue;
        } else if (i == '.') {
            pointNum += 1;
            if (pointNum > 1) {
                ERRLOG("convert string to double failed, the fromat is wrong.");
                return MP_FAILED;
            }
            continue;
        } else {
            ERRLOG("convert string to double failed, the fromat is wrong.");
            return MP_FAILED;
        }
    }
    resDouble = CMpString::SafeStod(stringToDouble);
    if (resDouble >= MIN_PERCENT && resDouble <= MAX_PERCENT) {
        return MP_SUCCESS;
    }
    WARNLOG("The cpu or mem usage rate is out of range [0, 100].");
    return MP_SUCCESS;
}

#ifndef WIN32
mp_int32 CHost::AccumulateResourceUsageLinux(ResourceUageInfoT& ResourceUsageInfo)
{
    mp_int32 iRet = MP_FAILED;

    std::vector<mp_string> vecResult;
    for (const auto& processName : PROCESS_NAME_OF_AGENT) {
        mp_string strCmd = "ps -eo user,pid,pcpu,pmem,args | grep -v grep | grep "
            + m_userName + " | grep " + processName;
        iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecResult);
        if (iRet != MP_SUCCESS) {
            ERRLOG("check 'ps' command may not exist, iRet is %d.", iRet);
            return MP_FAILED;
        }
    }
    if (m_userName == "rdadmin") {
        for (const auto& processName : PROCESS_NAME_OF_PLUGIN) {
            mp_string strCmd = "ps -eo user,pid,pcpu,pmem,args | grep -v grep | grep " + processName;
            iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecResult);
            if (iRet != MP_SUCCESS) {
                ERRLOG("check 'ps' command may not exist, iRet is %d.", iRet);
                return MP_FAILED;
            }
        }
    }

    if (CalculateCpuAndMem(vecResult, ResourceUsageInfo.cpuUsage, ResourceUsageInfo.memUsage) != MP_SUCCESS) {
        ERRLOG("Calculat usage rate failed!");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CHost::CalculateCpuAndMem(const std::vector<mp_string> &vecResult, mp_double &resCpu, mp_double &resMem)
{
    mp_int32 iRet = MP_FAILED;
    for (auto strResult : vecResult) {
        std::vector<mp_string> strResultCut;
        iRet = CutString(strResult, strResultCut);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "cutString failed, iRet is %d.", iRet);
            return MP_FAILED;
        }
        mp_int32 vecSize = strResultCut.size();
        COMMLOG(OS_LOG_DEBUG, "str is [%s].", strResult.c_str());
        // 小于5为无效打印 需过滤
        if (vecSize >= MIN_VALID_COLUMN) {
            mp_double resCpuTmp = 0;
            mp_double resMemTmp = 0;
            mp_int32 iRetCpu = CheckStringToDouble(strResultCut[CPU_COLUMN], resCpuTmp);
            mp_int32 iRetMem = CheckStringToDouble(strResultCut[MEM_COLUMN], resMemTmp);
            if (iRetCpu != MP_SUCCESS || iRetMem != MP_SUCCESS) {
                COMMLOG(OS_LOG_WARN, "string to double failed.");
                resCpuTmp = 0;  // 出现异常数据则置为0
                resMemTmp = 0;
            }
            resCpu += resCpuTmp;
            resMem += resMemTmp;
        }
    }
    mp_int32 cpuCoreNum = sysconf(_SC_NPROCESSORS_CONF);
    resCpu = resCpu / cpuCoreNum;
    CheckRange(resCpu);
    CheckRange(resMem);
    DBGLOG("CPU: %lf, MEM: %lf, cpuNum: %d", resCpu, resMem, cpuCoreNum);
    return MP_SUCCESS;
}
#endif

void CHost::CheckRange(mp_double &numOfRate)
{
    if (numOfRate < MIN_PERCENT) {
        WARNLOG("The number of rate is lower then %lf.", MIN_PERCENT);
        numOfRate = MIN_PERCENT;
        return;
    } else if (numOfRate > MAX_PERCENT) {
        WARNLOG("The number of rate is higher then %lf.", MAX_PERCENT);
        numOfRate = MAX_PERCENT;
        return;
    }
}

#ifdef WIN32
mp_void CHost::GetPIDFromSnapshot(const LPCTSTR& szProcessName, const HANDLE& hSnapshot, PROCESSENTRY32& processInfo)
{
    mp_uint32 pid = 0;
    if (Process32First(hSnapshot, &processInfo)) {
        do {
            if (_tcsicmp(processInfo.szExeFile, szProcessName) == 0) {
            pid = processInfo.th32ProcessID;
            m_vecProcessID.push_back(pid);            // 这个地方同名进程有可能不止一个
            }
        } while (Process32Next(hSnapshot, &processInfo));
    }
}

mp_int32 CHost::GetPIDByName(const LPCTSTR& szProcessName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 processInfo = { sizeof(processInfo) };
        GetPIDFromSnapshot(szProcessName, hSnapshot, processInfo);
        CloseHandle(hSnapshot);
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 CHost::GetPid()
{
    mp_int32 iRet;
    for (int i = 0; i < PROCESS_NAME_OF_AGENT_WIN.size(); ++i) {
        string temp = PROCESS_NAME_OF_AGENT_WIN[i];
        iRet = GetPIDByName(temp.c_str());
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get process snapshot failed. iRet is %d", iRet);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int64 CHost::FileTimeToInt64(const FILETIME& time)
{
    ULARGE_INTEGER timeToInt;
    timeToInt.LowPart = time.dwLowDateTime;
    timeToInt.HighPart = time.dwHighDateTime;
    return timeToInt.QuadPart;
}

mp_int32 CHost::GetCpuNumber()
{
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return (mp_int32)info.dwNumberOfProcessors;
}

mp_int32 CHost::GetCpuTime(const mp_uint32 processID, std::vector<mp_int64>& vecTime, std::vector<mp_int64>& vecSysTime)
{
    FILETIME now;
    FILETIME creationTime;
    FILETIME exitTime;
    FILETIME kernelTime;
    FILETIME userTime;
    mp_int64 sysTime = 0;
    mp_int64 time = 0;

    GetSystemTimeAsFileTime(&now);

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, processID);
    if (!hProcess) {
        vecTime.emplace_back(0);
        vecSysTime.emplace_back(0);
        COMMLOG(OS_LOG_WARN, "Open process failed.");
        return MP_FAILED;
    }
    if (!GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
        vecTime.emplace_back(0);
        vecSysTime.emplace_back(0);
        CloseHandle(hProcess);
        COMMLOG(OS_LOG_WARN, "Get process time failed.");
        return MP_FAILED;
    }
    sysTime = (FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime));  // CPU使用时间  没有除CPU总数
    time = FileTimeToInt64(now);
    vecSysTime.emplace_back(sysTime);
    vecTime.emplace_back(time);

    CloseHandle(hProcess);
    return MP_SUCCESS;
}

mp_int32 CHost::GetCpuUsage(std::vector<mp_uint32>& processID, mp_double& res)
{
    mp_int32 iRet;
    mp_int32 numOfCpu = GetCpuNumber();
    vector<mp_int64> lastTime;
    vector<mp_int64> lastTimeSys;
    vector<mp_int64> newTime;
    vector<mp_int64> newTimeSys;

    for (auto pid : processID) {
        iRet = GetCpuTime(pid, lastTime, lastTimeSys);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Get CPU time failed, PID is %d, iRet is %d.", pid, iRet);
        }
    }

    Sleep(ONE_SECOND);    // 等待一秒

    for (auto pid : processID) {
        iRet = GetCpuTime(pid, newTime, newTimeSys);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Get CPU time failed, PID is %d, iRet is %d.", pid, iRet);
        }
    }

    for (int i = 0; i < processID.size(); ++i) {
        mp_double cpuUsage = 0;
        mp_double cpuTime = (mp_double)(newTimeSys[i] - lastTimeSys[i]);
        mp_double allSysTime = (mp_double)(newTime[i] - lastTime[i]);
        if (allSysTime > 0) {             // 判断是为了避免由于其他故障导致或巧合导致 PID 找不到的情况
            cpuUsage = cpuTime / allSysTime / (mp_double)numOfCpu * PERCENT;   //  结果带百分号
        }
        res += cpuUsage;
    }
    return MP_SUCCESS;
}

mp_double CHost::GetMemUsage(const mp_uint32 ProcessID)
{
    MEMORYSTATUSEX totalmemInfo;
    totalmemInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&totalmemInfo);
    mp_uint64 totalPhys = totalmemInfo.ullTotalPhys / 1024 / 1024;  // 单位：MB

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, false, ProcessID);
    PROCESS_MEMORY_COUNTERS workingMemInfo;

    mp_double memUsage = 0;
    if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&workingMemInfo, sizeof(workingMemInfo))) {
        mp_uint64 workingPhys = workingMemInfo.WorkingSetSize / 1024 / 1024;    // MB
        COMMLOG(OS_LOG_DEBUG, "mem = %d MB.", workingPhys);
        memUsage = (mp_double)workingPhys / (mp_double)totalPhys * PERCENT;    // 结果带百分号
    }
    CloseHandle(hProcess);

    return memUsage;
}

mp_int32 CHost::AccumulateResourceUsageWin(ResourceUageInfoT& ResourceUsageInfo)
{
    mp_int32 iRet = GetPid();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PID usage failed.");
        return MP_FAILED;
    }
    iRet = GetCpuUsage(m_vecProcessID, ResourceUsageInfo.cpuUsage);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get CPU usage failed.");
        return MP_FAILED;
    }
    for (auto pid : m_vecProcessID) {
        ResourceUsageInfo.memUsage += GetMemUsage(pid);
    }
    CheckRange(ResourceUsageInfo.cpuUsage);
    CheckRange(ResourceUsageInfo.memUsage);
    COMMLOG(OS_LOG_DEBUG, "CPU is %lf, MEM is %lf.", ResourceUsageInfo.cpuUsage, ResourceUsageInfo.memUsage);
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  : 查询主机总体资源使用率
Output       : cpu rate & mem rate
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : gongqinhua 30046491
------------------------------------------------------------- */
mp_int32 CHost::GetCpuAndMemRate(SysResourceUsageRestT& sysResourceUsage)
{
    COMMLOG(OS_LOG_DEBUG, "Start to get system rest resource rate!");
    mp_int32 iRet = GetCpuUsageRate(sysResourceUsage.sysCpuRateRest);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get system cpu rate failed!");
        return iRet;
    }
    iRet = GetMemUsageRate(sysResourceUsage.sysMemRateRest, sysResourceUsage.sysMemRest);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    return MP_SUCCESS;
}

mp_int32 CHost::GetCpuUsageRate(mp_double& sysCpuRateRest)
{
    COMMLOG(OS_LOG_DEBUG, "Start to get system rest cpu rate!");
    mp_int32 iRet = MP_FAILED;
#ifdef WIN32
    iRet = GetCpuUsageRateWin(sysCpuRateRest);
#elif defined(LINUX)
    iRet = GetCpuUsageRateLinux(sysCpuRateRest);
#else
    COMMLOG(OS_LOG_WARN, "This system has not been surpported yet.");
    return MP_FAILED;
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get system cpu usage rate failed.");
        return iRet;
    }
    return MP_SUCCESS;
}

mp_int32 CHost::GetMemUsageRate(mp_double& sysMemRateRest, mp_uint64& sysAvailMem)
{
    COMMLOG(OS_LOG_DEBUG, "Start to get system rest memory rate!");
    mp_int32 iRet = MP_FAILED;
#ifdef WIN32
    iRet = GetMemUsageRateWin(sysMemRateRest, sysAvailMem);
#elif defined(LINUX)
    iRet = GetMemUsageRateLinux(sysMemRateRest, sysAvailMem);
#else
    COMMLOG(OS_LOG_WARN, "This system has not been surpported yet.");
    return MP_FAILED;
#endif
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get system memory usage rate failed.");
        return iRet;
    }
    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 CHost::GetCpuUsageRateWin(mp_double& sysCpuRateRest)
{
    FILETIME preIdleTime;
    FILETIME preKernelTime;
    FILETIME preUserTime;
    GetSystemTimes(&preIdleTime, &preKernelTime, &preUserTime);
    mp_int64 preTotalTime = (FileTimeToInt64(preKernelTime) + FileTimeToInt64(preUserTime));
    mp_int64 preAvilTime = FileTimeToInt64(preIdleTime);

    Sleep(ONE_SECOND);

    FILETIME idleTime;
    FILETIME kernelTime;
    FILETIME userTime;
    GetSystemTimes(&idleTime, &kernelTime, &userTime);
    mp_int64 totalTime = (FileTimeToInt64(kernelTime) + FileTimeToInt64(userTime));
    mp_int64 avilTime = FileTimeToInt64(idleTime);

    if (totalTime - preTotalTime <= 0) {
        COMMLOG(OS_LOG_ERROR, "The cpu time is invalid");
        return MP_FAILED;
    }

    sysCpuRateRest = (double)(avilTime - preAvilTime) / (double)(totalTime - preTotalTime)  * PERCENT;
    COMMLOG(OS_LOG_DEBUG, "Get system rest cpu rate = %lf", sysCpuRateRest);
    return MP_SUCCESS;
}

mp_int32 CHost::GetMemUsageRateWin(mp_double& sysMemRateRest, mp_uint64& sysAvailMem)
{
    MEMORYSTATUSEX sysMemInfo;
    sysMemInfo.dwLength = sizeof(sysMemInfo);
    GlobalMemoryStatusEx(&sysMemInfo);
    sysAvailMem = sysMemInfo.ullAvailPhys / SIZE_MB;    // 单位为MB
    mp_uint64 sysTotalMem = sysMemInfo.ullTotalPhys / SIZE_MB;

    if (sysTotalMem <= 0) {
        COMMLOG(OS_LOG_ERROR, "The memory info is not valid!");
        return MP_FAILED;
    }
    sysMemRateRest = (double)sysAvailMem / (double)sysTotalMem * PERCENT;
    COMMLOG(OS_LOG_DEBUG, "Get system rest memory rate = %lf", sysMemRateRest);
    return MP_SUCCESS;
}

#elif defined(LINUX)
mp_int32 CHost::GetCpuInfo(vector<mp_double>& vecCpuTime)
{
    COMMLOG(OS_LOG_DEBUG, "Start to open file /proc/stat.");
    ifstream file(CPU_FILE_NAME_LINUX); // 打开/proc/stat文件
    string line;
    getline(file, line);
    file.close();
    COMMLOG(OS_LOG_DEBUG, "Close file /proc/stat.");

    if (line.size() < SIZE_OF_CPU) {
        COMMLOG(OS_LOG_ERROR, "the string is empty", time);
        return MP_FAILED;
    }
    stringstream cpuinfo(line);
    cpuinfo.ignore(SIZE_OF_CPU);
    mp_double time;
    while (cpuinfo >> time) {
        vecCpuTime.push_back(time);
    }
    return MP_SUCCESS;
}
mp_int32 CHost::GetCpuUsageRateLinux(mp_double& sysCpuRateRest)
{
    mp_double cpuTotal = 0;
    mp_double cpuAvail = 0;
    if (!CMpFile::FileExist(CPU_FILE_NAME_LINUX)) {
        COMMLOG(OS_LOG_WARN, "Cpuinfo file (/proc/stat) not exist.");
        return MP_FAILED;
    }
    vector<mp_double> vecCpuTimeBefore;
    mp_int32 iRet1 = GetCpuInfo(vecCpuTimeBefore);
    sleep(1); // 等待一秒
    vector<mp_double> vecCpuTimeAfter;
    mp_int32 iRet2 = GetCpuInfo(vecCpuTimeAfter);
    if (vecCpuTimeAfter.size() < VALID_TIME_MUNBER || vecCpuTimeBefore.size() < VALID_TIME_MUNBER) {
        COMMLOG(OS_LOG_ERROR, "Cpuinfo file (/proc/stat) is not valid.");
        return MP_FAILED;
    }
    mp_double idleBefore = vecCpuTimeBefore[LOCATION_IDLE_TIME];
    mp_double idleAfter = vecCpuTimeAfter[LOCATION_IDLE_TIME];

    mp_double totalTime = 0;
    for (mp_int32 i = 0; i < VALID_TIME_MUNBER; ++i) {
        totalTime += (vecCpuTimeAfter[i] - vecCpuTimeBefore[i]);
    }
    if (totalTime <= 0) {
        COMMLOG(OS_LOG_ERROR, "Cpuinfo file (/proc/stat) not valid, total cpu time error. %d", totalTime);
        return MP_FAILED;
    }
    sysCpuRateRest = (idleAfter - idleBefore) / totalTime * PERCENT;
    COMMLOG(OS_LOG_DEBUG, "sysCpuRateRest = %lf.", sysCpuRateRest);
    return MP_SUCCESS;
}

mp_int32 CHost::GetMemUsageRateLinux(mp_double& sysMemRateRest, mp_uint64& memAvail)
{
    mp_double memTotal = 0;
    if (!CMpFile::FileExist(MEM_FILE_NAME_LINUX)) {
        COMMLOG(OS_LOG_WARN, "Meminfo file(/proc/meminfo) not exist.");
        return MP_FAILED;
    }
    vector<mp_string> vecMemInfo;
    COMMLOG(OS_LOG_DEBUG, "Start to open file /proc/meminfo.");
    ifstream file(MEM_FILE_NAME_LINUX); // 打开/proc/meminfo文件
    string line;
    mp_int32 count = 0;
    while (getline(file, line)) {
        vecMemInfo.push_back(line);
        ++count;
        if (count > MEM_INFO_FILE_VALID_COUNT) {
            break;
        }
    }
    file.close();
    COMMLOG(OS_LOG_DEBUG, "Close file /proc/meminfo.");

    for (auto line : vecMemInfo) {
        if (line.compare(0, SIZE_OF_MEMTOTAL, "MemTotal:") == 0) {
            memTotal = CMpString::SafeStoll(line.substr(SIZE_OF_MEMTOTAL + 1)) / SIZE_KB;
        } else if (line.compare(0, SIZE_OF_MEMAVAILABLE, "MemAvailable:") == 0) {
            memAvail = CMpString::SafeStoll(line.substr(SIZE_OF_MEMTOTAL + 1)) / SIZE_KB;      // 单位MB
        }
    }
    sysMemRateRest = (double)memAvail / memTotal * PERCENT;
    COMMLOG(OS_LOG_DEBUG, "sysMemRateRest = %lf, sysMemRest = %d", sysMemRateRest, memAvail);
    return MP_SUCCESS;
}
#endif