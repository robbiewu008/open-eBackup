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
#include "servicecenter/services/device/PrepareFileSystem.h"

#include "common/Log.h"
#include "common/Defines.h"
#include "common/ConfigXmlParse.h"
#include "message/tcp/CSocket.h"
#include "securecom/RootCaller.h"
#include "host/host.h"
#include "common/Utils.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"
#include "securecom/CryptAlg.h"
#include "pluginfx/ExternalPluginManager.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"

#ifdef WIN32
#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <Winnetwk.h>
#endif

using namespace std;
using namespace AppProtect;

namespace {
    const mp_int32 CHECK_HOST_RETRY_TIMES = 3;
    const mp_int32 DEFAULT_EXIT_CODE = 1;
    const mp_int32 LINK_HASH_DIR_INDEX = 3;
    const mp_int32 FILECLIENT_MOUNT_POINT_5 = 5;
    const mp_string IN_AGENT_TYPE = "1";
    const mp_string OUT_AGENT_TYPE = "0";
    const mp_string DORADO_ENVIRONMENT = "0";
    const mp_string VIRTUAL_DORADO_ENVIRONMENT = "1";
    const mp_string AIX_SYS_NAME = "aix";
    const mp_string SOLARIS_SYS_NAME = "solaris";
    const int MAX_REMOTE_NAME_LENTH = 1024;
    const mp_float WIN_SERVER_VERSION_2012 = 6.2;
    const mp_int32 CALL_API_RETRY_TIMES = 3;
    const mp_int32 CALL_API_RETRY_INTERVAL = 1000;   // ms
    const mp_string DRIVE_UNAVAIL = "UNAVAIL";
    const mp_string MOUNT_MODE_READ_ONLY = "ro";
    const mp_string MOUNT_MODE_READ_WRITE = "rw";
    const uint64_t ONE_HOUR = 3600;               // 1h = 3600s
    const mp_string MOUNT_PROTOCOL_NFS = "nfs";
    const mp_string MOUNT_PROTOCOL_CIFS = "cifs";
    const mp_string MOUNT_PROTOCOL_DATATURBO = "dataturbo";
    const mp_string MOUNT_PROTOCOL_FILECLIENT = "FileClient";
    static std::map<mp_string, mp_string> G_ProtocolTypePortMap = {
        {MOUNT_PROTOCOL_NFS, "111"},
        {MOUNT_PROTOCOL_CIFS, "445"},
        {MOUNT_PROTOCOL_DATATURBO, "12300"}
    };
}

