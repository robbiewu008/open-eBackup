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
#ifndef __AGENT_HOST_H__
#define __AGENT_HOST_H__

#include <vector>
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"

#include "common/Types.h"
#include "common/CMpThread.h"
#include "common/Defines.h"
#include "alarm/alarmdb.h"
#include "array/array.h"
#ifdef WIN32
#include <windows.h>
#include <Psapi.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "cfgmgr32.h"
#include "setupapi.h"
#else
#include <unistd.h>
#include <sys/sysinfo.h>
#endif

static const mp_int32 g_bytes_256 = 256;
static const mp_int32 g_bytes_64 = 64;
static const mp_int32 g_bytes_32 = 32;
static const mp_int32 g_bytes_8 = 8;
static const mp_int32 g_bytes_30 = 8;
static const mp_int32 g_bytes_1000 = 8;

static const mp_string AGENT_INSTALL_MODE = "push";

static const mp_int32 WAIT_AFTER_HOT_ADD = g_bytes_30 * g_bytes_1000;
static const mp_uchar MAX_SYS_VERSION_LENGTH = 10;  // 系统版本最大长度

static const mp_uchar HOST_ISDST = 1;
static const mp_uchar HOST_ISNOTDST = 0;
static const mp_int32 HOST_TZ_CONVER_UNIT = -60;  // 时区换算单位
static const mp_uchar HOST_TIMEZONE_LENGTH = 6;
static const mp_uchar MAX_ZIP_LOG_NUM = 10;

static const mp_string SCRIPT_INITIATOR_LINUX = "initiator.sh";
static const mp_string SCRIPT_INITIATOR_WIN = "initiator.bat";
static const mp_string SCRIPT_ONLINE_WIN = "online.bat";
static const mp_string SCRIPT_INITIATOR_PARAM_FC = "fcs";
static const mp_string SCRIPT_INITIATOR_PARAM_FC_STATE = "fcs_state";
static const mp_string SCRIPT_INITIATOR_PARAM_ISCSI = "iscsis";
static const mp_string SCRIPT_LINK_INITIATOR = "linkiscsitarget.bat";

typedef struct tag_host_info {
    mp_string sn;           // 主机UUID
    mp_string hostName;     // 主机名称
    mp_string type;         // 环境主类型，写死“agent”
    mp_int32 subType;       // 环境的子类型
    mp_string endPoint;     // 环境的IP地址或域名
    mp_int32 port;          // 端口
    mp_string username;     // 连接环境的用户名
    mp_string password;     // 连接环境的密码
    mp_int32 nOS;           // 操作系统类型
    mp_string strOS;        // 操作系统类型
    mp_string osVersion;    // 操作系统的版本
    mp_int32 osBit;         // 32 or 64bits
    mp_string extendInfo;   // 扩展信息
    mp_string virtualType;  // 虚拟机的虚拟化类型
    mp_string version;      // agent的版本
    tag_host_info() : type("agent"), subType(0), port(59526), nOS(1), osBit(HOST_OS_BIT_64), virtualType("null")
    {
    }
    mp_void TrOS()
    {
        static const mp_int32 osSize = 22;
        if (nOS < osSize && nOS > 0) {
            static mp_string strOsType[osSize] = {
                "Unknown", "windows", "RedHat", "HPUX IA", "SOLARIS", "AIX", "SUSE", "ROCKY", "OEL", "ISOFT", "CentOS",
                "Kylin", "NeoKylin", "openEuler", "UnionTech OS", "Ubuntu", "Debian", "Asianux", "NFSChina", "CEOS",
                "ANOLIS", "KylinSec"};
            strOS = strOsType[nOS];
        }
    }
} host_info_t;

typedef struct TagResourceUsageInfo {
    mp_string sn;            // 主机UUID
    mp_double cpuUsage = 0;
    mp_double memUsage = 0;
    mp_double cpuRateSendAlarmThreshold = 50;
    mp_double memRateSendAlarmThreshold = 50;
    mp_double cpuRateClearAlarmThreshold = 40;
    mp_double memRateClearAlarmThreshold = 40;
} ResourceUageInfoT;

