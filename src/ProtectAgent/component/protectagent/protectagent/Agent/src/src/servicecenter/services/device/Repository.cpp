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
#include "servicecenter/services/device/Repository.h"
#include <chrono>
#include "host/host.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "pluginfx/ExternalPluginManager.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"

namespace {
    const mp_int32 SUPPORT_DATATURBO_PROTOCOL = 1024;
    const mp_int32 RESOURCE_IDENTIFY_LENGTH = 5;
    const mp_int32 CORRECT_IPANDESN_SIZE = 2;
}

namespace AppProtect {
Repository::Repository()
{}

Repository::~Repository()
{}

mp_int32 Repository::Mount(PluginJobData &data, StorageRepository &stRep,
    Json::Value &jsonRep_new, const MountPermission &permit)
{
    MountNasParam param;
    if (GetMountNasParam(param, data, stRep, permit) != MP_SUCCESS) {
        ClearPwd(param);
        ERRLOG("Get mount nas param failed, jobId=%s, subJobId=%s.", data.mainID.c_str(), data.subID.c_str());
        return MP_FAILED;
    }
#ifndef WIN32
    if (data.IsSanClientMount() && (param.repositoryType == "data" || param.repositoryType == "log")) {
        return MountFileIoSystem(param, data, stRep, jsonRep_new);
    } else if (param.isFileClientMount) {
        return MountFileClient(param, data, stRep, jsonRep_new);
    }
#endif
    std::vector<mp_string> mountPoints, dtbMountPoints;
    PrepareFileSystem mountHandler;
    param.isDeduptionOn = IsDataturboMount(data);
    mp_bool ipLinkCondition = param.isDeduptionOn && ClusterSelectStorage(param.esn, data.appType) &&
                            ((stRep.repositoryType == RepositoryDataType::type::DATA_REPOSITORY) ||
                            (stRep.repositoryType == RepositoryDataType::type::LOG_REPOSITORY));
    if (!data.IsSanClientMount() && (param.isFcOn || ipLinkCondition)) {  // 若开启源端重删功能，则使用源端重删方式挂载
        mp_int32 iRet = mountHandler.MountDataturboFileSystem(param, mountPoints, dtbMountPoints);
        if (iRet == MP_SUCCESS) {
            ClearPwd(param);
            InsertMountPoint(data, stRep, mountPoints);
            data.dtbMountPoints.insert(make_pair(stRep.repositoryType, dtbMountPoints));
            return AssembleRepository(data, stRep, jsonRep_new);
        }
        // 若文件系统使用Dataturbo挂载失败，不直接返回失败，直接使用NAS挂载
        // 上报label信息， 错误码，错误码参数（文件系统）
        std::vector<mp_string> errorParams = {param.storagePath};
        ReportMainJobDetail(DATATURBO_FAILED_LABEL, iRet, errorParams, JobLogLevel::type::TASK_LOG_WARNING, data);
        // FC挂载失败则直接报错
        if (param.isFcOn) {
            ERRLOG("Mount dataturbo file system failed, appType=%s, jobId=%s, subJobId=%s.",
                data.appType.c_str(), data.mainID.c_str(), data.subID.c_str());
            return MP_FAILED;
        }
    }
#ifdef WIN32
    std::lock_guard<std::mutex> lk(AppProtect::AppProtectJobHandler::GetInstance()->GetMutexOfMountCifs());
#endif
    ModifyParam(param);
    mp_int32 ret = mountHandler.MountNasFileSystem(param, mountPoints);
    ClearPwd(param);
    if (ret != MP_SUCCESS) {
        ERRLOG("Mount nas file system failed, appType=%s, jobId=%s, subJobId=%s.",
            data.appType.c_str(), data.mainID.c_str(), data.subID.c_str());
        return ret;
    }
    InsertMountPoint(data, stRep, mountPoints);
    return AssembleRepository(data, stRep, jsonRep_new);
}

mp_void Repository::ModifyParam(MountNasParam &param)
{
    if (!param.isDeduptionOn && param.cifsAuthKey.empty()) {
        param.cifsAuthKey = param.authKey;
        param.cifsAuthPwd = param.authPwd;
    }
}

mp_void Repository::ClearPwd(MountNasParam &param)
{
    ClearString(param.authPwd);
    ClearString(param.cifsAuthPwd);
}

mp_void Repository::InsertMountPoint(PluginJobData &data, StorageRepository &stRep,
    const std::vector<mp_string> &mountPoints)
{
    m_mountPoint = mountPoints;
    data.mountPoints.insert(make_pair(stRep.repositoryType, mountPoints));
}

mp_int32 Repository::GetSanclientParam(MountNasParam &param, const PluginJobData &data)
{
    LOGGUARD("");
    if (!data.param.isMember("agents") || !data.param["agents"].isArray()) {
        ERRLOG("Param invalid, no agent info.");
        return MP_FAILED;
    }
    Json::Value agentInfo = data.param["agents"];
    mp_string agentId;
    Json::Value lunInfo;
    CHost hostHandeler;
    mp_string hostId;
    if (hostHandeler.GetHostSN(hostId) != MP_SUCCESS) {
        ERRLOG("Get agent id failed, check param.");
        return MP_FAILED;
    }
    for (auto &agent : agentInfo) {
        if (!agent.isObject()) {
            continue;
        }
        GET_JSON_STRING(agent, "id", agentId);
        if (agentId == hostId) {
            if (agent.isMember("sanClients") && agent["sanClients"].isArray()) {
                lunInfo["sanClients"] = agent["sanClients"];
                break;
            }
        }
    }
    if (!lunInfo.isMember("sanClients")) {
        ERRLOG("Param invalid, no lunInfo.");
        return MP_FAILED;
    }
    Application application;
    auto appInfo = data.param["appInfo"];
    JsonToStruct(appInfo, application);
    mp_string resourceId = application.id;
    mp_string vgId = resourceId.substr(resourceId.size() - RESOURCE_IDENTIFY_LENGTH, RESOURCE_IDENTIFY_LENGTH);
    mp_string lunInfoString = GetMountLunInfo(lunInfo, param, vgId);
    if (lunInfoString.empty()) {
        ERRLOG("No luninfo append, sanclient param parse failed.");
        return MP_FAILED;
    }
    param.lunInfo = lunInfoString;
    return MP_SUCCESS;
}

#ifndef WIN32
mp_bool Repository::CheckSanclientMounted(
    PluginJobData &data, const mp_string &repositoryType)
{
    auto it = data.mountPoints.find(RepositoryDataType::type::META_REPOSITORY);
    if (it == data.mountPoints.end()) {
        ERRLOG("Not mount meta path, no need check sanclient mount.");
        return MP_FALSE;
    }
    for (const mp_string &metaMountPath : it->second) {
        mp_string isMountedFile = metaMountPath + PATH_SEPARATOR + repositoryType + "_mounted.lock";
        if (CMpFile::FileExist(isMountedFile)) {
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}
#endif

#ifndef WIN32
mp_int32 Repository::RecordSanclientMounted(
    PluginJobData &data, const mp_string &repositoryType, mp_string &isMountedFile)
{
    auto it = data.mountPoints.find(RepositoryDataType::type::META_REPOSITORY);
    if (it == data.mountPoints.end()) {
        ERRLOG("Not mount meta path, no need check sanclient mount.");
        return MP_FAILED;
    }
    isMountedFile = it->second.front() + PATH_SEPARATOR + data.mainID + CHAR_UNDERLINE +
        repositoryType + "_mounted.lock";
    return MP_SUCCESS;
}
#endif

#ifndef WIN32
mp_int32 Repository::MountFileIoSystem(
    MountNasParam &param, PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new)
{
    if (GetSanclientParam(param, data) != MP_SUCCESS) {
        ERRLOG("Parse sanclient mode job param failed, appType=%s, jobId=%s, subJobId=%s.",
            data.appType.c_str(), data.mainID.c_str(), data.subID.c_str());
        return MP_FAILED;
    }
    std::vector<mp_string> mountPoints;
    PrepareFileSystem mountHandler;
    mp_string mountedRecordFile;
    mp_int32 ret = RecordSanclientMounted(data, param.repositoryType, mountedRecordFile);
    if (ret != MP_SUCCESS) {
        ERRLOG("Create mounted file failed, no need mounted on this node.");
        return MP_FAILED;
    }
    ret = mountHandler.MountFileIoSystem(param, mountPoints, mountedRecordFile);
    ClearString(param.authPwd);
    if (ret != MP_SUCCESS) {
        ERRLOG("Mount sanclient fileio system failed, appType=%s, jobId=%s, subJobId=%s.",
            data.appType.c_str(), data.mainID.c_str(), data.subID.c_str());
        return ret;
    }
    m_mountPoint = mountPoints;
    for (auto &mountPath : mountPoints) {
        mountPath += ("#" + param.lunInfo);
        DBGLOG("MountPath append info is :%s", mountPath.c_str());
    }
    data.mountPoints.insert(make_pair(stRep.repositoryType, mountPoints));
    return AssembleRepository(data, stRep, jsonRep_new);
}
#endif

#ifndef WIN32
mp_int32 Repository::MountFileClient(
    MountNasParam &param, PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new)
{
    std::vector<mp_string> mountPoints;
    std::vector<mp_string> realMountPoints;
    PrepareFileSystem mountHandler;
    mp_int32 ret = mountHandler.MountFileClientSystem(param, mountPoints, realMountPoints);
    ClearPwd(param);
    if (ret != MP_SUCCESS) {
        ERRLOG("Mount nas file system failed, appType=%s, jobId=%s, subJobId=%s.",
            data.appType.c_str(),
            data.mainID.c_str(),
            data.subID.c_str());
        return ret;
    }
 
    m_mountPoint = mountPoints;
    data.mountPoints.insert(make_pair(stRep.repositoryType, mountPoints));
    return AssembleRepository(data, stRep, jsonRep_new);
}

mp_void Repository::GetParamFromExtenInfo(MountNasParam &param, const PluginJobData &data)
{
    if (data.param.isMember("extendInfo") && data.param["extendInfo"].isObject()) {
        Json::Value extendInfo = data.param["extendInfo"];
        if (extendInfo.isMember("mountType") && extendInfo["mountType"].isString()) {
            mp_string mountType = extendInfo["mountType"].asString();
            param.isFullPath = (mountType.compare("1") == 0) ? true : false;
        }
        if (extendInfo.isMember("agentMountType") && extendInfo["agentMountType"].isString()) {
            mp_string mountType = extendInfo["agentMountType"].asString();
            param.isFileClientMount = (mountType.compare("fuse") == 0) ? true : false;
        }
        if (extendInfo.isMember("OSADAuthPort") && extendInfo["OSADAuthPort"].isInt()) {
            param.OSADAuthPort = extendInfo["OSADAuthPort"].asInt();
        }
        if (extendInfo.isMember("OSADServerPort") && extendInfo["OSADServerPort"].isInt()) {
            param.OSADServerPort = extendInfo["OSADServerPort"].asInt();
        }
        if (extendInfo.isMember("pvcTaskId") && extendInfo["pvcTaskId"].isString()) {
            param.pvcTaskId = extendInfo["pvcTaskId"].asString();
        }
        if (extendInfo.isMember("pvcOSADAuthPort") && extendInfo["pvcOSADAuthPort"].isInt()) {
            param.pvcOSADAuthPort = extendInfo["pvcOSADAuthPort"].asInt();
        }
        if (extendInfo.isMember("pvcOSADServerPort") && extendInfo["pvcOSADServerPort"].isInt()) {
            param.pvcOSADServerPort = extendInfo["pvcOSADServerPort"].asInt();
        }
    }
}
#endif

mp_void Repository::ReportMainJobDetail(const mp_string &label, const mp_int32 &errorCode,
    std::vector<std::string> &errorParams, AppProtect::JobLogLevel::type level, PluginJobData &data)
{
    ERRLOG("Mount Filesystem through Dataturbo Protocol failed, appType=%s, jobId=%s, subJobId=%s.",
        data.appType.c_str(), data.mainID.c_str(), data.subID.c_str());
    AppProtect::LogDetail tmplogDetail;
    for (const auto &item : data.logDetail) {  // 去重
        JsonToStruct(item, tmplogDetail);
        if (tmplogDetail.description == label) {
            WARNLOG("Main Job Detail has contain the label %s.", label.c_str());
            return;
        }
    }
    
    std::string listenIP;
    std::string listenPort;
    CIP::GetListenIPAndPort(listenIP, listenPort);
    std::vector<std::string> params = { listenIP };
    
    AppProtect::LogDetail logDetail;
    if (errorCode != MP_FAILED) {
        logDetail.__set_errorCode(errorCode);
        logDetail.__set_errorParams(errorParams);
    }
    logDetail.__set_description(label);
    logDetail.__set_level(level);
    logDetail.__set_params(params);
    std::chrono::system_clock::duration d = std::chrono::system_clock::now().time_since_epoch();
    std::chrono::milliseconds mil = std::chrono::duration_cast<std::chrono::milliseconds>(d);
    logDetail.__set_timestamp(mil.count());
    Json::Value labeldetail;
    StructToJson(logDetail, labeldetail);
    data.logDetail.push_back(labeldetail);
}

mp_int32 Repository::Umount(
    const std::vector<mp_string> &mountPoints, const mp_string &jobID, const bool &isFileClientMount)
{
    PrepareFileSystem mountHandler;
    mp_int32 ret = mountHandler.UmountNasFileSystem(mountPoints, jobID, isFileClientMount);
    if (ret != MP_SUCCESS) {
        WARNLOG("Umount repository failed, jobId=%s.", jobID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_void Repository::QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path)
{
    return;
}

mp_int32 Repository::AssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new)
{
    Json::Value afterHandleJson;
    StructToJson(stRep, afterHandleJson);
    afterHandleJson["path"] = Json::arrayValue;
    for (auto mountPt : m_mountPoint) {
        mp_string perMountPath;
        mp_string subDirPath;
        // subDirPath由UBC统一下发，agent进行拼接
        if (afterHandleJson.isMember("subDirPath")) {
            subDirPath = afterHandleJson["subDirPath"].asString();
            DBGLOG("subDirPath is %s", subDirPath.c_str());
        }
#ifdef WIN32
        std::vector<mp_string> mountInfoVec;
        CMpString::StrSplit(mountInfoVec, mountPt, '&');
        perMountPath = mountInfoVec.front();
        subDirPath = CMpString::StrReplace(subDirPath, "/", "\\");
        perMountPath += subDirPath;
#else
        perMountPath = mountPt;
#endif
        DBGLOG("AssembleRepository perMountPath is %s", perMountPath.c_str());
        afterHandleJson["path"].append(perMountPath);
    }
    INFOLOG("Mount path count is %d.", m_mountPoint.size());
    jsonRep_new.append(std::move(afterHandleJson));
    return MP_SUCCESS;
}

mp_int32 Repository::GetMountIP(const StorageRepository& stRep, MountNasParam& param)
{
    mp_bool isdorado = true;
    auto iRet = CIP::CheckIsDoradoEnvironment(isdorado);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get CheckIsDoradoEnvironment Failed.");
        return MP_FAILED;
    }
    for (const auto& host:stRep.remoteHost) {
        if (host.supportProtocol == SUPPORT_DATATURBO_PROTOCOL) {
            DBGLOG("Add dataturbo ip %s.", host.ip.c_str());
            param.vecDataturboIP.push_back(host.ip);
        } else {
            DBGLOG("Add nfs/cifs ip %s.", host.ip.c_str());
            param.vecStorageIp.push_back(host.ip);
        }
    }
    return MP_SUCCESS;
}

mp_int32 Repository::GetMountNasParam(MountNasParam& param, const PluginJobData &data,
    const StorageRepository &stRep, const MountPermission &permit)
{
    LOGGUARD("");
    param.appType = data.appType;
    param.jobID = data.mainID;
    param.storagePath = stRep.remotePath;
    param.storageName = stRep.remoteName;
    param.repositoryType = STORAGE_TYPE_ARRAY[mp_int32(stRep.repositoryType)];
    if (GetMountIP(stRep, param) != MP_SUCCESS) {
        ERRLOG("Get Mount nas file system ip failed");
        return MP_FAILED;
    }
    GetParamFromRepoExtenInfo(param, stRep);

    param.authKey = stRep.auth.authkey;
    param.authPwd = stRep.auth.authPwd;
    param.cifsAuthKey = stRep.cifsAuth.authkey;
    param.cifsAuthPwd = stRep.cifsAuth.authPwd;
    mp_int32 taskType = 0; // Default taskType=UNDEFINE
    CJsonUtils::GetJsonInt32(data.param, "taskType", taskType);
    param.taskType = taskType;
#ifdef WIN32
    GetCopyFormat(param, data, stRep);
#else
    mp_string runPluginName;
    mp_int32 iRet = ExternalPluginManager::GetInstance().GetPluginNameByAppType(data.appType, runPluginName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to get mount plugin name.");
        return MP_FAILED;
    }

    param.runAccount = ExternalPluginManager::GetInstance().GetStartUser(runPluginName);
    if (param.runAccount.empty()) {
        ERRLOG("Fail to get start user for %s.", runPluginName);
        return MP_FAILED;
    }
    GetParamFromExtenInfo(param, data);
#endif // WIN32
    param.permit = permit;
    param.isLinkEncryption = IsLinkEncryption(data);
    param.isFcOn = data.IsCurAgentFcOn();
    QueryRepoSubPath(data, param.permit.vecPath);
    return MP_SUCCESS;
}

mp_int32 Repository::GetParamFromRepoExtenInfo(MountNasParam& param, const StorageRepository &stRep)
{
    RepositoryExtendInfo eInfo;
    if (!JsonHelper::JsonStringToStruct(stRep.extendInfo, eInfo)) {
        ERRLOG("Json String To Struct fail.");
        return MP_FAILED;
    }
    param.isCloneFsMount = eInfo.isCloneFileSystem;
    param.esn = eInfo.esn;
    param.nCopyFormat = eInfo.copyFormat;
    DBGLOG("Repo extend info: esn(%s) copyFormat(%d) isCloneFs(%d).", eInfo.esn.c_str(), eInfo.copyFormat,
        eInfo.isCloneFileSystem);
    return MP_SUCCESS;
}

mp_void Repository::GetCopyFormat(MountNasParam& param, const PluginJobData &data,
    const StorageRepository &stRep)
{
    // 查询副本格式，快照还是目录结构
    mp_int32 taskType = 1;
    CJsonUtils::GetJsonInt32(data.param, "taskType", taskType);
    if ((MainJobType)taskType == MainJobType::BACKUP_JOB) {
        if (data.param.isMember("taskParams")) {
            CJsonUtils::GetJsonInt32(data.param["taskParams"], "copyFormat", param.nCopyFormat);
        }
    } else {
        if (!data.param["copies"].empty()) {
            CJsonUtils::GetJsonInt32(data.param["copies"][0], "format", param.nCopyFormat);
        }
    }
    if (stRep.repositoryType == RepositoryDataType::type::CACHE_REPOSITORY) {
        // cache 固定为目录格式
        param.nCopyFormat = CopyFormatType::type::INNER_DIRECTORY;
    }
}

mp_int32 Repository::GetRepositoryExtenInfo(const StorageRepository &stRep, mp_string key, mp_string &value)
{
    Json::Value extendInfo;
    if (JsonHelper::JsonStringToJsonValue(stRep.extendInfo, extendInfo) != true) {
        ERRLOG("Get %s from StorageRepository extendInfo failed.", key.c_str());
        return MP_FAILED;
    }

    if (extendInfo.isMember(key) && extendInfo[key].isString()) {
        value = extendInfo[key].asString();
    } else {
        ERRLOG("StorageRepository extendInfo not found %s.", key.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 Repository::AssembleRemoteHost(const std::set<mp_string>& availStorageIp, StorageRepository &stRep)
{
    mp_bool isdorado = MP_TRUE;
    auto iRet = CIP::CheckIsDoradoEnvironment(isdorado);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get CheckIsDoradoEnvironment Failed.");
        return MP_FAILED;
    }
    if (isdorado) {
        stRep.remoteHost.clear();
        HostAddress hostAddressJson;
        for (auto storageIp : availStorageIp) {
            hostAddressJson.ip = std::move(storageIp);
            hostAddressJson.port = 0;
            stRep.remoteHost.push_back(hostAddressJson);
        }
    } else {
        // erase unreachable storage ip
        for (auto iter = stRep.remoteHost.begin(); iter != stRep.remoteHost.end();) {
            if (std::find(availStorageIp.begin(), availStorageIp.end(), iter->ip) == availStorageIp.end()) {
                WARNLOG("Storage ip %s is unreachable, remove from remostHost.", iter->ip.c_str());
                iter = stRep.remoteHost.erase(iter);
                continue;
            }
            ++iter;
        }
    }
    return MP_SUCCESS;
}

mp_bool Repository::IsDataturboMount(PluginJobData &data)
{
    if (data.param.isMember("taskParams") && data.param["taskParams"].isMember("dataLayout")) {
        if (data.param["taskParams"]["dataLayout"].isMember("srcDeduption") &&
            data.param["taskParams"]["dataLayout"]["srcDeduption"].isBool()) {
            if (data.param["taskParams"]["dataLayout"]["srcDeduption"].asBool()) {
                data.param["taskParams"]["dataLayout"]["extendInfo"]["srcDeduption"] = true;
                DBGLOG("This backup need use dataturbo mount.");
                return MP_TRUE;
            }
        }
        data.param["taskParams"]["dataLayout"]["extendInfo"]["srcDeduption"] = false;
    }
    DBGLOG("This backup do not need use dataturbo mount.");
    return MP_FALSE;
}

mp_bool Repository::IsLinkEncryption(const PluginJobData &data)
{
    mp_bool isEncryption = false;
    if (data.param.isMember("taskParams") && data.param["taskParams"].isMember("linkEncryption") &&
        data.param["taskParams"]["linkEncryption"].isBool()) {
        isEncryption = data.param["taskParams"]["linkEncryption"].asBool();
    }
    return isEncryption;
}

mp_int32 Repository::GetHostAndStorageMap(std::map<mp_string, mp_string> &ipAndEsnMap)
{
    mp_string hostAndStorageMap;
    mp_int32 ret = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_HOST_STORAGE_MAP,
        hostAndStorageMap);
    if (ret != MP_SUCCESS) {
        WARNLOG("Failed to get host and storage map.");
        return MP_FAILED;
    }

    if (hostAndStorageMap.empty()) {
        WARNLOG("host and storage map is empty.");
        return MP_FAILED;
    }

    std::vector<mp_string> hostItem;
    CMpString::StrSplit(hostItem, hostAndStorageMap, ';');
    if (hostItem.empty()) {
        WARNLOG("Get ip info failed!");
        return MP_FAILED;
    }

    for (auto item : hostItem) {
        std::vector<mp_string> ipAndEsn;
        CMpString::StrSplit(ipAndEsn, item, '@');
        DBGLOG("The ipAndEsn is %s, the size is %d", item.c_str(), ipAndEsn.size());
        if (ipAndEsn.size() != CORRECT_IPANDESN_SIZE) {
            continue;
        }
        ipAndEsnMap.insert(std::map<mp_string, mp_string>::value_type (ipAndEsn[0], ipAndEsn[1]));
        DBGLOG("Get ip and storage releation %s : %s!", ipAndEsn[0].c_str(), ipAndEsn[1].c_str());
    }

    if (ipAndEsnMap.empty()) {
        WARNLOG("Get ip info failed!");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_bool Repository::ClusterSelectStorage(mp_string &esn, mp_string &subType)
{
    if (subType.find("DWS") == std::string::npos) {  // 当前只有DWS支持集群环境
        return MP_TRUE;
    }

    // 获取本节点IP与存储ESN的映射关系
    std::map<mp_string, mp_string> ipAndEsnMap;
    mp_int32 iRet = GetHostAndStorageMap(ipAndEsnMap);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Get the map between host ip and storage failed!");
        return MP_FALSE;
    }

    // 获取本节点IP列表
    std::vector<mp_string> ipv4List;
    std::vector<mp_string> ipv6List;
    iRet = CIP::GetHostIPList(ipv4List, ipv6List);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Get host iplist failed.");
        return MP_FALSE;
    }

    // 检查ESN号对应的IP是否为该存储的IP号
    std::map<mp_string, mp_string>::iterator iter;
    for (iter = ipAndEsnMap.begin(); iter != ipAndEsnMap.end(); ++iter) {
        if (esn != iter->second) {
            continue;
        }
        if ((std::count(ipv4List.begin(), ipv4List.end(), iter->first) != 0) ||
            (std::count(ipv6List.begin(), ipv6List.end(), iter->first) != 0)) {
            INFOLOG("Get the map of local host IP %s and storage esn %s!", iter->first.c_str(),
                iter->second.c_str());
            return MP_TRUE;
        }
    }
    WARNLOG("The mapping relationship between the host and the storage(esn:%s) was not found!",
        esn.c_str());
    return MP_FALSE;
}

mp_bool Repository::CheckLun(const Json::Value &sanClient)
{
    const mp_string lunCheckKeys[] = {"sanclientWwpn", "lunName", "lunId", "path", "agentWwpn"};
    if (!sanClient.isMember("luninfo") || !sanClient["luninfo"].isArray()) {
        ERRLOG("Param not have lun info, check failed.");
        return MP_FALSE;
    }
    for (const auto &lun : sanClient["luninfo"]) {
        if (!lun.isObject()) {
            ERRLOG("Lun info check failed with necessary field.");
            return MP_FALSE;
        }
        for (const mp_string &key : lunCheckKeys) {
            if (!lun.isMember(key) || !lun[key].isString()) {
                ERRLOG("Lun info check failed with necessary field.");
                return MP_FALSE;
            }
        }
    }
    return MP_TRUE;
}

mp_string Repository::GetMountLunInfo(const Json::Value &lunInfo, MountNasParam &param, const mp_string &resourceId)
{
    LOGGUARD("");
    DBGLOG("Lun info: %s", lunInfo.toStyledString().c_str());
    if (!lunInfo.isObject() || !lunInfo.isMember("sanClients") || !lunInfo["sanClients"].isArray()) {
        ERRLOG("Invalid lunInfo.");
        return "";
    }
    mp_string lunInfoString = resourceId;
    lunInfoString += CHAR_COLON;
    mp_bool hasLunInfo = false;
    for (const auto& sanClient: lunInfo["sanClients"]) {
        if (!sanClient.isObject() || !CheckLun(sanClient)) {
            ERRLOG("Sanclient has not luninfo, no need parse.");
            continue;
        }
        if (sanClient.isMember("iqns") && sanClient["iqns"].isArray() && !sanClient["iqns"].empty()) {
            param.protocolType = "iqns";
        } else {
            param.protocolType = "fc";
        }
        for (auto &lun : sanClient["luninfo"]) {
            mp_int32 repositoryTypeIndex = CMpString::SafeStoi(lun["lunName"].asString(), 0);
            mp_string fileSystemSize = std::to_string(lun["filesystemsize"].asInt64() - 1);
            mp_string unidirectionalAuthPwd = "";
            mp_string sanclientPort = "";
            mp_string sanclientWwpn = lun["sanclientWwpn"].asString();
            mp_string agentWwpn = lun["agentWwpn"].asString();
            mp_string jobType = lun["jobType"].asString();
            mp_string fileioName = lun["fileioName"].asString();
            if (param.protocolType == "iqns") {
                DecryptStr(lun["UnidirectionalAuthPwd"].asString(), unidirectionalAuthPwd);
                sanclientPort = lun["port"].asString();
            }
            if (STORAGE_TYPE_ARRAY[repositoryTypeIndex] == param.repositoryType) {
                mp_string wpnInfo = sanclientWwpn + CHAR_COMMA + unidirectionalAuthPwd +
                    CHAR_SLASH + lun["lunId"].asString() + CHAR_COMMA + param.repositoryType +
                    CHAR_COMMA + fileSystemSize + CHAR_COMMA + agentWwpn + CHAR_COMMA + fileioName +
                    CHAR_COMMA + jobType + CHAR_COMMA + sanclientPort;
                lunInfoString += wpnInfo + CHAR_SLASH + CHAR_SLASH;
            }
            ClearString(unidirectionalAuthPwd);
            ClearString(sanclientWwpn);
            ClearString(agentWwpn);
        }
        if (!hasLunInfo) {
            hasLunInfo = true;
        }
    }
    return hasLunInfo ? lunInfoString : "";
}
}