#ifdef WIN32
mp_int32 PrepareFileSystem::MountNasFileSystem(
    MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath)
{
    mp_string strSubPath = GenerateSubPath(mountNasParam);
    // 密码特殊字符 !#$%&()*+-.:<=>?@[]^_{|}~
    mp_string outStr;
    EncryptStr(mountNasParam.cifsAuthPwd, outStr);
    if (outStr.empty()) {
        ERRLOG("Encrypt pwd failed, mount failed.");
        return MP_FAILED;
    }
    _putenv_s("AGENT_ENV_AuthPassword", outStr.c_str());
    if (mountNasParam.protocolType.empty()) {
        mountNasParam.protocolType = MOUNT_PROTOCOL_CIFS;
    }

    MountNasKeyParam mountKeyParam(mountNasParam);
    mountKeyParam.strSubPath = strSubPath;
    AdjustMountIpList(mountNasParam.vecStorageIp);
    for (mp_string strIp : mountNasParam.vecStorageIp) {
        mountKeyParam.storageIp = strIp;
        mountNasParam.ip = strIp;
        mountKeyParam.isInerSnapshot =
            (mountNasParam.nCopyFormat == CopyFormatType::type::INNER_SNAPSHOT) ? true : false;
        mp_string mountPath;
        mp_int32 iRet = WinMountOperation(mountKeyParam, mountPath);
        if (iRet == MP_SUCCESS) {
            HandleAfterMountSuccess(mountNasParam);
            successMountPath.push_back(mountPath);
            INFOLOG("Use ip %s mount success.", strIp.c_str());
            break;
        }
        HandleAfterMountFail(mountNasParam, iRet);
        WARNLOG("Ip %s can not access.", strIp.c_str());
    }
    if (successMountPath.empty()) {
        ERRLOG("File system %s have no valid ip.", mountNasParam.storageName.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
#else
EXTER_ATTACK mp_int32 PrepareFileSystem::MountNasFileSystem(
    MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath)
{
    mp_string pluginName;
    if (GetMountParam(mountNasParam, mountNasParam.mountOption, mountNasParam.mountProtocol, pluginName) !=
        MP_SUCCESS) {
        ERRLOG("Failed to get mount param.");
        return MP_FAILED;
    }
    CHECK_FAIL_EX(CheckParamValid(mountNasParam.mountOption));
    CHECK_FAIL_EX(CheckParamValid(mountNasParam.mountProtocol));
    mp_string subPath = "";
    for (auto &path : mountNasParam.permit.vecPath) {
        subPath = subPath + ":" + path;
    }
    mountNasParam.subPath = subPath.substr(subPath.find_first_of(":") + 1);
    if (mountNasParam.protocolType.empty()) {
        mountNasParam.protocolType = MOUNT_PROTOCOL_NFS;
    }
    mountNasParam.pluginName = pluginName;
    CRootCaller rootCaller;

    mp_bool allIPInvalid = MP_TRUE;  // one storage fs, which must have a valid ip
    mp_int32 mountErrCode = 0;
    AdjustMountIpList(mountNasParam.vecStorageIp);
    for (auto iterIp : mountNasParam.vecStorageIp) {
        CHECK_FAIL_EX(CheckParamValid(iterIp));
        mountNasParam.ip = iterIp;
        GetMountPath(mountNasParam, iterIp, mountNasParam.mountPath);
        CHECK_FAIL_EX(CheckParamValid(mountNasParam.mountPath));

        mp_string scriptParamStr = GetNasMountScriptParama(mountNasParam);
        mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_NAS_FILESYS, scriptParamStr, NULL);
        if (ret == MP_SUCCESS) {
            HandleAfterMountSuccess(mountNasParam);
            successMountPath.push_back(mountNasParam.mountPath);
            allIPInvalid = MP_FALSE;
        } else {
            mountErrCode = ret;
            HandleAfterMountFail(mountNasParam, mountErrCode);
            WARNLOG("Mount %s NFS filesystem failed ip is %s", mountNasParam.storagePath.c_str(), iterIp.c_str());
        }
    }

    if (allIPInvalid == MP_TRUE) {  // if one storage fs have no valid ip, return failed
        ERRLOG("File system %s have no valid ip.", mountNasParam.storagePath.c_str());
        return (mountErrCode > DEFAULT_EXIT_CODE) ? mountErrCode : MP_FAILED;
    }
    return successMountPath.empty() ? MP_FAILED : MP_SUCCESS;
}
#endif // WIN32

#ifndef WIN32
EXTER_ATTACK mp_int32 PrepareFileSystem::MountFileIoSystem(
    MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, const mp_string &recordFile)
{
    mp_string subPath = "";
    for (const auto &path : mountNasParam.permit.vecPath) {
        subPath = subPath + ":" + path;
    }
    subPath = subPath.substr(subPath.find_first_of(":") + 1);

    CRootCaller rootCaller;
    // one storage fs, which must have a valid ip
    mp_bool allIPInvalid = MP_TRUE;
    mp_int32 mountErrCode = 0;
    AdjustMountIpList(mountNasParam.vecStorageIp);
    for (auto iterIp : mountNasParam.vecStorageIp) {
        CHECK_FAIL_EX(CheckParamValid(iterIp));
        mp_string mountPath;
        GetMountPath(mountNasParam, iterIp, mountPath);
        CHECK_FAIL_EX(CheckParamValid(mountPath));
        mountNasParam.ip = iterIp;
        mountNasParam.mountPath = mountPath;
        ostringstream scriptParam;
        scriptParam << "mountPath=" << mountPath << NODE_COLON
            << "repositoryType=" << mountNasParam.repositoryType << NODE_COLON
            << "protocolType=" << mountNasParam.protocolType << NODE_COLON
            << "subPath=" << subPath << NODE_COLON
            << "LunInfo=" << mountNasParam.lunInfo << NODE_COLON
            << "recordFile=" << recordFile << NODE_COLON
            << "uid=" << mountNasParam.permit.uid << NODE_COLON
            << "gid=" << mountNasParam.permit.gid << NODE_COLON
            << "mode=" << mountNasParam.permit.mode << NODE_COLON;
        INFOLOG("Mount param is %s.", scriptParam.str().c_str());
        mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_SANCLIENT_FILESYS, scriptParam.str(), NULL);
        if (ret == MP_SUCCESS) {
            successMountPath.push_back(mountPath);
            allIPInvalid = MP_FALSE;
        } else {
            mountErrCode = ret;
            WARNLOG("Mount %s SanClient filesystem failed protocol type is %s, lun_info is %s",
                mountNasParam.storagePath.c_str(), mountNasParam.protocolType.c_str(), mountNasParam.lunInfo.c_str());
        }
    }

    // if one storage fs have no valid ip, return failed
    if (allIPInvalid == MP_TRUE) {
        ERRLOG("File system %s have no valid ip.", mountNasParam.storagePath.c_str());
        return (mountErrCode > DEFAULT_EXIT_CODE) ? mountErrCode : MP_FAILED;
    }
    return MP_SUCCESS;
}
#endif

#ifndef WIN32
EXTER_ATTACK mp_int32 PrepareFileSystem::MountFileClientSystem(
    MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, std::vector<mp_string> &realMountPath)
{
    mp_string pluginName;
 
    mp_int32 iRet = ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(
        mountNasParam.appType, pluginName);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Fail to get plugin name for apptype=%s.", mountNasParam.appType.c_str());
    }

    mp_string subPath = "";
    for (auto &path : mountNasParam.permit.vecPath) {
        subPath = subPath + ":" + path;
    }
    mountNasParam.subPath = subPath.substr(subPath.find_first_of(":") + 1);
    mountNasParam.protocolType = MOUNT_PROTOCOL_FILECLIENT;
    mountNasParam.pluginName = pluginName;
    CRootCaller rootCaller;

    mp_string jobId = mountNasParam.jobID;
    mp_int32 OSADAuthPort = mountNasParam.OSADAuthPort;
    mp_int32 OSADServerPort = mountNasParam.OSADServerPort;
    if (mountNasParam.repositoryType == "index" && !mountNasParam.pvcTaskId.empty()) {
        jobId = mountNasParam.pvcTaskId;
        OSADAuthPort = mountNasParam.pvcOSADAuthPort;
        OSADServerPort = mountNasParam.pvcOSADServerPort;
    }

    mp_string mountPath = MOUNT_PUBLIC_PATH + mountNasParam.appType + PATH_SEPARATOR + jobId;
    CHECK_FAIL_EX(CheckParamValid(mountPath));
    mp_string mountSuccPath = MOUNT_PUBLIC_PATH + mountNasParam.appType + PATH_SEPARATOR + jobId +
                              mountNasParam.storagePath;
    CHECK_FAIL_EX(CheckParamValid(mountSuccPath));

    mp_string StorageIpList = CMpString::StrJoin(mountNasParam.vecStorageIp, ",");
    mp_int32 mountErrCode = 0;
    ostringstream scriptParam;
    scriptParam << "storageName=FileClient" << NODE_COLON << "nasFileSystemName=" << mountNasParam.storagePath
                << NODE_COLON << "mountPath=" << mountPath << NODE_COLON << "repositoryType="
                << mountNasParam.repositoryType << NODE_COLON << "runAccount=" << mountNasParam.runAccount
                << NODE_COLON << "subPath=" << subPath << NODE_COLON << "uid=" << mountNasParam.permit.uid
                << NODE_COLON << "gid=" << mountNasParam.permit.gid << NODE_COLON << "mode="
                << mountNasParam.permit.mode << NODE_COLON << "ipList=" << StorageIpList << NODE_COLON
                << "jobID=" << jobId << NODE_COLON << "authPort=" << OSADAuthPort << NODE_COLON << "serverPort="
                << OSADServerPort << NODE_COLON << "isLinkEncryption=" << mountNasParam.isLinkEncryption;
 
    mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_DATATURBO_FILESYS, scriptParam.str(), nullptr);
    if (ret == MP_SUCCESS) {
        successMountPath.push_back(mountSuccPath);
        realMountPath.push_back(mountPath);
    } else {
        mountErrCode = ret;
        WARNLOG(
            "Mount %s FileClient filesystem failed ip is %s", mountNasParam.storagePath.c_str(), StorageIpList.c_str());
    }
    return ret;
}
#endif