typedef struct SysResourceUsageRest {
    mp_double sysCpuRateRest = 0;
    mp_double sysMemRateRest = 0;
    mp_uint64 sysMemRest = 0;
} SysResourceUsageRestT;

typedef struct tag_agent_info {
    mp_string curVersion;
    mp_string versionTimeStamp;
} agent_info_t;

typedef struct tag_initiator_info {
    std::vector<mp_string> iscsis;
    std::vector<mp_string> fcs;
} initiator_info_t;

typedef struct tag_host_lun_info {
    mp_string arrayVendor;
    mp_string lunId;
    mp_string wwn;
    mp_string arraySn;
    mp_string arrayVersion;
    mp_string arrayModel;
    mp_string deviceName;
    mp_string diskNumber;
} host_lun_info_t;

typedef struct tag_partitisions_info {
    mp_int32 iType;
    mp_uint64 lCapacity;
    mp_string strPartitionName;
    mp_string strDiskNumber;
    mp_string strVolName;
    mp_string strLba;
} partitisions_info_t;

typedef struct tag_timezone_info {
    mp_int32 iIsDST;
    mp_string strTzBias;
} timezone_info_t;

// ## sturct for class CHBA
#ifdef WIN32
typedef struct HBA_wwn {
    unsigned char wwn[g_bytes_8];
} HBA_WWN;

typedef struct HBA_fc4types {
    char bits[g_bytes_32];
} HBA_FC4TYPES;

typedef struct HBA_AdapterAttributes {
    char Manufacturer[g_bytes_64];
    char SerialNumber[g_bytes_64];
    char Model[g_bytes_256];
    char ModelDescription[g_bytes_256];
    HBA_WWN NodeWWN;
    char NodeSymbolicName[g_bytes_256];
    char HardwareVersion[g_bytes_256];
    char DriverVersion[g_bytes_256];
    char OptionROMVersion[g_bytes_256];
    char FirmwareVersion[g_bytes_256];
    unsigned int VendorSpecificID;
    unsigned int NumberOfPorts;
    char DriverName[g_bytes_256];
} HBA_ADAPTERATTRIBUTES;

typedef struct HBA_PortAttributes {
    HBA_WWN NodeWWN;
    HBA_WWN PortWWN;
    unsigned int PortFcId;
    unsigned int PortType;
    unsigned int PortState;
    unsigned int PortSupportedClassofService;
    HBA_FC4TYPES PortSupportedFc4Types;
    HBA_FC4TYPES PortActiveFc4Types;
    char PortSymbolicName[g_bytes_256];
    char OSDeviceName[g_bytes_256];
    unsigned int PortSupportedSpeed;
    unsigned int PortSpeed;
    unsigned int PortMaxFrameSize;
    HBA_WWN FabricName;
    unsigned int NumberofDiscoveredPorts;
} HBA_PORTATTRIBUTES;

typedef struct _MSFC_HBAPortAttributesResults {
    UCHAR NodeWWN[g_bytes_8];
    UCHAR PortWWN[g_bytes_8];
    ULONG PortFcId;
    ULONG PortType;
    ULONG PortState;
    ULONG PortSupportedClassofService;
    UCHAR PortSupportedFc4Types[g_bytes_32];
    UCHAR PortActiveFc4Types[g_bytes_32];
    ULONG PortSupportedSpeed;
    ULONG PortSpeed;
    ULONG PortMaxFrameSize;
    UCHAR FabricName[g_bytes_8];
    ULONG NumberofDiscoveredPorts;
} MSFC_HBAPortAttributesResults;

typedef int (*Hba_GetNumberOfAdapters)();
typedef int (*Hba_GetAdapterName)(int, char*);
typedef HANDLE (*Hba_OpenAdapter)(char*);
typedef void (*Hba_CloseAdapter)(HANDLE);
typedef void (*GetDiscoveredPortAttributes)(unsigned int PortIndex, unsigned int DiscoveredPortIndex, ULONG HBAStatus,
    MSFC_HBAPortAttributesResults PortAttributes);
