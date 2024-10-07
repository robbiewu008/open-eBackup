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
#include "Utils.h"
#include <memory>
#include <string>
#include <atomic>
#include <regex>
#include <common/JsonHelper.h>
#include <common/Constants.h>
#include <common/Structs.h>
#include <repository_handlers/RepositoryHandler.h>
#include "log/Log.h"
#include "system/System.hpp"
#include "config_reader/ConfigIniReaderImpl.h"
#ifndef WIN32
#include "protect_engines/hcs/utils/HcsOpServiceUtils.h"
#include "curl_http/CurlHttpClient.h"
#include "curl_http/HttpClientInterface.h"
#else
#include "securec.h"
#include <repository_handlers/win32filesystem/Win32FileSystemHandler.h>
#endif

#ifndef WIN32
using namespace HcsPlugin;
#endif

const int32_t MILSECOND = 1000;
namespace VirtPlugin {
namespace Utils {
const std::string INTERNAL_BACKUP_SCENCE = "1";
 
int SaveToFile(const std::shared_ptr<RepositoryHandler> &repoHandler,
    const std::string &filename, const std::string &content)
{
    LOGGUARD("");
    if (repoHandler == nullptr) {
        ERRLOG("repoHandler is null.");
        return FAILED;
    }

    if (repoHandler->Open(filename, "w+") != SUCCESS) {
        ERRLOG("Failed to open file for writing, filename[%s]", filename.c_str());
        return FAILED;
    }

    Defer _(nullptr, [&, repoHandler](...) {
        if (repoHandler != nullptr) {
            repoHandler->Close();
        }
    });

    size_t ret = repoHandler->Write(content);
    if (ret != content.length()) {
        ERRLOG("Failed to write content to file.");
        return FAILED;
    }

    return SUCCESS;
}

int SaveToFileWithRetry(const std::shared_ptr<RepositoryHandler> &repoHandler, const std::string &filename,
    const std::string &content)
{
    int retryTimes = 3;
    int sleepSeconds = 3;
    while (retryTimes > 0) {
        if (SaveToFile(repoHandler, filename, content) == SUCCESS) {
            return SUCCESS;
        }
        SleepSeconds(sleepSeconds);
        retryTimes--;
    }
    return FAILED;
}

int ReadFile(const std::shared_ptr<RepositoryHandler> &repoHandler,
    const std::string &filename, std::string &content)
{
    if (!repoHandler->Exists(filename)) {
        ERRLOG("Not exist file, filename[%s]", filename.c_str());
        return SUCCESS;
    }
    if (repoHandler->Open(filename, "r") != SUCCESS) {
        ERRLOG("Open file filed. File: %s", filename.c_str());
        return FAILED;
    }
    Defer _(nullptr, [&, repoHandler](...) {
        if (repoHandler != nullptr) {
            repoHandler->Close();
        }
    });
    size_t fileSize = repoHandler->FileSize(filename);
    if (fileSize <= 0) {
        ERRLOG("Get file size failed. File: %s", filename.c_str());
        return FAILED;
    }
    if (repoHandler->Seek(0) != 0) {
        ERRLOG("Seek to offset 0 failed. File: %s", filename.c_str());
        return FAILED;
    }
    size_t readcount = repoHandler->Read(content, fileSize);
    if (readcount != fileSize) {
        ERRLOG("Read file failed. File: %s, size return: %zu, read expect: %zu", filename.c_str(), readcount, fileSize);
        return FAILED;
    }
    return SUCCESS;
}

int GetRandom(uint64_t &num)
{
    int fd = open("/dev/random", O_RDONLY);
    if (fd == -1) {
        ERRLOG("Open /dev/random failed[%d]:%s.", errno, strerror(errno));
        return FAILED;
    }
    if (read(fd, &num, sizeof(uint64_t)) == -1) {
        close(fd);
        ERRLOG("Read /dev/random failed, errno[%d]:%s.", errno, strerror(errno));
        return FAILED;
    }
    close(fd);
    return SUCCESS;
}

int32_t GetAgentDeployScence(std::string &deployScence)
{
    DBGLOG("Start to get agent deploy scence.");

    tinyxml2::XMLDocument agentConf;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string agentConfPath = agentHomedir + AGENT_CONF_PATH;
    if (agentConf.LoadFile(agentConfPath.c_str()) != SUCCESS) {
        ERRLOG("Failed to load agent conf file.");
        return FAILED;
    }
    tinyxml2::XMLElement *root = agentConf.RootElement();
    if (root == nullptr) {
        ERRLOG("Failed to get agentconf root.");
        return FAILED;
    }
    tinyxml2::XMLElement *backUpElement = root->FirstChildElement("Backup");
    if (backUpElement == nullptr) {
        ERRLOG("Failed to get backup element.");
        return FAILED;
    }
    tinyxml2::XMLElement *backUpScence = backUpElement->FirstChildElement("backup_scene");
    if (backUpScence == nullptr) {
        ERRLOG("Failed to get backup agent deploy mode.");
        return FAILED;
    }
    const char *scence = backUpScence->Attribute("value");
    std::string agentDeployScence(scence);
    deployScence = agentDeployScence;
    DBGLOG("The backup agent is deployed in %s scence.", deployScence.c_str());
    return SUCCESS;
}

int32_t GetDeployType(std::string& result)
{
    return GetAgentConfig(result, "System", "deploy_type");
}

int32_t GetAgentConfig(std::string& result, const std::string& section, const std::string& child)
{
    tinyxml2::XMLDocument agentConf;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string agentConfPath = agentHomedir + AGENT_CONF_PATH;
    if (agentConf.LoadFile(agentConfPath.c_str()) != SUCCESS) {
        ERRLOG("Failed to load agent conf file.");
        return FAILED;
    }
    tinyxml2::XMLElement *root = agentConf.RootElement();
    if (root == nullptr) {
        ERRLOG("Failed to get agentconf root.");
        return FAILED;
    }
    tinyxml2::XMLElement *backUpElement = root->FirstChildElement(section.c_str());
    if (backUpElement == nullptr) {
        ERRLOG("Failed to get section(%s).", section.c_str());
        return FAILED;
    }
    tinyxml2::XMLElement *backUpScence = backUpElement->FirstChildElement(child.c_str());
    if (backUpScence == nullptr) {
        ERRLOG("Failed to get child(%s).", child.c_str());
        return FAILED;
    }
    const char *value = backUpScence->Attribute("value");
    result = value;
    return SUCCESS;
}
 
bool IsInnerAgentscence()
{
    std::string deployScence;
    int32_t ret = Utils::GetAgentDeployScence(deployScence);
    if (ret != SUCCESS) {
        WARNLOG("Failed to get the agent deploy scence.");
        return false;
    }
    if (deployScence != INTERNAL_BACKUP_SCENCE) {
        return false;
    }
    return true;
}
 
std::string GetInnerAgentLocalIp()
{
    std::string localIp;
    if (GetAgentConfig(localIp, "System", "agent_ip") != SUCCESS) {
        return "";
    }
    return localIp;
}
 
void InnerAgentAppointNetDevice(Module::HttpRequest& request)
{
#ifndef WIN32
    if (HcsOpServiceUtils::GetInstance()->GetIsOpServiceEnv()) {
        return;
    }
#endif
    static bool initFlag = false;
    static std::string localIp;
    if (initFlag) {
        request.specialNetworkCard = localIp;
    }
 
    initFlag = true;
    if (!IsInnerAgentscence()) {
        return;
    }
    localIp = GetInnerAgentLocalIp();
    request.specialNetworkCard = localIp;
}
 
bool GetStoragesInfoFromExtendInfo(const std::string &authExtendInfo, std::vector<StorageInfo> &storageVector)
{
    Storages storages;
    if (!Module::JsonHelper::JsonStringToStruct(authExtendInfo, storages)) {
        ERRLOG("Failed to get storages");
        return false;
    }
    Json::Value storageValue;
    if (!Module::JsonHelper::JsonStringToJsonValue(storages.m_storages, storageValue)) {
        ERRLOG("Failed to get storage value");
        return false;
    }
    for (const auto &item : storageValue) {
        StorageInfo storageInfo;
        Module::JsonHelper::JsonStringToStruct(item.toStyledString(), storageInfo);
        storageVector.push_back(storageInfo);
    }
    return true;
}

bool CheckVolIpInStorage(const VolInfo &volInfo, const StorageInfo &storageInfo)
{
    std::vector<std::string> ipList;
    GetIpVectorFromString(ipList, storageInfo.m_ip, storageInfo.m_ipList);
    for (std::string ip : ipList) {
        if (ip == volInfo.m_datastore.m_ip) {
            return true;
        }
    }
    return false;
}

void GetIpVectorFromString(std::vector<std::string> &ipList, const std::string &ip, const std::string &ipListString)
{
    if (ipListString.empty()) {
        ipList.push_back(ip);
        return;
    }
    boost::split(ipList, ipListString, boost::is_any_of(","));
    if (ipList.size() == 0) {
        ERRLOG("Get error ip info! Ip list: %s", ipListString.c_str());
        return;
    }
    return;
}

#ifndef WIN32
std::string GetProxyHostId(bool isOpenstack)
{
    static std::timed_mutex configMutext;
    configMutext.lock();
    std::string hostId = DoGetProxyHostId(isOpenstack);
    configMutext.unlock();
    return hostId;
}

std::string DoGetProxyHostId(bool isOpenstack)
{
    DBGLOG("Enter.");
    std::string hostId = Module::ConfigReader::getString(GENERAL_CONF, DMI_DECODE_UUID);
    if (!hostId.empty()) {
        INFOLOG("Get host id %s from config success.", hostId.c_str());
        return hostId;
    }
    WARNLOG("Get host id from config failed, run cmd to get it.");
    std::vector<std::string> cmdOut;
    std::string agentHomedir = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    std::string functionName = isOpenstack ? "get_server_uuid" : "get_apasara_instance_uuid";
    std::vector<Module::CmdParam> cmdParam {
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::SCRIPT_CMD_NAME, agentHomedir + SUDO_DISK_TOOL_PATH),
        functionName
    };
    if (Module::RunCommand("sudo", cmdParam, cmdOut) != 0) {
        ERRLOG("Get uuid of proxy host failed.");
        return "";
    }
    if (cmdOut.size() != 1) {
        ERRLOG("Get host id size is %d.", cmdOut.size());
        return "";
    }
    hostId = isOpenstack ? cmdOut[0] : "i-" + cmdOut[0];
    SaveStringToVirtConfig(hostId, DMI_DECODE_UUID);
    return hostId;
}

void SaveStringToVirtConfig(const std::string &str, const std::string &item)
{
    std::shared_ptr<FileSystemHandler> tmpRepoHandler = std::make_shared<FileSystemHandler>();
    std::string configFilePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath() +
            VIRTUAL_CONF_PATH + VIRTUAL_CONF_NAME;
    if (!tmpRepoHandler->Exists(configFilePath)) {
        ERRLOG("Config file %s dont exists, save uuid failed.", configFilePath.c_str());
        return;
    }
    tmpRepoHandler->Open(configFilePath, "a+");
    std::string cmd = "s/" + item + "=/" + item + "=" + str + "/g";
    std::vector<std::string> cmdOut;
    std::unordered_set<std::string> pathWhiteList;
    std::vector<Module::CmdParam> cmdParam {
        Module::CmdParam(Module::COMMON_CMD_NAME, "sudo"),
        Module::CmdParam(Module::COMMON_CMD_NAME, "sed"),
        Module::CmdParam(Module::CMD_OPTION_PARAM, "-i"),
        Module::CmdParam(Module::COMMON_PARAM, cmd),
        Module::CmdParam(Module::PATH_PARAM, configFilePath)
    };
    int ret = Module::RunCommand("sudo", cmdParam, cmdOut, pathWhiteList);
    // 关闭文件时，数据落盘
    tmpRepoHandler->Close();
    // 从配置文件中刷新值
    Module::ConfigReaderImpl::instance()->refresh();
    INFOLOG("Save %s to %s, ret = %d", str.c_str(), item.c_str(), ret);
    return;
}
#endif

void ReportJobDetailWithLabel(const ReportJobDetailsParam &jobParam, const std::vector<std::string> &args,
    std::string jobId, std::string subJobId)
{
    SubJobDetails subJobDetails;
    std::vector<LogDetail> logDetailList;
    ActionResult result;
    LogDetail logDetail{};
    INFOLOG("Report job detail label: %s jobstatus: %d totalSize: %ld",
        jobParam.label.c_str(), jobParam.status, jobParam.dataSize);
    logDetail.__set_description(jobParam.label);
    logDetail.__set_level(jobParam.level);
    logDetail.__set_params(args);
    if (jobParam.errCode != 0) {
        logDetail.__set_errorCode(jobParam.errCode);
        logDetail.__set_errorParams(args);
        if (!jobParam.extendInfo.empty()) {
            logDetail.__set_additionalDesc(std::vector<std::string>({jobParam.extendInfo}));
        }
    }
    logDetail.__set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    ReportLog2AgentParam param = {subJobDetails, result, logDetailList, logDetail,
        jobParam.progress, jobParam.dataSize, jobParam.status};
    ReportLog2Agent(param, jobId, subJobId);
}

void ReportLog2Agent(ReportLog2AgentParam &param, std::string jobId, std::string subJobId)
{
    param.subJobDetails.__set_jobId(jobId);
    if (subJobId != "") {
        param.subJobDetails.__set_subJobId(subJobId);
    }
    if (param.logDetail.description != "") {
        DBGLOG("Current label is : %s.", WIPE_SENSITIVE(param.logDetail.description).c_str());
        param.logDetailList.push_back(param.logDetail);
    }
    param.subJobDetails.__set_jobStatus(param.curJobstatus);
    if (param.progress != 0) {
        param.subJobDetails.__set_progress(param.progress);
    }
    // 增量备份时，备份的数据可能为0，若不set datasize的话，Agent会判断datasize的isset为false，
    // 而上报给UBC datasize为-1，UBC会采取其他方式计算副本大小
    if (param.dataSize >= 0) {
        param.subJobDetails.__set_dataSize(param.dataSize);
    }
    param.subJobDetails.__set_logDetail(param.logDetailList);

    param.subJobDetails.__set_extendInfo(param.subJobDetails.extendInfo);

    DBGLOG("report job=[%s], subJobId=[%s], description=[%s], jobStatus=[%d].",
        param.subJobDetails.jobId.c_str(), param.subJobDetails.subJobId.c_str(),
        WIPE_SENSITIVE(param.logDetail.description).c_str(), param.curJobstatus);
    uint16_t retry = 0;
    do {
        JobService::ReportJobDetails(param.returnValue, param.subJobDetails);
    } while (param.curJobstatus == SubJobStatus::COMPLETED
        && param.returnValue.code != SUCCESS && retry++ < MAX_RETRY_CNT);
    param.logDetailList.clear();
    param.logDetail.__set_description("");
}

void RemoveSpecialSymbols(std::string &str)
{
    size_t pos = str.find('\n');
    if (pos != std::string::npos) {
        str.erase(pos, 1);
    }
}

void ConvertDiskToVolume(const Disk &disk, OpenStackPlugin::Volume &volume)
{
    volume.m_name = disk.m_name;
    volume.m_status = disk.m_status;
    volume.m_snapshotId = disk.m_sourceSnapshotId;
    volume.m_id = disk.m_id;
    volume.m_size = disk.m_size;
    volume.m_bootable = disk.m_type;
    volume.m_volumeType = disk.m_category;
}

int GetMetaRepoPath(std::string &metaRepoPath, std::shared_ptr<RepositoryHandler> &metaRepoHandler,
                    const AppProtect::BackupJob &job)
{
    for (const auto &repo: job.repositories) {
        if (repo.repositoryType != RepositoryDataType::META_REPOSITORY) {
            continue;
        }
#ifdef WIN32
        // 此处规避windows场景meta repo protocol为nas的特殊场景
        metaRepoHandler = std::make_shared<Win32FileSystemHandler>();
#else
        metaRepoHandler = RepositoryFactory::CreateRepositoryHandler(repo);
        if (metaRepoHandler == nullptr) {
            ERRLOG("Create repository handler failed.");
            return FAILED;
        }
#endif
        for (const auto &path: repo.path) {
            if (metaRepoHandler->Exists(path) && metaRepoHandler->IsDirectory(path)) {
                metaRepoPath = path;
                break;
            }
        }
    }
    if (metaRepoPath.empty()) {
        ERRLOG("Get meta repo path failed.");
        return FAILED;
    }
    return SUCCESS;
}

bool CheckIpv6Valid(const std::string &ip)
{
    std::regex ipv6Reg("(([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}{1}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}{1}|"
                       "((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})([.](25[0-5]|2[0-4][0-9]|"
                       "[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(:[0-9A-Fa-f]{1,4}"
                       "{1,2}|:((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})([.](25[0-5]|2[0-4]"
                       "[0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(:[0-9A-Fa-f]"
                       "{1,4}{1,3}|:((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})([.](25[0-5]|"
                       "2[0-4][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|:))|(([0-9A-Fa-f]{1,4}:){3}"
                       "(:[0-9A-Fa-f]{1,4}{1,4}|:((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})"
                       "([.](25[0-5]|2[0-4][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|:))|(([0-9A-Fa-f]"
                       "{1,4}:){2}(:[0-9A-Fa-f]{1,4}{1,5}|:((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]"
                       "{1,2})([.](25[0-5]|2[0-4][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|:))|"
                       "(([0-9A-Fa-f]{1,4}:){1}(:[0-9A-Fa-f]{1,4}{1,6}|:((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9]"
                       "[0-9]|([0-9])]{1,2})([.](25[0-5]|2[0-4][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|"
                       ":))|(:(:[0-9A-Fa-f]{1,4}{1,7}|:((22[0-3]丨2[0-1][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]"
                       "{1,2})([.](25[0-5]|2[0-4][0-9]|[0-1][0-9][0-9]|0[1 -9][0-9]|([0-9])]{1,2})){3})|:))");
    return std::regex_match(ip, ipv6Reg);
}

int32_t SetJsonValueByKeyToJsonString(std::string& bodyStr, const std::string& key, const Json::Value& value)
{
    Json::Value jsonValue;
    if (!Module::JsonHelper::JsonStringToJsonValue(bodyStr, jsonValue)) {
        ERRLOG("Convert %s failed", WIPE_SENSITIVE(bodyStr).c_str());
        return FAILED;
    }
    jsonValue[key] = value;
    Json::FastWriter jsonWriter;
    bodyStr = jsonWriter.write(jsonValue);
    return SUCCESS;
}

int32_t GetStringValueByKeyFromJsonString(const std::string& bodyStr, const std::string& key, std::string& result)
{
    Json::Value jsonValue;
    if (!Module::JsonHelper::JsonStringToJsonValue(bodyStr, jsonValue)) {
        ERRLOG("Convert %s failed", WIPE_SENSITIVE(bodyStr).c_str());
        return FAILED;
    }
    if (!jsonValue.isMember(key) || !jsonValue[key].isString()) {
        ERRLOG("Extend %s content validate failed, key is: %s", WIPE_SENSITIVE(bodyStr).c_str(), key.c_str());
        return FAILED;
    }
    result = jsonValue[key].asString();
    DBGLOG("find value: %s with key: %s", WIPE_SENSITIVE(result).c_str(), key.c_str());
    return SUCCESS;
}

int32_t GetJsonValueByKeyFromJsonString(const std::string& bodyStr, const std::string& key, Json::Value& result)
{
    Json::Value jsonValue;
    if (!Module::JsonHelper::JsonStringToJsonValue(bodyStr, jsonValue)) {
        ERRLOG("Convert %s failed", WIPE_SENSITIVE(bodyStr).c_str());
        return FAILED;
    }
    if (!jsonValue.isMember(key) || !jsonValue[key].isObject()) {
        ERRLOG("Extend %s content validate failed, key is: %s", WIPE_SENSITIVE(bodyStr).c_str(), key.c_str());
        return FAILED;
    }
    result = jsonValue[key];
    DBGLOG("find value: %s with key: %s", WIPE_SENSITIVE(result.toStyledString()).c_str(), key.c_str());
    return SUCCESS;
}


int32_t GetBoolValueByKeyFromJsonString(const std::string& bodyStr, const std::string& key, bool& result)
{
    Json::Value jsonValue;
    if (!Module::JsonHelper::JsonStringToJsonValue(bodyStr, jsonValue)) {
        ERRLOG("Convert %s failed", WIPE_SENSITIVE(bodyStr).c_str());
        return FAILED;
    }
    if (!jsonValue.isMember(key) || !jsonValue[key].isBool()) {
        ERRLOG("Extend %s content validate failed, key is:", WIPE_SENSITIVE(bodyStr).c_str(), key.c_str());
        return FAILED;
    }
    result = jsonValue[key].asBool();
    DBGLOG("find value: %d with key: %s", result, key.c_str());
    return SUCCESS;
}

void InitAndRegTracePoint()
{
#ifdef EBK_TRACE_POINT
    // 注册TP点
    (void)EbkTracePoint::GetInstance().RegTP("TP_AllowBackupInLocalNode", "AllowBackupInLocalNode return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_CreateSnapGroup", "CreateConsistencySnapshot return failed.",
        TP_TYPE_CALLBACK, BoolReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP__CreateSnap", "CreateSnap return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_GetMachineMetaData", "GetMachineMetaData return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_GetVolumeMetaData", "GetVolumeMetaData return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_AttachVolume", "AttachVolume return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_OpenDisk", "OpenDisk return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_ReadBlocks", "ReadBlocks return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_WriteBlocks", "WriteBlocks return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_DeleteSnapShot", "DeleteSnapShot return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_CreateVolume", "CreateVolume return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_RestoreAttachVolume", "AttachVolume return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_RestoreDetachVolume", "DetachVolume return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_DeleteBitmapFaield", "Delete bitmap return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_CreateMachineFailed", "Create machine return failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
#endif
    InitAndRegTracePointForHcs();
}

void InitAndRegTracePointForHcs()
{
#ifdef EBK_TRACE_POINT
    // 注册ECS相关的TP点
    (void)EbkTracePoint::GetInstance().RegTP("TP_FusionStorageOpenForBakFailed", "FusionStorage open fo bak failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_FusionStorageReadFailed", "FusionStorage read failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_FusionStorageWriteFailed", "FusionStorage write failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_HCS_DetachVolumeFailed", "HCS DetachVolume failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_HCS_AttachVolumeFailed", "HCS AttachVolume failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_HCS_AllowBackupInLocalNodeFailed",
        "HCS AllowBackupInLocalNode failed.", TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_HCS_GetMachineMetadataFailed", "HCS GetMachineMetadata failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_HCS_CreateVolumeSnapshotFailed", "HCS _CreateVolumeSnapshot failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_HCS_CheckBeforeBackupFailed", "HCS CheckBeforeBackup failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_OceanStorageOpenForBakFailed", "OceanStorage open for bak failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_OceanStorageReadFailed", "OceanStorage read failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_OceanStorageWriteFailed", "OceanStorage write failed.",
        TP_TYPE_CALLBACK, IntReturnFailHook);
    (void)EbkTracePoint::GetInstance().RegTP("TP_OpenStackDeleteVolumeSnapshotFailed",
        "OpenStack DeleteVolumeSnapshot failed.", TP_TYPE_CALLBACK, IntReturnFailHook);
#endif
}

int32_t GetUtf8CharNumber(const std::string &str)
{
    if (str.size() == 0) {
        return 0;
    }
    int idx = 0;
    int cnt = 0;
    while (str[idx]) {
        // 统计所有二进制最高两位非10的字节数量
        if ((str[idx] & 0xc0) != 0x80) {
            cnt++;
        }
        idx++;
    }
    return cnt;
}
} // end namespace Utils
} // end namespace VirtPlugin