EXTER_ATTACK mp_int32 PrepareFileSystem::CheckAndCreateDataturboLink(const mp_string &storageName,
    const MountNasParam &mountNasParam)
{
    LOGGUARD("");
    if (StaticConfig::IsInnerAgent()) {
        ERRLOG("Inner agent don't support dataturbo.");
        return ERR_NOT_SUPPORT_DATA_TURBO;
    }
    if (!mountNasParam.isFcOn && mountNasParam.vecDataturboIP.size() == 0) {
        ERRLOG("File system %s have no valid ip.", storageName.c_str());
        return ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT;
    }

    ostringstream scriptParam;
    if (mountNasParam.isFcOn) {
        scriptParam << "storageName=" << storageName << NODE_COLON << "linkType=FC" << NODE_COLON
                    << "userName=" << mountNasParam.authKey << NODE_COLON << "password=" << mountNasParam.authPwd
                    << NODE_COLON << "dedupSwitch=" << (mountNasParam.isDeduptionOn ? "ON" : "OFF");
    } else {
        std::vector<mp_string> connectableIP;
        std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
        if (httpClient == nullptr) {
            COMMLOG(OS_LOG_ERROR, "HttpClient create failed.");
            return ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT;
        }
        for (const mp_string &ip : mountNasParam.vecDataturboIP) {
            if (httpClient->TestConnectivity(ip, G_ProtocolTypePortMap[MOUNT_PROTOCOL_DATATURBO])) {
                COMMLOG(OS_LOG_INFO, "Can connect ip(%s).", ip.c_str());
                connectableIP.push_back(ip);
            }
        }
        mp_string ipList = CMpString::StrJoin(connectableIP, ",");
        scriptParam << "storageName=" << storageName << NODE_COLON << "ipList=" << ipList << NODE_COLON
                    << "userName=" << mountNasParam.authKey << NODE_COLON << "password=" << mountNasParam.authPwd
                    << NODE_COLON << "dedupSwitch=ON";
    }
    mp_int32 ret = MP_FAILED;
#ifndef WIN32
    CRootCaller rootCaller;
    ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_CHECK_AND_CREATE_DATATURBO_LINK, scriptParam.str(), NULL);
#else
    ret = SecureCom::SysExecScript(SCRIPT_CHECK_AND_CREATE_DATATURBO_LINK, scriptParam.str(), NULL);
#endif
    if (ret != MP_SUCCESS) {
        ERRLOG("Check and Create Dataturbo link Failed!");
        return ERR_CREATE_DATA_TURBO_LINK;
    }
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 PrepareFileSystem::MountDataturboFileSystem(
    MountNasParam &mountNasParam, std::vector<mp_string> &successMountPath, std::vector<mp_string> &dtbMountPath)
{
    LOGGUARD("");
    CHost host;
    mp_string storageName;
    mp_int32 iRet = host.GetHostSN(storageName); // 使用HostSN做Dataturbo对象的storage_name
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return MP_FAILED;
    }

    iRet = CheckAndCreateDataturboLink(storageName, mountNasParam);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create Dataturbo link Failed!");
        return iRet;
    }
    mountNasParam.protocolType =
        mountNasParam.protocolType.empty() ? MOUNT_PROTOCOL_DATATURBO : mountNasParam.protocolType;

    mp_string subPath = "";
    for (auto &path : mountNasParam.permit.vecPath) {
        subPath = subPath + ":" + path;
    }
    subPath = subPath.substr(subPath.find_first_of(":") + 1);

    mp_string mountPath = MOUNT_PUBLIC_PATH + mountNasParam.appType + PATH_SEPARATOR + mountNasParam.jobID +
                          PATH_SEPARATOR + mountNasParam.repositoryType + mountNasParam.storagePath;
    if (!mountNasParam.isFcOn) {
        // 保证原有逻辑不变
        mountPath += (PATH_SEPARATOR + "Dataturbo");
    }
    CHECK_FAIL_EX(CheckParamValid(mountPath));
    ostringstream scriptParam;
    scriptParam << "storageName=" << storageName << NODE_COLON << "nasFileSystemName=" << mountNasParam.storagePath
                << NODE_COLON << "mountPath=" << mountPath << NODE_COLON
                << "repositoryType=" << mountNasParam.repositoryType << NODE_COLON
                << "runAccount=" << mountNasParam.runAccount << NODE_COLON << "subPath=" << subPath << NODE_COLON
                << "uid=" << mountNasParam.permit.uid << NODE_COLON << "gid=" << mountNasParam.permit.gid << NODE_COLON
                << "mode=" << mountNasParam.permit.mode;
    mp_int32 ret = MP_FAILED;
#ifndef WIN32
    CRootCaller rootCaller;
    ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_MOUNT_DATATURBO_FILESYS, scriptParam.str(), NULL);
#else
    mountPath = CMpString::StrReplace(mountPath, "/", "\\");
    ret = SecureCom::SysExecScript(SCRIPT_DATATURBO_MOUNT, scriptParam.str(), NULL);
#endif
    if (ret != MP_SUCCESS) {
        WARNLOG("Mount %s Dataturbo filesystem failed!", mountNasParam.storageName.c_str());
        return ERR_MOUNT_DATA_TURBO_FILE_SYSTEM;
    }
    successMountPath.push_back(mountPath);
    dtbMountPath.push_back(mountPath + "@" + mountNasParam.storagePath);

    return MP_SUCCESS;
}

mp_string PrepareFileSystem::GetNasMountScriptParama(const MountNasParam &mountNasParam)
{
    ostringstream scriptParam;
    scriptParam << "storageIp=" << mountNasParam.ip << NODE_COLON
        << "nasFileSystemName=" << mountNasParam.storagePath << NODE_COLON
        << "mountPath=" << mountNasParam.mountPath << NODE_COLON
        << "mountOption:" << mountNasParam.mountOption << NODE_COLON
        << "repositoryType=" << mountNasParam.repositoryType << NODE_COLON
        << "mountProtocol=" << mountNasParam.mountProtocol << NODE_COLON
        << "runAccount=" << mountNasParam.runAccount << NODE_COLON
        << "subPath=" << mountNasParam.subPath << NODE_COLON
        << "uid=" << mountNasParam.permit.uid << NODE_COLON << "gid=" << mountNasParam.permit.gid << NODE_COLON
        << "mode=" << mountNasParam.permit.mode << NODE_COLON
        << "pluginName=" << mountNasParam.pluginName << NODE_COLON;
    DBGLOG("Mount param is %s.", scriptParam.str().c_str());
    return scriptParam.str();
}

