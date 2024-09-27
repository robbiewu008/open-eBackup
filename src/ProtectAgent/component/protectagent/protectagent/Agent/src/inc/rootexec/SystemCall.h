/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file SystemCall.h
 * @brief  Contains function declarations for SystemCall
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _SYSTEM_CALL_H_
#define _SYSTEM_CALL_H_
#include "common/Defines.h"
#include "common/Types.h"
#include <map>
#include <vector>
#include <list>
#include <functional>

// redhat 5.2、redhat 5.5
static const mp_string RAW_CTRL_FILE = "/dev/rawctl";
// suse10、suse11、redhat6.3
static const mp_string RAW_CTRL_FILE_NEW = "/dev/raw/rawctl";

// file system 2.6.29up
static const mp_uchar OPER_TIME_OUT = 30;
static const mp_uchar THAW_ERR = 22;

class CSystemCall {
public:
    static mp_int32 ExecSysCmd(const mp_string& strUniqueID, mp_int32 iCommandID,
        const std::vector<mp_string>& vecParam);
    static mp_int32 ExecScript(const mp_string& strUniqueID, mp_int32 iCommandID,
        const std::vector<mp_string>& vecParam);
    static mp_int32 ExecEbkUserScript(mp_string& strScriptFileName, const mp_string& strUniqueID);
    static mp_int32 ExecAppScript(const mp_string& strUniqueID, mp_int32 iCommandID);
    static mp_int32 ExecAddFirewall();
    static mp_int32 GetDisk80Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 GetDisk83Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 GetDisk00Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 GetDiskC8Page(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 GetDiskCapacity(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 GetVendorAndProduct(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 ExecThirdPartyScript(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 ExecScriptByScriptUser(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 GetRawMajorMinor(const mp_string& strUniqueID);
    static mp_int32 BatchGetLUNInfo(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 ReloadUDEVRules(const mp_string& strUniqueID);
    static mp_int32 SyncDataFile(const mp_string& strUniqueID);
    static mp_int32 GetHostLunID(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 CheckDirExist(mp_string& strUniqueID);
    static mp_int32 SignScripts();
    static mp_int32 GetParamFromTmpFile(const mp_string& strUniqueID, mp_string& strParam);
    static mp_int32 StartDataProcess(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 ExecAddHostNameToFile(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 ExecUserDefineScript(const mp_string& userDefineCmd);

    static mp_bool WhiteListVerify(mp_int32 iCommandID, const mp_string& strParam);
    static mp_bool KillWhiteListVerify(const mp_string& strParam);
    static mp_bool CatWhiteListVerify(const mp_string& strParam);
    static mp_bool MountWhiteListVerify(const mp_string& strParam);
    static mp_bool VerifyProcess(const mp_string& processInfo);
    static mp_bool RmWhiteListVerify(const mp_string &filePath);
    static mp_int32 ChecVddkVersion(const mp_string& version);
    static mp_bool GetParentPid(mp_string& ppid);
    static mp_bool GetParentPidImpl(const mp_string& pid, mp_string& ppid);
    static mp_bool GetProcessInfo(const mp_string& pid, mp_string& result);
    static mp_int32 CheckParentPath(const mp_string& path);
    static mp_int32 WriteCNToHosts(const mp_string& strUniqueID, const std::vector<mp_string>& vecParam);
    static mp_int32 ScanDirAndFileForInstantlyMount(const mp_string &strUniqueID,
        const std::vector<mp_string> &vecParam);
    static mp_int32 WriteScanResultForInstantlyMount(const mp_string &strUniqueID,
        const std::vector<mp_string> &vecParam);
#ifdef LIN_FRE_SUPP
    static mp_int32 ThawFileSys(const mp_string& strUniqueID);
    static mp_int32 FreezeFileSys(const mp_string& strUniqueID);
#endif

private:
    static const mp_int32 systemCallNum256 = 256;
    static mp_int32 GetLUNInfo(mp_string& strDevice, mp_string& strLUNInfo);
    static mp_int32 GetLUNIdWhenC8(mp_string& strDevice, mp_string& strLUNID);
    static mp_int32 GetAPPScripts(
        mp_int32 iCommandID, const mp_string& appScriptFolder, std::vector<mp_string>& vecScripts);
    static mp_int32 CheckUserDefineScriptLegality(const mp_string& scriptCmd);
    static bool CheckIfCNExist(const mp_string &hostName);
    static mp_int32 AddHostNameToFile(const mp_string& hostName);
    static mp_int32 ScanDirAndFile(mp_string &rootPath, std::vector<mp_string> &rootfolderpath,
        std::vector<mp_string> &rootfilepath);
    static mp_int32 GetFolderPath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath);
    static mp_int32 GetFilePath(mp_string &strFolder, std::vector<mp_string> &vecFolderPath);
};

class CCommandMap {
public:
    CCommandMap();
    ~CCommandMap()
    {}
    mp_string GetCommandString(mp_int32 iCommandID);
    mp_bool NeedEcho(mp_int32 iCommandID);
    mp_bool NoParam(mp_int32 iCommandID);
    mp_bool WhiteListVerify(int iCommandID, const std::vector<mp_string>& vecParam);
    mp_void InitDB2ScriptMap();
    mp_void InitOracleScriptMap();
    mp_void InitOracleBkScriptMap();
    mp_void InitHostScriptMap();
    mp_void InitSysCmdMap1();
    mp_void InitSysCmdMap2();
    mp_void InitSysCmdMap4();
    mp_void InitSysCmdMap5();
    mp_void InitNeedEchoCmdMap1();
    mp_void InitNeedEchoCmdMap2();
    mp_void InitUpgradeCmdMap();
    mp_void InitModifyCmdMap();
    mp_void InitNoParamMap();
    mp_void InitSanclientMap();
    mp_void InitDpcCmdMap();

private:
    mp_void InitWhiteFunMap();
    mp_bool UpgradeCallVerify(const std::vector<mp_string>& vecParam);

private:
    std::map<mp_int32, mp_string> m_mapCommand;
    std::map<mp_int32, mp_bool> m_mapNeedEcho;
    std::map<mp_int32, mp_bool> m_mapNoParam;
    std::list<mp_string> m_listKillWhite;
    using WhiteVerifyFun = std::function<bool(const std::vector<mp_string>&)>;
    std::map<int, WhiteVerifyFun> m_mapWhiteFun;
};

#endif