typedef int (*Hba_GetAdapterAttributes)(HANDLE handle, HBA_ADAPTERATTRIBUTES* hbaattributes);
typedef int (*Hba_GetAdapterPortAttributes)(HANDLE handle, unsigned int portindex, HBA_PORTATTRIBUTES* portattributes);
#endif

enum { LOG_UNKOWN = 0, LOG_INIT, LOG_COLLECTING, LOG_COMPLETED, LOG_FAILED };

class CLogCollector {
public:
#ifdef WIN32
    static DWORD WINAPI LogCollectThread(LPVOID param);
#else
    static mp_void* LogCollectThread(mp_void* param);
#endif
    CLogCollector()
    {
        m_status = LOG_INIT;
        m_strLogName = "";
        (mp_void) memset_s(&m_hThread, sizeof(m_hThread), 0, sizeof(m_hThread));
    }
    ~CLogCollector()
    {
        if (m_hThread.os_id != 0) {
            CMpThread::WaitForEnd(&m_hThread, NULL);
        }
    }
    mp_int32 CollectLog();
    mp_int32 CleanCollectedLogFile(const mp_string &logId);
    mp_uint32 GetStatus()
    {
        return m_status;
    }
    mp_void SetStatus(mp_uint32 uiStatus)
    {
        m_status = uiStatus;
    }

    mp_int32 GetErrCode()
    {
        return m_errCode;
    }
    mp_void SetErrCode(mp_int32 iErrCode)
    {
        m_errCode = iErrCode;
    }

    mp_string GetLogName()
    {
        return m_strLogName;
    }
    mp_void SetLogName(const mp_string& strLogName)
    {
        m_strLogName = strLogName;
    }

    mp_string GetLogId()
    {
        return m_strLogId;
    }
    mp_void SetLogId(const mp_string& strLogId)
    {
        m_strLogId = strLogId;
    }
    mp_string FormatLogName()
    {
        return m_strLogId;
    }
    mp_bool IsCollect()
    {
        return m_isCollect;
    }

    mp_string FormatLogNameFromLogId(const mp_string &logId);

private:
    mp_string GenerateLogName();

private:
    thread_id_t m_hThread;
    mp_uint32 m_status = LOG_UNKOWN;
    mp_int32 m_errCode = -1;
    mp_string m_strLogName;
    mp_string m_strLogId;
    mp_bool m_isCollect = true;
};

// ## class to get wwn
#ifdef WIN32
class CHBA {
public:
    CHBA();
    ~CHBA();
    mp_int32 GetHBAInfo(std::vector<mp_string>&);

private:
    HMODULE m_hbadll;
    Hba_GetNumberOfAdapters Hbafun_GetNumber;
    Hba_GetAdapterName Hbafun_GetAdapterName;
    Hba_OpenAdapter Hbafun_OpenAdapter;
    Hba_CloseAdapter Hbafun_CloseAdapter;
    Hba_GetAdapterAttributes Hbafun_GetAdapterAttributes;
    Hba_GetAdapterPortAttributes Hbafun_GetAdapterPortAttributes;
    static const mp_uchar HBA_NUM_8 = 8;
    static const mp_uchar HBA_NUM_16 = 16;
    static const mp_uchar HBA_NUM_32 = 32;
    static const mp_int32 HBA_NUM_256 = 256;

private:
    mp_bool LoadHBAdllModule();
    mp_bool InitDll();
    mp_void FreeHBAdllModule();
    mp_void HandleHBA(std::vector<mp_string>& pvecResult, mp_int32 hbaCount);
};
#endif
// ##
class CHost {
public:
    CHost();
    ~CHost();