mp_void PrepareFileSystem::GetMountPath(const MountNasParam &mountNasParam, const mp_string iterIp,
    mp_string &mountPath)
{
    INFOLOG("GetMountPath, appType=%s, iterIp=%s.",
            mountNasParam.appType.c_str(), iterIp.c_str());
    if (mountNasParam.appType == "Fileset") {
#if defined(AIX) || defined(SOLARIS)
        // Solaris & AIX文件集备份，防止挂载路径超长导致备份失败。
        mp_string originMountPath;
        mp_string hashMountPath;
        std::hash<std::string> strHash;
        originMountPath = mountNasParam.appType + PATH_SEPARATOR + mountNasParam.jobID +
            PATH_SEPARATOR + mountNasParam.repositoryType + mountNasParam.storagePath + PATH_SEPARATOR + iterIp;
        hashMountPath = std::to_string(strHash(originMountPath));
        mountPath = MOUNT_PUBLIC_PATH_PREFIX + hashMountPath;
        INFOLOG("GetMountPath, originMountPath=%s, mountPath=%s.",
            originMountPath.c_str(), mountPath.c_str());
        return;
#endif
    }
    mountPath = MOUNT_PUBLIC_PATH + mountNasParam.appType + PATH_SEPARATOR + mountNasParam.jobID +
        PATH_SEPARATOR + mountNasParam.repositoryType + mountNasParam.storagePath + PATH_SEPARATOR + iterIp;
    if (!mountNasParam.isFullPath) {
        mountPath = MOUNT_PUBLIC_PATH + mountNasParam.appType + PATH_SEPARATOR +
            mountNasParam.jobID + PATH_SEPARATOR + mountNasParam.repositoryType + PATH_SEPARATOR + iterIp;
    }
}

mp_int32 PrepareFileSystem::GetPluginDefinedMountOption(const MountNasParam &mountNasParam, mp_string &mountOption)
{
    mp_string mountOptionKey =
        mountNasParam.isLinkEncryption ? mountNasParam.appType + "_LinkEncry" : mountNasParam.appType;
#ifdef AIX
    mountOptionKey = mountOptionKey + "_" + mountNasParam.repositoryType + "_" + AIX_SYS_NAME;
#else
    mountOptionKey = mountOptionKey + "_" + mountNasParam.repositoryType;
#endif
    // 获取插件配置文件中的挂载参数
    mountOption = ExternalPluginManager::GetInstance().GetParseManager()->GetMountParamByAppType(
        mountNasParam.appType, mountOptionKey);
    if (!mountOption.empty()) {
        INFOLOG("Get plugins mount param success, appType=%s, repoType=%s, mountParam=%s.",
            mountNasParam.appType.c_str(), mountNasParam.repositoryType.c_str(), mountOption.c_str());
        return MP_SUCCESS;
    }
    mountOptionKey =
        mountNasParam.isLinkEncryption ? mountNasParam.appType + "_LinkEncry" : mountNasParam.appType;
#ifdef AIX
    mountOptionKey = mountOptionKey + "_" + AIX_SYS_NAME;
#elif defined(SOLARIS)
    mountOptionKey = mountOptionKey + "_" + SOLARIS_SYS_NAME;
#endif
    mountOption = ExternalPluginManager::GetInstance().GetParseManager()->GetMountParamByAppType(
        mountNasParam.appType, mountOptionKey);
    if (!mountOption.empty()) {
        INFOLOG("Get plugins mount param success, appType=%s, mountParam=%s.",
            mountNasParam.appType.c_str(), mountOption.c_str());
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 PrepareFileSystem::GetMountParam(const MountNasParam &mountNasParam, mp_string &mountOption,
    mp_string &mountProtocol, mp_string &pluginName)
{
    mp_int32 ret = CConfigXmlParser::GetInstance().GetValueString(CFG_MOUNT_SECTION, CFG_GENERAL_MOUNT_PROTOCOL,
        mountProtocol);
    if (ret != MP_SUCCESS) {
        ERRLOG("Failed to get mount protocol.");
        return MP_FAILED;
    }
    mp_int32 iRet = ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(
        mountNasParam.appType, pluginName);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Fail to get plugin name for apptype=%s.", mountNasParam.appType.c_str());
    }
    if (GetPluginDefinedMountOption(mountNasParam, mountOption) == MP_SUCCESS) {
        return MP_SUCCESS;
    }
#ifdef AIX
    mp_string cfgMountOption = CFG_AIX_GENERAL_MOUNT_OPTION;
#elif defined(SOLARIS)
    mp_string cfgMountOption = CFG_SOLARIS_GENERAL_MOUNT_OPTION;
#else
    mp_string cfgMountOption =
        (mountNasParam.isLinkEncryption == true) ? CFG_LINKENCRY_MOUNT_OPTION : CFG_GENERAL_MOUNT_OPTION;
#endif
    ret = CConfigXmlParser::GetInstance().GetValueString(CFG_MOUNT_SECTION, cfgMountOption, mountOption);
    if (ret != MP_SUCCESS) {
        ERRLOG("No mount parameters available, apptype=%s.", mountNasParam.appType.c_str());
        return MP_FAILED;
    }
    // Non clone fs use ro mount
    if (!mountNasParam.isCloneFsMount) {
        mp_size pos = mountOption.find(MOUNT_MODE_READ_WRITE);
        if (pos != std::string::npos) {
            mountOption.replace(pos, MOUNT_MODE_READ_WRITE.size(), MOUNT_MODE_READ_ONLY);
        }
    }
    INFOLOG("Get agent mount param success, appType=%s, option=%s.",
        mountNasParam.appType.c_str(), mountOption.c_str());
    return MP_SUCCESS;
}

mp_int32 PrepareFileSystem::UmountNasFileSystem(
    const vector<mp_string> &successMountPath, const mp_string &jobId, const bool &isFileClientMount)
{
#ifdef WIN32
    return WinUmountOperation(successMountPath, jobId);
#else
    std::set<mp_string> setMountPath;
    for (mp_string strPath : successMountPath) {
        if (strPath.empty()) {
            continue;
        }
        EraseMountInfoFromMap(strPath);
        if (isFileClientMount) {
            mp_string realPath = SplitFileClientMountPath(strPath);
            if (realPath.empty()) {
                continue;
            }
            setMountPath.insert(realPath);
        } else {
            setMountPath.insert(strPath);
        }
    }
    vector<mp_string> vecErrMountPath;
    for (mp_string strPath : setMountPath) {
        ostringstream scriptParam;
        scriptParam << "umountPath=" << strPath << NODE_COLON << "repositoryType=" << m_umountRepoType << NODE_COLON
        << "repositoryTempDir=" << m_repoTempDir << NODE_COLON << "jobid=" << jobId << NODE_COLON;
        if (isFileClientMount) {
            scriptParam << "isFileClientMount=true" << NODE_COLON;
        }
        CRootCaller rootCaller;
        mp_int32 ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UMOUNT_NAS_FILESYS, scriptParam.str(), NULL);
        if (ret != MP_SUCCESS) {
            vecErrMountPath.push_back(strPath);
            WARNLOG("Umount %s failed.", strPath.c_str());
        } else {
            INFOLOG("Unmount %s success.", strPath.c_str());
            HandleAfterUMountSuccess(jobId, strPath);
        }
    }

    if (!vecErrMountPath.empty()) {
        ERRLOG("Unmount %s failed.", CMpString::StrJoin(vecErrMountPath, ",").c_str());
        return MP_FAILED;
    }
    INFOLOG("Unmount finish.");
    return MP_SUCCESS;
#endif
}

mp_string PrepareFileSystem::SplitFileClientMountPath(const mp_string &mountPath)
{
    std::vector<mp_string> vecPath;
    mp_string realPath;
    CMpString::StrSplit(vecPath, mountPath, '/');
    if (vecPath.size() < FILECLIENT_MOUNT_POINT_5) {
        WARNLOG("Umount point is not vaild: %s.", mountPath.c_str());
        return "";
    }
    for (uint32_t i = 1; i < FILECLIENT_MOUNT_POINT_5; i++) {
        realPath += PATH_SEPARATOR + vecPath[i];
    }
    return realPath;
}

mp_string PrepareFileSystem::GenerateSubPath(const MountNasParam &mountNasParam)
{
    mp_string strSubPath;
    if (!mountNasParam.permit.vecPath.empty()) {
        if (mountNasParam.nCopyFormat == CopyFormatType::type::INNER_SNAPSHOT) {
            return CMpString::StrJoin(mountNasParam.permit.vecPath, ":");
        }
        std::vector<mp_string> vecTokens;
        CMpString::StrSplitEx(vecTokens, mountNasParam.storagePath, "/");
        if (vecTokens.size() > 1) {
            vecTokens.erase(vecTokens.begin());
            std::vector<mp_string> vecSubPath;
            for (mp_string strPath : mountNasParam.permit.vecPath) {
                vecSubPath.push_back(CMpString::StrJoin(vecTokens, PATH_SEPARATOR) + PATH_SEPARATOR + strPath);
            }
            strSubPath = CMpString::StrJoin(vecSubPath, ":");
        } else {
            strSubPath = CMpString::StrJoin(mountNasParam.permit.vecPath, ":");
        }
    }
    return strSubPath;
}

/* ------------------------------------------------------------
Function Name: CreateLocalFileDir
Description  : 循环创建目录
example      : linux: /tmp/test/，依次创建/tmp/、/tmp/test/
             : windows: D:\tmp\test\
Others       :-------------------------------------------------------- */
mp_int32 PrepareFileSystem::CreateLocalFileDir(mp_string localPath)
{
    if (localPath.empty()) {
        ERRLOG("Local path parameter is empty!");
        return MP_FAILED;
    }
    if (localPath.size() > MAX_PATH_LEN) {
        ERRLOG("File path length is %d, more than %d.", localPath.size(), MAX_PATH_LEN);
        return MP_FAILED;
    }
    if (localPath.find(PATH_SEPARATOR) != string::npos) {
        if (localPath.substr(localPath.size() - 1, 1) != PATH_SEPARATOR) {
            localPath.append(PATH_SEPARATOR);
        }
    }
    mp_string::size_type separatorPos = 0;
    mp_int32 separatorCount = 0;
    while ((separatorPos = localPath.find_first_of(PATH_SEPARATOR, separatorPos)) != string::npos) {
        if (separatorCount <= 0) {
            ++separatorCount;
            ++separatorPos;
            continue;
        } else {
            mp_string tmpDirPath = localPath.substr(0, separatorPos);
            (void)CMpFile::CreateDir(tmpDirPath.c_str());
            if (!CMpFile::DirExist(tmpDirPath.c_str())) {
                ERRLOG("Create director %s failed", tmpDirPath.c_str());
                return MP_FAILED;
            }
            ++separatorPos;
            ++separatorCount;
        }
    }
    return MP_SUCCESS;
}

std::mutex PrepareFileSystem::m_mountInfoMapMutex;
std::map<mp_string, mp_string> PrepareFileSystem::m_alreadyMountInfoMap;

#ifdef WIN32
mp_int32 PrepareFileSystem::WinMountOperation(MountNasKeyParam& mountKeyParam, mp_string& mountPath)
{
    INFOLOG("isInerSnapshot:%d, isCloneFsMount:%d.", mountKeyParam.isInerSnapshot, mountKeyParam.isCloneFsMount);
    if (mountKeyParam.isInerSnapshot && !mountKeyParam.isCloneFsMount) {
        mp_string newStr = CMpString::StrReplace(mountKeyParam.storagePath, "/", "\\");
        while (newStr.size() > 0 && newStr.at(0) == '\\') {
            newStr.erase(0, 1);
        }
        INFOLOG("Fomat is inner snapshot, use storagePath(%s), not storageName(%s).",
            newStr.c_str(), mountKeyParam.storageName.c_str());
        mountKeyParam.storageName = newStr;
    }
    mp_string tempKey = GetWinMountScriptParama(mountKeyParam) + "StoragePath=" + mountKeyParam.storagePath;

    DBGLOG("start mount info(%s) mount.", tempKey.c_str());
    if (QueryMountInfoFromMap(tempKey, mountPath, mountKeyParam.useMemSave)) {
        INFOLOG("mount info(%s) already mount on %s.", tempKey.c_str(), mountPath.c_str());
        return MP_SUCCESS;
    }

    std::string driverLetter;
    mp_int32 iRet = WinMountOperationInner(mountKeyParam, driverLetter);
    if (iRet != MP_SUCCESS || driverLetter.empty()) {
        WARNLOG("Mount \\\\%s\\%s failed.", mountKeyParam.storageIp.c_str(), mountKeyParam.storageName.c_str());
        return MP_FAILED;
    }

    mp_string sharedInfo = "\\\\" + mountKeyParam.storageIp + PATH_SEPARATOR + mountKeyParam.storageName;
    JobOccupyMountPoint(sharedInfo, mountKeyParam.jobID);

    if (mountKeyParam.isInerSnapshot) {
        // 快照格式，不需要拼接目录
        mountPath = driverLetter;
    } else {
        // copy using backup storage directory
        std::vector<mp_string> vecTokens;
        CMpString::StrSplitEx(vecTokens, mountKeyParam.storagePath, "/");
        if (vecTokens.empty()) {
            WARNLOG("Invalid storagePath=%s.", mountKeyParam.storagePath.c_str());
            return MP_FAILED;
        }
        vecTokens[0] = driverLetter;
        mountPath = CMpString::StrJoin(vecTokens, PATH_SEPARATOR);
    }

    mountPath += "&" + sharedInfo;
    AddMountInfoToMap(tempKey, mountPath, mountKeyParam.useMemSave);
    if (!mountKeyParam.strSubPath.empty()) {
        string subPath = driverLetter + "\\" + mountKeyParam.strSubPath;
        CreateLocalFileDir(subPath);
    }
    INFOLOG("mount info(%s) success mount on %s.", tempKey.c_str(), mountPath.c_str());
    return MP_SUCCESS;
}