    mp_int32 GetInfo(host_info_t& hostInfo);
    mp_int32 GetAgentResourceUsage(ResourceUageInfoT& ResourceUsageInfo);
    mp_int32 AccumulateResourceUsageLinux(ResourceUageInfoT& ResourceUsageInfo);
    mp_int32 CutString(const mp_string& strToCut, std::vector<mp_string>& strCache);
    mp_int32 CheckStringToDouble(const mp_string& stringToDouble, double& resDouble);
    mp_int32 GetCpuAndMemRate(SysResourceUsageRestT& SysResourceUsageRest);
    mp_int32 GetCpuUsageRate(mp_double& sysCpuRateRest);
    mp_int32 GetMemUsageRate(mp_double& sysMemRateRest, mp_uint64& sysAvailMem);
#ifdef WIN32
    mp_void GetPIDFromSnapshot(const LPCTSTR& szProcessName, const HANDLE& hSnapshot, PROCESSENTRY32& processInfo);
    mp_int32 GetPIDByName(const LPCTSTR& szProcessName);
    mp_int32 GetPid();
    mp_int64 FileTimeToInt64(const FILETIME& time);
    mp_int32 GetCpuNumber();
    mp_int32 GetCpuTime(const mp_uint32 processID, std::vector<mp_int64>& vecTime, std::vector<mp_int64>& vecSysTime);
    mp_int32 GetCpuUsage(std::vector<mp_uint32>& processID, mp_double& res);
    mp_double GetMemUsage(const mp_uint32 ProcessID);
    mp_int32 AccumulateResourceUsageWin(ResourceUageInfoT& ResourceUsageInfo);
    mp_int32 GetCpuUsageRateWin(mp_double& sysCpuRateRest);
    mp_int32 GetMemUsageRateWin(mp_double& sysMemRateRest, mp_uint64& sysAvailMem);
#else
    mp_int32 CalculateCpuAndMem(const std::vector<mp_string> &vecResult, mp_double &resCpu, mp_double &resMem);
#endif
#ifdef LINUX
    mp_int32 GetCpuInfo(std::vector<mp_double>& vecCpuTime);
    mp_int32 GetCpuUsageRateLinux(mp_double& sysCpuRateRest);
    mp_int32 GetMemUsageRateLinux(mp_double& sysMemRateRest, mp_uint64& memAvail);
#endif
    EXTER_ATTACK mp_int32 GetAgentInfo(agent_info_t& agentInfo);
    void CheckRange(mp_double &numOfRate);
    mp_int32 GetTaskStatus(mp_string& taskType, mp_string& strTaskStatus);
    mp_int32 CheckAgentInstallMode();
    mp_int32 GetUpgradeErrorDetails(Json::Value& jValue);
    mp_int32 UpdateUpgradeErrorDetails(const Json::Value& jValue);
    mp_int32 GetNetworkInfo(std::vector<mp_string>& vecMacs);
    mp_int32 QueryFusionStorageIP(std::vector<mp_string>& strFusionStorageIP);
    mp_int32 GetDiskInfo(std::vector<host_lun_info_t>& vecLunInfo);
    mp_int32 GetTimeZone(timezone_info_t& sttimezone);
    mp_int32 GetAgentVersion(mp_string& strAgentVersion, mp_string& strBuildNum);
    mp_int32 GetInitiatorsByProtocol(initiator_info_t& initInfo, const mp_string protocolType);
    mp_int32 GetInitiators(initiator_info_t& initInfo);
    mp_int32 GetPartisions();
    mp_int32 LinkiScsiTarget(const Json::Value& scsiTargets);
    mp_int32 QueryWwpns(std::vector<mp_string>& vecWwpns);
    mp_int32 GetHostInfo(Json::Value& jValue);
    mp_int32 QueryWwpnsV2(std::map<mp_string, mp_string>& mapWwpns);
    mp_int32 QueryIqns(std::map<mp_string, mp_string>& mapIqns);
    mp_int32 DataturboRescan();
    mp_int32 ScanDisk();
    mp_int32 UpdateTrapServer(const std::vector<trap_server>& vecTrapServer, const snmp_v3_param& stParam);
    mp_int32 RegTrapServer(trap_server& stTrapServer);
    mp_int32 UnRegTrapServer(trap_server& stTrapServer);
    EXTER_ATTACK mp_int32 VerifySnmp(snmp_v3_param& stParam);
    mp_int32 QueryThirdPartyScripts(std::vector<mp_string>& vecFileInfo);
    mp_int32 IsSafeDirectory(const mp_string& strInput);
    mp_int32 ExecThirdPartyScript(const mp_string& fileName,
        const mp_string& paraValues, std::vector<mp_string>& vecResult);
    mp_int32 CollectLog();
    mp_int32 CleanLogPackage(const mp_string &logId)
    {
        return m_logCollector.CleanCollectedLogFile(logId);
    }
    mp_int32 GetHostHypervisorType(mp_string& hypervisorType);
    mp_int32 GetHostSN(mp_string& strSN);
    mp_int32 SetHostSN(const mp_string& strHostSnFile, std::vector<mp_string>& vecMacs);
    mp_int32 SetLogLevel(const mp_int32 level);
    mp_string GetLogName()
    {
        return m_logCollector.GetLogName();
    }
    mp_string GetLogId()
    {
        return m_logCollector.GetLogId();
    }
    mp_uint32 GetCollectLogStatus()
    {
        return m_logCollector.GetStatus();
    }
    mp_int32 GetCollectLogErrCode()
    {
        return m_logCollector.GetErrCode();
    }
    mp_string FormatLogNameFromId(const mp_string &logId)
    {
        return m_logCollector.FormatLogNameFromLogId(logId);
    }
    mp_int32 GetHostExtendInfo(Json::Value& jValue, mp_int32 m_proxyRole);
    mp_int32 GetHostAgentIplist(Json::Value& jIpList);
    mp_int32 UpdateManagerServer(const std::vector<mp_string>& vecManagerServer);

#ifdef WIN32
    mp_int32 DeviceOnline(mp_string& strDiskNum);
    mp_int32 GetPartisions(std::vector<partitisions_info_t>& partisioninfos);
#endif
    mp_int32 GetHostIPList(const std::vector<mp_string>& reqIpv4List, const std::vector<mp_string>& reqIpv6List,
        std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List);
    mp_int32 GetContainerIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List);
    mp_int32 GetEipInfo(mp_string& eip);
    mp_int32 GetFloatingIp(mp_string& floatingIp);
    mp_int32 GetAvailableZone(mp_string& az);
    mp_int32 GetIsSharedAgent(mp_string& isShared);
    mp_int32 GetInstallMode(mp_string& mode);