mp_int32 PrepareFileSystem::WinMountOperationInner(const MountNasKeyParam &mountKeyParam, mp_string &driveLetter)
{
    mp_float winVersion = WIN_SERVER_VERSION_2012;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueFloat(CFG_SYSTEM_SECTION, CFG_WIN_VERISON_VALUE,
        winVersion);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Get win version from config failed, use default version: %f.", WIN_SERVER_VERSION_2012);
        winVersion = WIN_SERVER_VERSION_2012;
    }
    if (winVersion - WIN_SERVER_VERSION_2012 < 0.0) {
        DBGLOG("Win mount use api in version %f.", winVersion);
        return WinMountUseAPI(mountKeyParam, driveLetter);
    }
    DBGLOG("Win mount use script in version %f.", winVersion);
    return WinMountUseScript(mountKeyParam, driveLetter);
}

mp_string PrepareFileSystem::GetWinMountScriptParama(const MountNasKeyParam &mountKeyParam)
{
    mp_string linkPath = MOUNT_PUBLIC_PATH + mountKeyParam.appType + PATH_SEPARATOR + mountKeyParam.jobID +
        PATH_SEPARATOR + mountKeyParam.repositoryType + PATH_SEPARATOR + mountKeyParam.storageName;
    std::hash<std::string> strHash;
    mp_string hashLinkPath = std::to_string(strHash(linkPath));
    m_linkPath = MOUNT_PUBLIC_PATH + hashLinkPath;
    ostringstream scriptParam;
    scriptParam << "JobID=" << mountKeyParam.jobID << NODE_COLON
        << "OperationType=" << "mount" << NODE_COLON
        << "SubPath=" << mountKeyParam.strSubPath << NODE_COLON
        << "StorageIp=" << mountKeyParam.storageIp << NODE_COLON
        << "FilesystemName=" << mountKeyParam.storageName << NODE_COLON
        << "LinkPath=" << m_linkPath << NODE_COLON
        << "AuthKey=" << mountKeyParam.cifsAuthKey;
    mp_string scriptParamStr = scriptParam.str();
    return scriptParamStr;
}

mp_int32 PrepareFileSystem::WinMountUseScript(const MountNasKeyParam& mountKeyParam, mp_string& driveLetter)
{
    vector<mp_string> vecResult;
    mp_int32 iRet = MP_FAILED;
    iRet = SecureCom::SysExecScript(SCRIPT_CIFS_OPERATION, GetWinMountScriptParama(mountKeyParam), &vecResult);
    if (iRet != MP_SUCCESS || vecResult.empty()) {
        WARNLOG("Mount \\\\%s\\%s failed.", mountKeyParam.storageIp.c_str(), mountKeyParam.storageName.c_str());
        return MP_FAILED;
    }
    driveLetter = vecResult.front();
    INFOLOG("Win mount %s use script success mount on %s.", mountKeyParam.storageName.c_str(), driveLetter.c_str());
    return MP_SUCCESS;
}

mp_int32 PrepareFileSystem::WinMountUseAPI(const MountNasKeyParam& mountKeyParam, mp_string& driveLetter)
{
    DWORD dwFlags = CONNECT_TEMPORARY;
    NETRESOURCEA netResource;
    mp_int32 iRet = memset_s(&netResource, sizeof(netResource), 0, sizeof(netResource));
    if (EOK != iRet) {
        COMMLOG(OS_LOG_ERROR, "Call memset_s failed, iRet = %d.", iRet);
        return MP_FAILED;
    }

    mp_string remoteName = "\\\\" + mountKeyParam.storageIp + "\\" + mountKeyParam.storageName;
    netResource.dwScope = RESOURCE_CONNECTED;
    netResource.dwType = RESOURCETYPE_ANY;
    netResource.dwUsage = 0;
    netResource.lpLocalName = NULL;
    netResource.lpProvider = NULL;
    netResource.lpRemoteName = const_cast<LPSTR>(remoteName.c_str());

    bool successFlag = false;
    for (int i = 0; i < CALL_API_RETRY_TIMES; i++) {
        DWORD dw = WNetAddConnection2A(&netResource,
            mountKeyParam.cifsAuthPwd.c_str(), mountKeyParam.cifsAuthKey.c_str(), dwFlags);
        if (dw == NO_ERROR) {
            successFlag = true;
            break;
        }
        ERRLOG("call WNetAddConnection2A return %d.", dw);
        if (dw == ERROR_ALREADY_ASSIGNED) {
            ERRLOG("call WNetAddConnection2A return ERROR_ALREADY_ASSIGNED, %d.", dw);
            continue;
        }
        SleepForMS(CALL_API_RETRY_INTERVAL);
    }
    if (!successFlag) {
        ERRLOG("call WNetAddConnection2A fail.");
        return MP_FAILED;
    }
    if (CreateLocalFileDir(m_linkPath) != MP_SUCCESS) {
        ERRLOG("Create link path %s failed.", m_linkPath.c_str());
        return MP_FAILED;
    }
    mp_string linkPath = m_linkPath + PATH_SEPARATOR + mountKeyParam.storageIp;
    if (!CMpFile::DirExist(linkPath.c_str()) && !CreateLink(linkPath, remoteName)) {
        ERRLOG("Create symbolic link %s to %s failed.", remoteName.c_str(), linkPath.c_str());
        return MP_FAILED;
    }
    driveLetter = linkPath;
    INFOLOG("Win mount %s use api success mount on %s.", remoteName.c_str(), driveLetter.c_str());
    return MP_SUCCESS;
}
#endif

void PrepareFileSystem::AddMountInfoToMap(const mp_string& mountInfo, const mp_string& mountPath, bool needSave)
{
    if (!needSave) {
        DBGLOG("Do not save mount info.");
        return;
    }
    std::lock_guard<std::mutex> lock(m_mountInfoMapMutex);
    m_alreadyMountInfoMap[mountInfo] = mountPath;
}

bool PrepareFileSystem::QueryMountInfoFromMap(const mp_string& mountInfo, mp_string& mountPath, bool needSave)
{
    if (!needSave) {
        DBGLOG("Do not use mount info in mem.");
        return false;
    }
    std::lock_guard<std::mutex> lock(m_mountInfoMapMutex);
    if (m_alreadyMountInfoMap.count(mountInfo) != 0) {
        mountPath = m_alreadyMountInfoMap[mountInfo];
        return true;
    }
    return false;
}

void PrepareFileSystem::EraseMountInfoFromMap(const mp_string& mountPath)
{
    std::lock_guard<std::mutex> lock(m_mountInfoMapMutex);
    for (auto it = m_alreadyMountInfoMap.begin(); it != m_alreadyMountInfoMap.end(); it++) {
        if (it->second == mountPath) {
            INFOLOG("Erase mount path %s from map.", mountPath.c_str());
            m_alreadyMountInfoMap.erase(it);
            break;
        }
    }
}

std::map<mp_string, std::set<mp_string>> PrepareFileSystem::m_mountPointOccupiedInfoMap;
std::mutex PrepareFileSystem::m_mountPointMapMutex;

void PrepareFileSystem::JobOccupyMountPoint(const mp_string& mountPoint, const mp_string& jobId)
{
    std::lock_guard<std::mutex> lock(m_mountPointMapMutex);
    if (m_mountPointOccupiedInfoMap.count(mountPoint) == 0) {
        std::set<mp_string> jobSet = { jobId };
        m_mountPointOccupiedInfoMap[mountPoint] = jobSet;
    } else {
        std::set<mp_string>& jobSet = m_mountPointOccupiedInfoMap[mountPoint];
        jobSet.insert(jobId);
    }
    int refCount = m_mountPointOccupiedInfoMap[mountPoint].size();
    INFOLOG("Job(%s) use mount point(%s), now reference count is %d.", jobId.c_str(), mountPoint.c_str(), refCount);
}

void PrepareFileSystem::JobReleaseMountPoint(const mp_string& mountPoint, const mp_string& jobId)
{
    std::lock_guard<std::mutex> lock(m_mountPointMapMutex);
    if (m_mountPointOccupiedInfoMap.count(mountPoint) == 0) {
        INFOLOG("No find mount point(%s) have been record.", mountPoint.c_str());
        return;
    }
    if (jobId.empty()) {
        INFOLOG("JobId is empty, Force Release mount point(%s).", mountPoint.c_str());
        m_mountPointOccupiedInfoMap.erase(mountPoint);
        return;
    }
    std::set<mp_string>& jobSet = m_mountPointOccupiedInfoMap[mountPoint];
    if (jobSet.count(jobId) == 0) {
        INFOLOG("No find job(%s) occupy info in mount point(%s).", jobId.c_str(), mountPoint.c_str());
        return;
    }
    jobSet.erase(jobId);
    INFOLOG("Erase job(%s) occupy info of mount point(%s), now reference count is %d.",
        jobId.c_str(), mountPoint.c_str(), jobSet.size());

    if (jobSet.empty()) {
        m_mountPointOccupiedInfoMap.erase(mountPoint);
        INFOLOG("No job use mount point(%s).", mountPoint.c_str());
    }
}

bool PrepareFileSystem::IsMountPointOccupied(const mp_string& mountPoint)
{
    std::lock_guard<std::mutex> lock(m_mountPointMapMutex);
    if (m_mountPointOccupiedInfoMap.count(mountPoint) == 0) {
        return false;
    }
    return true;
}

#ifdef WIN32
mp_void PrepareFileSystem::SplitMountPath(const mp_string &strPath, std::map<mp_string, mp_string> &mapMountPath)
{
    mp_string linkPath;
    vector<mp_string> mountInfoVec;
    CMpString::StrSplit(mountInfoVec, strPath, '&');
    linkPath = mountInfoVec.front();
    vector<mp_string> linkDirVec;
    CMpString::StrSplitEx(linkDirVec, linkPath, PATH_SEPARATOR);
    if (linkDirVec.size() > LINK_HASH_DIR_INDEX) {
        linkPath = MOUNT_PUBLIC_PATH + linkDirVec[LINK_HASH_DIR_INDEX];
    }
    mapMountPath.insert(std::pair<mp_string, mp_string>(linkPath, mountInfoVec.back()));
}

mp_bool PrepareFileSystem::CreateLink(const mp_string &linkPath, const mp_string &targetPath)
{
    mp_int32 pSize = MultiByteToWideChar(CP_OEMCP, 0, targetPath.c_str(), strlen(targetPath.c_str()) + 1, NULL, 0);
    mp_wchar *wcTargetPath = new mp_wchar[pSize + 1];
    MultiByteToWideChar(CP_OEMCP, 0, targetPath.c_str(), strlen(targetPath.c_str()) + 1, wcTargetPath, pSize);
    pSize = MultiByteToWideChar(CP_OEMCP, 0, linkPath.c_str(), strlen(linkPath.c_str()) + 1, NULL, 0);
    wchar_t *wcLinkPath = new mp_wchar[pSize + 1];
    MultiByteToWideChar(CP_OEMCP, 0, linkPath.c_str(), strlen(linkPath.c_str()) + 1, wcLinkPath, pSize);
    if (!CreateSymbolicLink(wcLinkPath, wcTargetPath, SYMBOLIC_LINK_FLAG_DIRECTORY)) {
        delete[] wcTargetPath;
        delete[] wcLinkPath;
        ERRLOG("CreateSymbolicLink failed, error: %d.", GetLastError());
        return MP_FALSE;
    }
    delete[] wcTargetPath;
    delete[] wcLinkPath;
    return MP_TRUE;
}

mp_int32 PrepareFileSystem::WinUmountOperation(const vector<mp_string> &successMountPath, const mp_string &jobId)
{
    LOGGUARD("");
    std::map<mp_string, mp_string> mapMountPath;
    for (mp_string strPath : successMountPath) {
        if (!strPath.empty()) {
            EraseMountInfoFromMap(strPath);
            SplitMountPath(strPath, mapMountPath);
        }
    }
    vector<mp_string> vecErrMountPath;
    for (auto iter = mapMountPath.begin(); iter != mapMountPath.end(); iter++) {
        ostringstream scriptParam;
        JobReleaseMountPoint(iter->second, jobId);
        if (IsMountPointOccupied(iter->second)) {
            INFOLOG("Another job still use mount point(%s), skip umount.", iter->second.c_str());
            continue;
        }

        mp_string umountPath = iter->second;
        umountPath = CMpString::StrReplace(umountPath, "/", "\\");
        mp_int32 ret = MP_FAILED;
        if ((mp_string::npos != umountPath.find("\\data\\")) || (mp_string::npos != umountPath.find("\\log\\"))) {
            scriptParam << "umountPath=" << umountPath << NODE_COLON;
            ret = SecureCom::SysExecScript(SCRIPT_DATATURBO_UMOUNT, scriptParam.str(), NULL);
        } else {
            scriptParam << "OperationType=" << "unmount" << NODE_COLON << "LinkPath=" << iter->first << NODE_COLON
            << "SharedPath=" << iter->second << NODE_COLON;
            ret = SecureCom::SysExecScript(SCRIPT_CIFS_OPERATION, scriptParam.str(), nullptr);
        }

        if (ret != MP_SUCCESS) {
            vecErrMountPath.push_back(iter->first);
            WARNLOG("Umount %s failed.", iter->first.c_str());
        } else {
            INFOLOG("Unmount %s success.", iter->first.c_str());
        }
    }

    if (!vecErrMountPath.empty()) {
        ERRLOG("Unmount %s failed.", CMpString::StrJoin(vecErrMountPath, ",").c_str());
        return MP_FAILED;
    }
    INFOLOG("Unmount finish.");
    return MP_SUCCESS;
}
#endif