private:
    mp_string m_userName = "rdadmin";
    mp_string m_hostsnFile;
    thread_lock_t m_pMutex;
    CLogCollector m_logCollector;
#ifdef WIN32
    CHBA m_CHBA;
    std::vector<mp_uint32> m_vecProcessID;
#endif
    static const mp_uchar HOST_NUM_1 = 1;
    static const mp_uchar HOST_NUM_2 = 2;
    static const mp_uchar HOST_NUM_8 = 8;
    static const mp_uchar HOST_NUM_16 = 16;
    static const mp_uchar HOST_NUM_32 = 32;
    static const mp_int32 HOST_NUM_256 = 256;

private:
    mp_int32 GetSingleDiskInfo(const mp_string& diskName, host_lun_info_t& structLunInfo);
    mp_int32 GetHostOSBit(mp_int32& iOSBit);
    mp_int32 GetHostOS(mp_int32& iOSType, mp_string& strOSVersion);
    mp_int32 GetHostOSExecCmd(mp_int32& iOSType, mp_string& strOSVersion, const mp_string& strCmd);
    mp_int32 ReadHostSNInfo(std::vector<mp_string>& vecMacs);
    mp_int32 CopyHostSN(const mp_string& strSrcHostSnFile, const mp_string& strDestHostSnFile);
    mp_int32 CheckTrapServer(const trap_server& stTrapServer);
    mp_int32 Prepare4HP();
    mp_int32 AfterHandle4HP(mp_string& strDevName, std::vector<mp_string>::iterator& iter,
        host_lun_info_t& structLunInfo, std::vector<host_lun_info_t>& vecLunInfo);
    mp_int32 GetVendorAndProduct(mp_string& strDevName, mp_string& strVendor, mp_string& strProduct);
#ifdef WIN32
    mp_int32 GetWindowsOs(mp_int32& iOSType, mp_string& strOSVersion);
#endif

    mp_string BuildLinkTargetParam(
        const mp_string& targetIp, mp_uint32 targetPort, const mp_string& chapUser);
};

#endif  // __AGENT_HOST_H__