std::map<mp_string, uint64_t> PrepareFileSystem::m_mountFailIpMap;
std::mutex PrepareFileSystem::m_mountFailIpMapMutex;

void PrepareFileSystem::AdjustMountIpList(std::vector<mp_string>& mountIpList)
{
    ReleaseMountFailIp();

    std::vector<mp_string> tmpIpList;
    std::lock_guard<std::mutex> lock(m_mountFailIpMapMutex);
    for (const auto& it : m_mountFailIpMap) {
        mp_string dstIp = it.first;
        auto iter = std::find(mountIpList.begin(), mountIpList.end(), dstIp);
        if (iter != mountIpList.end()) {
            mountIpList.erase(iter);
            tmpIpList.push_back(dstIp);
        }
    }

    for (const auto& ip : tmpIpList) {
        mountIpList.push_back(ip);
        DBGLOG("Put ip(%s) at the end.", ip.c_str());
    }

    DBGLOG("Mount ip list size is %d.", mountIpList.size());
}

void PrepareFileSystem::AddMountFailIp(const mp_string& ip)
{
    std::lock_guard<std::mutex> lock(m_mountFailIpMapMutex);
    uint64_t nowTime = time(nullptr);
    m_mountFailIpMap[ip] = nowTime;
    INFOLOG("Add mount fail ip(%s).", ip.c_str());
}

void PrepareFileSystem::ReleaseMountFailIp(const mp_string& ip)
{
    if (!ip.empty()) {
        std::lock_guard<std::mutex> lock(m_mountFailIpMapMutex);
        if (m_mountFailIpMap.count(ip) != 0) {
            m_mountFailIpMap.erase(ip);
            INFOLOG("Release mount ip(%s) due to success.", ip.c_str());
        }
        return;
    }

    std::lock_guard<std::mutex> lock(m_mountFailIpMapMutex);
    uint64_t nowTime = time(nullptr);
    for (auto it = m_mountFailIpMap.begin(); it != m_mountFailIpMap.end();) {
        mp_string ip = it->first;
        uint64_t rTime = it->second;
        if (nowTime - rTime > ONE_HOUR) {
            it = m_mountFailIpMap.erase(it);
            INFOLOG("Release mount ip(%s) due to timeout.", ip.c_str());
        } else {
            it++;
        }
    }
}

void PrepareFileSystem::HandleAfterMountSuccess(const MountNasParam &mountNasParam)
{
    MountChainInfo chainInfo(mountNasParam);
    JobAddMountChainInfo(mountNasParam.jobID, chainInfo);
    ReleaseMountFailIp(mountNasParam.ip);
}

void PrepareFileSystem::HandleAfterMountFail(const MountNasParam &mountNasParam, mp_int32 errCode)
{
    AddMountFailIp(mountNasParam.ip);
}

void PrepareFileSystem::HandleAfterUMountSuccess(const mp_string& jobId, const mp_string& mountPath)
{
    JobReleaseMountChainInfo(jobId);
}

MountChainInfo::MountChainInfo(const MountNasParam& mountNasParam)
{
    repositoryType = mountNasParam.repositoryType;
    protocolType = mountNasParam.protocolType;
    storageIp = mountNasParam.ip;
    if (G_ProtocolTypePortMap.count(mountNasParam.protocolType) == 0) {
        ERRLOG("unknown mount protocol type %s.", mountNasParam.protocolType.c_str());
        return;
    }
    protocolPort = G_ProtocolTypePortMap[mountNasParam.protocolType];

    sharePath = mountNasParam.storagePath;
}

// job mount chain info
std::map<mp_string, std::vector<MountChainInfo>> PrepareFileSystem::m_mountChainInfoMap;
std::mutex PrepareFileSystem::m_mountChainMapMutex;

void PrepareFileSystem::JobAddMountChainInfo(const mp_string& jobId, const MountChainInfo& mountChainInfo)
{
    DBGLOG("Job %s repo %s use ip %s mount.", jobId.c_str(),
        mountChainInfo.repositoryType.c_str(), mountChainInfo.storageIp.c_str());

    std::lock_guard<std::mutex> lock(m_mountChainMapMutex);
    if (m_mountChainInfoMap.count(jobId) == 0) {
        std::vector<MountChainInfo> chainSet = { mountChainInfo };
        m_mountChainInfoMap[jobId] = chainSet;
        return;
    }
    std::vector<MountChainInfo>& chainSet = m_mountChainInfoMap[jobId];
    for (const MountChainInfo& chain : chainSet) {
        if (chain.repositoryType == mountChainInfo.repositoryType) {
            return;
        }
    }
    chainSet.push_back(mountChainInfo);
}

void PrepareFileSystem::JobReleaseMountChainInfo(const mp_string& jobId)
{
    std::lock_guard<std::mutex> lock(m_mountChainMapMutex);
    if (m_mountChainInfoMap.count(jobId) != 0) {
        m_mountChainInfoMap.erase(jobId);
    }
    DBGLOG("Job %s release ip mount.", jobId.c_str());
}

bool PrepareFileSystem::IsMountChainGood(const mp_string& jobId)
{
    std::vector<MountChainInfo> chainSet;
    {
        std::lock_guard<std::mutex> lock(m_mountChainMapMutex);
        if (m_mountChainInfoMap.count(jobId) == 0) {
            ERRLOG("No job %s find in chain info, No need check.", jobId.c_str());
            return true;
        }
        chainSet = m_mountChainInfoMap[jobId];
    }

    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        ERRLOG("Create new http client failed.");
        return false;
    }

    for (const auto& chainInfo : chainSet) {
        const mp_string& ip = chainInfo.storageIp;
        const mp_string& port = chainInfo.protocolPort;
        const mp_string& repo = chainInfo.repositoryType;
        bool good = false;
        mp_int32 retryTimes = 0;
        while (retryTimes < COMMON_MID_RETRY_TIMES) {
            if (httpClient->TestConnectivity(ip, port)) {
                INFOLOG("Can cennect ip(%s) port(%s).", ip.c_str(), port.c_str());
                good = true;
                break;
            }
            WARNLOG("Can not cennect ip(%s) port(%s).", ip.c_str(), port.c_str());
            retryTimes++;
            DoSleep(COMMON_LOW_RETRY_INTERVAL);
        }

        if (!good) {
            ERRLOG("Mount chain ip(%s) port(%s) repo(%s) is not good.", ip.c_str(), port.c_str(), repo.c_str());
            return false;
        }
        INFOLOG("Mount chain ip(%s) port(%s) repo(%s) is good.", ip.c_str(), port.c_str(), repo.c_str());
    }

    INFOLOG("All mount chain is good.");
    return true;
}
