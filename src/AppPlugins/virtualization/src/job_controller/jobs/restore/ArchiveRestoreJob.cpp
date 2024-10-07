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
#ifndef VIRTUAL_ARCHIVE_RESTORE_JOB_H
#define VIRTUAL_ARCHIVE_RESTORE_JOB_H
#include "ArchiveRestoreJob.h"
#include "curl_http/CodeConvert.h"
#include "common/Structs.h"
#ifndef WIN32
#include "common/openstorage_api_client/api/OpenStorageApiClient.h"
#include "security/cmd/CmdParam.h"
#include "security/cmd/CmdExecutor.h"
#endif
#include "job_controller/factory/VirtualizationJobFactory.h"

namespace {
const std::string MODULE = "ARCHIVE_RESTORE";
const std::string VIRT_PLUGIN_VOL_CLOUDPATH_MATCH_INFO =
    VirtPlugin::VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "cloudpath_match.info";
using Defer = std::shared_ptr<void>;
constexpr int IO_TIME_OUT = 5; // ms

const int PREPARE_ARCHIVE_CLIENT_INTERVAL = 10;
const int GET_FILELIST_INTERVAL = 10;
const int COPY_FILE_RETRY_TIMES = 3;
const int COPY_FILE_RETRY_INTERVAL = 3;

const int SKD_PREPARE_CODE_CUR_SUCESS = 0;
const int SKD_PREPARE_CODE_LISTING = 1;
const int SKD_PREPARE_CODE_ALL_COMPLITE = 2;
const int SKD_PREPARE_CODE_FAILED = 3;
const int SKD_QUERY_CODE_FAILED = -1;
constexpr uint32_t GB_SIZE = 1024 * 1024 * 1024;
constexpr uint32_t SIZE_4M = 4 * 1024 * 1024;
const int GET_FILE_LIST_MAX_COUNT = 360;
const long long READ_FILELIST_NUM = 10000;
const int CONTROL_FILE_OFFSET_1 = 1;
const int CONTROL_FILE_OFFSET_2 = 2;
const int CONTROL_FILE_OFFSET_3 = 3;
const int CONTROL_FILE_OFFSET_4 = 4;
const int CONTROL_FILE_OFFSET_5 = 5;
const int CONTROL_FILE_OFFSET_6 = 6;
const int BASE64_SIZE_NUM = 2;
const std::string RAW_SUFFIX = ".raw";
const std::string DISTRIBUTE_DEPLOY_TYPE = "d7";
const std::string EXTERNAL_DEPLOY_SCENE = "0";
}

namespace VirtPlugin {
EXTER_ATTACK int ArchiveRestoreJob::PrerequisiteJob()
{
    m_isArchiveRestore = true;
    int ret = PrerequisiteArchiveRestoreJobInner();
    ReportJobResult(ret, "PrerequisiteJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}
int ArchiveRestoreJob::PrerequisiteArchiveRestoreJobInner()
{
    DBGLOG("Begin to exeute archive restore requisite job");
    if (InitJobInfo() != SUCCESS) {
        ERRLOG("InitJobInfo fail, %s", m_jobId.c_str());
        return FAILED;
    }
    if (InitArchiveAndGetMeta() != SUCCESS) {
        ERRLOG("InitArchiveAndGetMeta fail, %s", m_jobId.c_str());
        return FAILED;
    }
    PreInitStateHandles();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_PRE_CHECK_BEFORE_RECOVER);
    return RunStateMachine();
}
int ArchiveRestoreJob::InitArchiveAndGetMeta()
{
    DBGLOG("Begin to exeute archive restore requisite job, %s", m_jobId.c_str());
    if (!InitArchiveClient()) {
        ERRLOG("InitArchiveClient failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (!PrepareS3Client()) {
        ERRLOG("PrepareS3Client failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (GetCloudPathList() != SUCCESS) {
        ERRLOG("Failed to GetCloudPathList, %s", m_jobId.c_str());
        return FAILED;
    }
    if (!LoadArchiveMetaData()) {
        ERRLOG("Load meta data failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (!CloseArchiveClient()) {
        ERRLOG("CloseArchiveClient failed, %s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("InitArchiveAndGetMeta SUCCESS, %s", m_jobId.c_str());
    return SUCCESS;
}


/**
 * @brief 获取云上副本文件列表
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
int ArchiveRestoreJob::GetCloudPathList()
{
    DBGLOG("GetCloudPathList Begin, %s", m_jobId.c_str());
    m_cloudFileMatchPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_CLOUDPATH_MATCH_INFO;
    if (!GetFileListFromS3()) {
        ERRLOG("GetFileListFromS3 failed, %s", m_jobId.c_str());
        return FAILED;
    }
    std::string filePathPairstr = "";
    if (!Module::JsonHelper::StructToJsonString(m_filePathPairInfo, filePathPairstr)) {
        ERRLOG("Convert filePathPair failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (Utils::SaveToFile(m_cacheRepoHandler, m_cloudFileMatchPath, filePathPairstr) != SUCCESS) {
        ERRLOG("Failed to save filePathPair info, %s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("GetCloudPathList SUCCESS, %s", m_jobId.c_str());
    return SUCCESS;
}
/**
 * @brief 获取虚拟机元数据信息
 * @param vmMetaData
 * @return true: 成功, false: 失败
 */
bool ArchiveRestoreJob::LoadArchiveMetaData()
{
    std::string vmmetaDataPath = m_cacheRepoPath + VIRT_PLUGIN_VM_INFO;
    if (Utils::LoadFileToStruct(m_cacheRepoHandler, vmmetaDataPath, m_copyVm) != SUCCESS) {
        ERRLOG("Failed to read meta file %s", vmmetaDataPath.c_str());
        return false;
    }
    DBGLOG("Read  vol file %d", m_restorePara->restoreSubObjects.size());
    if (m_restorePara->restoreSubObjects.size() == 0) {
        ERRLOG("No volume req from pm, %s", m_jobId.c_str());
        return false;
    }
    for (const auto &vol : m_restorePara->restoreSubObjects) {
        std::string volmetaDataPath = m_cacheRepoPath + VIRT_PLUGIN_VOLUMES_META_DIR + vol.id + ".ovf";
        DBGLOG("read  vol file %s", volmetaDataPath.c_str());
        std::string volMetaData = "";
        if (Utils::ReadFile(m_cacheRepoHandler, volmetaDataPath, volMetaData) != SUCCESS) {
            ERRLOG("Failed to read file %s", volmetaDataPath.c_str());
            return false;
        }
        VolInfo volInfo;
        if (!Module::JsonHelper::JsonStringToStruct(volMetaData, volInfo)) {
            ERRLOG("Failed to convert volMetaData to Struct %s", m_jobId.c_str());
            return false;
        }
        m_backupedVolList.push_back(volInfo);
    }
    m_copyVm.m_volList = m_backupedVolList;
    return true;
}

/**
 * @brief 初始化任务参数
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
int ArchiveRestoreJob::InitJobInfo()
{
    if (!CommonInfoInit()) {
        ERRLOG("CommonInfoInit failed, %s", m_jobId.c_str());
        return FAILED;
    }
    if (!InitS3Info()) {
        ERRLOG("Init s3 Info failed, %s", m_jobId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

#ifndef WIN32
bool ArchiveRestoreJob::AddArchiveIpRoutePolicy(const std::string &ip)
{
    std::string deployType;
    int32_t ret = Utils::GetDeployType(deployType);
    if (ret == SUCCESS && deployType == DISTRIBUTE_DEPLOY_TYPE) {
        WARNLOG("Deployed in distribute storage, no need to add route policy.");
        return true;
    }
    std::string deployScence;
    ret = Utils::GetAgentDeployScence(deployScence);
    if (ret == SUCCESS && deployScence == EXTERNAL_DEPLOY_SCENE) {
        WARNLOG("Deployed in external, no need to add route policy.");
        return true;
    }
    UpdateIpRoutePolicyRequest req;
    req.SetTaskType("backup");
    req.SetDestIp(ip);
    OpenStorageApiClient apiClient;
    std::shared_ptr<UpdateIpRoutePolicyResponse> response = apiClient.AddIpPolicy(req);
    if (response == nullptr) {
        ERRLOG("AddIpPolicy failed, null response handler returned.");
        return false;
    }
    int64_t errCode = response->GetErrorCode();
    if (errCode != Module::SUCCESS) {
        ERRLOG("AddIpPolicy failed, error code: %ld.", errCode);
        return false;
    }
    INFOLOG("AddIscsiLogicIpRoutePolicy(%s) success.", ip.c_str());
    return true;
}
#endif
/**
 * @brief 初始化SDK，连接Archive服务
 * @param
 * @return true: 成功, false: 失败
 */

bool ArchiveRestoreJob::InitArchiveClient()
{
    if (m_restorePara->copies.empty()) {
        ERRLOG("No copy data, %s", m_jobId.c_str());
        return false;
    }
    m_copyId = m_restorePara->copies[0].id; // 副本ID
    m_jobId = m_restorePara->jobId;
    m_protectId = m_restorePara->copies[0].protectObject.id;
    INFOLOG("Enter InitArchiveClient jobid, %s, m_protectId: %s, m_copyId: %s", m_jobId.c_str(),
        m_protectId.c_str(), m_copyId.c_str());
    m_clientHandler = std::make_shared<ArchiveStreamService>();
    if (m_clientHandler == nullptr) {
        ERRLOG("Archive client ptr is null.");
        return false;
    }
    std::string objList = "";
    int ret = m_clientHandler->Init(m_copyId, m_jobId, objList);
    if (ret != MP_SUCCESS) {
        ERRLOG("Init archive client failed.");
        return false;
    }
    if (m_s3Info.serviceInfo.empty()) {
        ERRLOG("m_s3Info.serviceInfo is empty.");
        return false;
    }
    std::string ipList = "";
    int port = m_s3Info.serviceInfo[0].port;
    bool enableSSL = m_s3Info.enableSSL;

    for (std::size_t i = 0; i < m_s3Info.serviceInfo.size(); i++) {
        auto info = m_s3Info.serviceInfo[i];
        DBGLOG("Ip: %s port: %d ssl: %s", info.ip.c_str(), port, (m_s3Info.enableSSL ? "ssl on" : "ssl off"));
        ipList.append(info.ip);
#ifndef WIN32
        if (!AddArchiveIpRoutePolicy(info.ip)) {
            ERRLOG("AddArchiveIpRoutePolicy(%s) failed.", info.ip.c_str());
            return false;
        }
#endif
        if (i == m_s3Info.serviceInfo.size() -1) {
            break;
        }
        ipList.append(",");
    }
    DBGLOG("S3 iplist info: %s", WIPE_SENSITIVE(ipList).c_str());
    if (m_clientHandler->Connect(ipList, port, enableSSL) != MP_SUCCESS) {
        ERRLOG("Archive client Connect false.");
        return false;
    }
    DBGLOG("Archive client connect success.");
    isInitArchiveClient = true;
    return true;
}
/**
 * @brief 断开Archive连接
 * @return true: 成功, false: 失败
 */
bool ArchiveRestoreJob::CloseArchiveClient() const
{
    DBGLOG("Enter CloseArchiveClient.");
    if (!isInitArchiveClient) {
        INFOLOG("Archive client not init connect.");
        return true;
    }
    int ret = m_clientHandler->Disconnect();
    if (ret != MP_SUCCESS) {
        ERRLOG("Archive client disconnect failed");
        return false;
    }
    return true;
}

bool ArchiveRestoreJob::InitS3Info()
{
    // 归档到云s3信息
    std::string s3Info = "";
    for (const auto& copy : m_restorePara->copies) {
        for (const auto& repo : copy.repositories) {
            if (repo.protocol == RepositoryProtocolType::type::S3) {
                s3Info = repo.extendInfo;
            }
        }
    }
    if (s3Info.empty()) {
        ERRLOG("Init s3 Info is none");
        return false;
    }
    DBGLOG("S3Info: %s", WIPE_SENSITIVE(s3Info).c_str());
    if (!Module::JsonHelper::JsonStringToStruct(s3Info, m_s3Info)) {
        ERRLOG("Init s3 Info to struct failed.");
        return false;
    }
    if (m_s3Info.serviceInfo.empty()) {
        ERRLOG("Init s3 Info service info is none.");
        return false;
    }
    return true;
}

/**
 * @brief Archive前置准备
 * @return true: 成功, false: 失败
 */
bool ArchiveRestoreJob::PrepareS3Client()
{
    DBGLOG("Enter PrepareS3Client jobId: %s", m_restorePara->jobId.c_str());
    int ret = m_clientHandler->PrepareRecovery(m_metaFileMountPath);
    if (ret != MP_SUCCESS) {
        ERRLOG("PrepareRecovery failed.");
        return false;
    }
    int preState = 0;
    while (preState == 0) {
        if (m_clientHandler->QueryPrepareStatus(preState) != MP_SUCCESS) {
            ERRLOG("QueryPrepareStatus failed.");
            return false;
        }
        Utils::SleepSeconds(PREPARE_ARCHIVE_CLIENT_INTERVAL);
        DBGLOG("QueryPrepareStatus in progress ...");
    }
    DBGLOG("preState is: %d", preState);
    if (preState == SKD_QUERY_CODE_FAILED) {
        ERRLOG("QueryPrepareStatus state is failed.");
        return false;
    }
    DBGLOG("QueryPrepareStatus finish success jobId: %s", m_jobId.c_str());
    if (!m_cacheRepoHandler->Exists(m_metaFileMountPath.c_str())) {
        ERRLOG("Not exist metaFileMountPath:  %s", WIPE_SENSITIVE(m_metaFileMountPath).c_str());
        return false;
    }
    DBGLOG("Meta file path is: %s", WIPE_SENSITIVE(m_metaFileMountPath).c_str());
    // 拷贝所有meta文件到cache仓
    if (!CopyFileToCacheRepo(m_metaFileMountPath)) {
        ERRLOG("Copy File To Cache Repo fail: %s", WIPE_SENSITIVE(m_metaFileMountPath).c_str());
        return false;
    }
    return true;
}
/**
 * @brief 获取云上文件列表
 * @param
 * @return true: 成功, false: 失败
 */
bool ArchiveRestoreJob::GetFileListFromS3()
{
    DBGLOG("Enter GetFileListFromS3 jobId: %s", m_jobId.c_str());
    int createdJobCount = 0;
    int getState = 1;
    std::string control = "";
    std::string checkpoint = "";
    long long outNum {0};
    while (getState != SKD_PREPARE_CODE_ALL_COMPLITE) { // 非全部取完
        int count = 0;
        while (getState == SKD_PREPARE_CODE_CUR_SUCESS || getState == SKD_PREPARE_CODE_LISTING) { // 循环调用
            if (count > GET_FILE_LIST_MAX_COUNT) {
                ERRLOG("GetRecoverObjectList timeout, checkpoint: %s", checkpoint.c_str());
                return false;
            }
            if (m_clientHandler->GetRecoverObjectList(
                READ_FILELIST_NUM, checkpoint, control, outNum, getState) != MP_SUCCESS) {
                ERRLOG("GetRecoverObjectList return failed");
                return false;
            }
            DBGLOG("Status getState is: %d checkpoint: %s control: %s outNum: %lld",
                getState, checkpoint.c_str(), control.c_str(), outNum);
            if (getState == 0) {
                break;
            }
            if (getState == SKD_PREPARE_CODE_FAILED) { // 失败
                ERRLOG("GetRecoverObjectList status failed, checkpoint: %s", checkpoint.c_str());
                return false;
            }
            Utils::SleepSeconds(GET_FILELIST_INTERVAL);
            count++;
        }
         // 本次已完成
        if (getState == SKD_PREPARE_CODE_ALL_COMPLITE || getState == SKD_PREPARE_CODE_CUR_SUCESS) {
            if (!ProcessControlFile(control)) {
                return false;
            }
        }
    }
    DBGLOG("Finish GetFileListFromS3 success jobId: %s", m_jobId.c_str());
    return true;
}
/**
 * @brief 解析SDK返回的列举文件
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
bool ArchiveRestoreJob::ProcessControlFile(const std::string& control)
{
    INFOLOG("Enter ProcessControlFile jobId: %s", m_restorePara->jobId.c_str());

    if (!m_cacheRepoHandler->Exists(control.c_str())) {
        ERRLOG("Not exist control: %s", WIPE_SENSITIVE(control).c_str());
        return false;
    }
    std::ifstream file(control);
    if (!file.is_open()) {
        ERRLOG("open file failed, file name: %s", WIPE_SENSITIVE(control).c_str());
        return false;
    }
    std::string line = "";
    while (!file.eof()) {
        std::vector<std::string> strs;
        std::getline(file, line);
        DBGLOG("GetFileListFromCtrl one line: %s", line.c_str());
        std::stringstream iss(line);
        std::string word = "";
        while (std::getline(iss, word, ',')) {
            strs.push_back(word);
        }
        if (strs.empty() || strs[CONTROL_FILE_OFFSET_2] == "/") {
            continue;
        }
        std::string fsId = strs[1];
        std::string srcName = ""; // 解码后源文件名
        std::string decName = ""; // 解码后归档副本上原始路径

        if (!HandleRestoreFileName(strs[CONTROL_FILE_OFFSET_2], decName, srcName)) {
            ERRLOG("HandleRestoreFileName file failed name: %s", WIPE_SENSITIVE(decName).c_str());
            return false;
        }
        std::string mark = "/source_policy_" + m_protectId + "_Context_Global_MD";
        std::string metaFileName = "";
        if (decName.substr(0, mark.size()) == mark) {
            metaFileName = decName.substr(mark.size(), decName.length());
            DBGLOG("metaFileName: %s", metaFileName.c_str());

            if (RestoreBackupMetaData(decName, metaFileName, fsId) != SUCCESS) {
                ERRLOG("Restore file failed name: %s", WIPE_SENSITIVE(decName).c_str());
                return false;
            }
        }
        if (srcName.size() > 0) {
            m_filePathPairInfo.AddFilePathPair(srcName, decName, fsId);
        }
    }
    file.close();
    return true;
}

/**
 * @brief 恢复备份元数据
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
int ArchiveRestoreJob::RestoreBackupMetaData(const std::string& cloudFilePath,
    const std::string& fileName, const std::string& fsId)
{
    std::string cacheRepoFilePath = m_cacheRepoPath + fileName;
    DBGLOG("m_cacheRepoHandler is %s.", cacheRepoFilePath);
    if (m_cacheRepoHandler == nullptr) {
        ERRLOG("m_cacheRepoHandler is null.");
        return FAILED;
    }
    if (fileName.back() == '/') {   // 目录
        if (!m_cacheRepoHandler->CreateDirectory(cacheRepoFilePath)) {
            ERRLOG("Failed to CreateDirectory, filename[%s]", fileName.c_str());
            return FAILED;
        }
    } else {
        std::string cacheRepoFilePathDir = cacheRepoFilePath.substr(0, cacheRepoFilePath.rfind('/') + 1);
        DBGLOG("cacheRepoFilePathDir is %s.", cacheRepoFilePathDir.c_str());
        if (!m_cacheRepoHandler->Exists(cacheRepoFilePathDir)) {
            if (!m_cacheRepoHandler->CreateDirectory(cacheRepoFilePathDir)) {
                ERRLOG("Failed to CreateDirectory, filename[%s]", cacheRepoFilePathDir.c_str());
                return FAILED;
            }
        }
        if (m_cacheRepoHandler->Open(cacheRepoFilePath, "a") != SUCCESS) { // 返回值
            ERRLOG("Failed to open file for writing, filename[%s]", cacheRepoFilePath.c_str());
            return FAILED;
        }
        if (RestoreBackupMetaFile(cloudFilePath, fileName, fsId) != SUCCESS) {
            ERRLOG("Failed to CreateDirectory, filename[%s]", fileName.c_str());
            m_cacheRepoHandler->Close();
            return FAILED;
        }
        m_cacheRepoHandler->Close();
    }
    return SUCCESS;
}

/**
 * @brief 恢复单个备份元数据文件
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
int ArchiveRestoreJob::RestoreBackupMetaFile(const std::string& cloudFilePath,
    const std::string& fileName, const std::string& fsId)
{
    int readEnd = 0;
    long long offset = 0;
    do {
        ArchiveStreamGetFileReq req {};
        req.taskID = m_jobId;
        req.archiveBackupId = m_copyId;
        req.fsID = fsId;
        req.filePath = cloudFilePath; // 文件归档的路径
        req.fileOffset = offset;
        req.readSize = ARCHIVESTREAM_READ_SIZE_4M ; // 0
        req.maxResponseSize = ARCHIVESTREAM_RESPONSE_SIZE_512K; // 6
        ArchiveStreamGetFileRsq retValue{};
        DBGLOG("req.taskID: %s, req.archiveBackupId: %s, req.fsID: %s, req.filePath:  %s",
            req.taskID.c_str(), req.archiveBackupId.c_str(), req.fsID.c_str(),
            req.filePath.c_str());
        if (m_clientHandler->GetFileData(req, retValue) != MP_SUCCESS) {
            ERRLOG("GetFileData failed: %s", fileName.c_str());
            return FAILED;
        }
        DBGLOG("retValue.fileSize: %d offset: %lld retValue.readEnd: %d", retValue.fileSize, offset, retValue.readEnd);
        readEnd = retValue.readEnd;
        size_t writeRet = m_cacheRepoHandler->Write(retValue.data);
        DBGLOG("writeRet.fileSize: %d", writeRet);
        if (writeRet == -1) {
            ERRLOG("write file failed: %s offset: %lld", fileName.c_str(), retValue.offset);
            free(retValue.data);
            retValue.data = nullptr;
            return FAILED;
        }
        free(retValue.data);
        retValue.data = nullptr;
        offset += retValue.fileSize;
        DBGLOG("retValue.fileSize: %d", retValue.fileSize);
    } while (readEnd != 1);
    INFOLOG("SUCCESS to RestoreBackupMetaFiles %s", fileName.c_str());
    return SUCCESS;
}

/**
 * @brief 转换云上文件名称
 * @param
 * @return true: 成功, false: 失败
 */
bool ArchiveRestoreJob::HandleRestoreFileName(const std::string& inFile,
    std::string& decName, std::string& srcName) const
{
#ifndef WIN32
    if (!Module::CodeConvert::DecodeBase64(inFile.size() * BASE64_SIZE_NUM, inFile, decName)) {
        ERRLOG("base64 decode file name failed: %s", inFile.c_str());
        return false;
    }
#else
    // ->:windows实现
#endif
    if (decName.size() > RAW_SUFFIX.size() &&
        decName.substr(decName.size() - RAW_SUFFIX.size(), RAW_SUFFIX.size()) == RAW_SUFFIX) {
        int pos = decName.find_last_of('/') + 1;
        srcName = decName.substr(pos, decName.size() - pos);
        DBGLOG("srcName: %s", srcName.c_str());
    }
    DBGLOG("decName: %s", decName.c_str());
    return true;
}

/**
 * @brief 拷贝文件至cache仓
 * @param
 * @return true: 成功, false: 失败
 */

bool ArchiveRestoreJob::CopyFileToCacheRepo(const std::string& file) const
{
#ifdef WIN32
    return true;
#else
    INFOLOG("copy file: %s", WIPE_SENSITIVE(file).c_str());
    INFOLOG("copy m_cacheRepoPath: %s", WIPE_SENSITIVE(m_cacheRepoPath).c_str());
   // Module::FileSystemIO fsIO {};
    if (!m_cacheRepoHandler->Exists(file.c_str())) {
        ERRLOG("no copy file: %s", WIPE_SENSITIVE(file).c_str());
        return false;
    }

    if (!m_cacheRepoHandler->Exists(m_cacheRepoPath.c_str())) {
        ERRLOG("no m_cacheRepoPath: %s", WIPE_SENSITIVE(m_cacheRepoPath).c_str());
        return false;
    }
    INFOLOG("copy file and cacheFsPath both exist");

    int retry = COPY_FILE_RETRY_TIMES;
    while (retry > 0) {
        std::vector<std::string> output;

        std::string cmdtest = "cp -ra " + file + " " + m_cacheRepoPath;
        WARNLOG("run cmdtest: %s", WIPE_SENSITIVE(cmdtest).c_str());
        std::unordered_set<std::string> pathWhitelist;
        pathWhitelist.insert(file);
        pathWhitelist.insert(m_cacheRepoPath);
        std::vector<Module::CmdParam> cmdParam {
            Module::CmdParam(Module::COMMON_CMD_NAME, "cp"),
            Module::CmdParam(Module::CMD_OPTION_PARAM, "-ra"),
            Module::CmdParam(Module::PATH_PARAM, file),
            Module::CmdParam(Module::PATH_PARAM, m_cacheRepoPath)
        };
        int ret = Module::RunCommand("cp", cmdParam, output, pathWhitelist);
        if (ret != 0) {
            std::string msg = "";
            for (auto &it : output) {
                msg += it + " ";
            }

            WARNLOG("run shell ret: %d", ret);
            WARNLOG("run shell msg: %s", WIPE_SENSITIVE(msg));
            WARNLOG("run shell copy fail retry ...");
        } else {
            INFOLOG("copy file to repo success file: %s", WIPE_SENSITIVE(file).c_str());
            return true;
        }
        retry--;
        Utils::SleepSeconds(COPY_FILE_RETRY_INTERVAL);
    }
    return true;
#endif
}
/**
 * @brief 执行子任务
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
EXTER_ATTACK int ArchiveRestoreJob::ExecuteSubJob()
{
    if (!CommonInfoInit()) {
        ERRLOG("Failed to Init common info,%s", m_jobId.c_str());
        return FAILED;
    }
    int ret = SUCCESS;
    if (m_subJobInfo->jobName == "PostSubJob") {
        PostSubJobStateInit();
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXEC_POST_SUBJOB_PREHOOK);
        ret = RunStateMachine();
    } else {
        m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_PREHOOK);
        ret = ArchiveExecuteSubJobInner();
        CloseArchiveClient();
        // 清理资源
        if ((SubTaskClean() != SUCCESS) || (ret != SUCCESS)) {
            ERRLOG("Failed to clean task, result:%d taskId:%s ", ret, m_jobId.c_str());
            ret = FAILED;
        }
    }
    ReportJobResult(ret, "ExecuteSubJob finish.", m_completedDataSize);
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}
/**
 * @brief 分解子任务
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
EXTER_ATTACK int ArchiveRestoreJob::GenerateSubJob()
{
    SetGenerateJobStateMachine();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_GEN_JOB_INIT);
    int ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return SUCCESS;
}
int ArchiveRestoreJob::ArchiveExecuteSubJobInner()
{
    DBGLOG("ArchiveExecuteSubJobInner Begin %s", m_jobId.c_str());
    if (ExecuteSubJobPreHook() != SUCCESS) {
        ERRLOG("Failed to ExecuteSubJobPreHook,%s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("ExecuteSubJobPreHook Success %s", m_jobId.c_str());
    if (ExcuteSubTaskInitialize() != SUCCESS) {
        ERRLOG("Failed to ExcuteSubTaskInitialize,%s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("ExcuteSubTaskInitialize Success %s", m_jobId.c_str());
    m_isArchiveRestore = true;
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_EXECJOB_LOAD_DIRTYRANGES);
    while (m_nextState == static_cast<int>(VirtPlugin::State::STATE_EXECJOB_LOAD_DIRTYRANGES)) {
        if (SubTaskGetDirtyRanges() != SUCCESS) {
            ERRLOG("Failed to SubTaskGetDirtyRanges,%s", m_jobId.c_str());
            return FAILED;
        }
        DBGLOG("SubTaskGetDirtyRanges Success %s", m_jobId.c_str());
        if (SubTaskExecute() != SUCCESS) {
            ERRLOG("Failed to SubTaskExecute,%s", m_jobId.c_str());
            return FAILED;
        }
    }
    DBGLOG("SubTaskExecute Success %s", m_jobId.c_str());
    if (ExecuteSubJobPostHook() != SUCCESS) {
        ERRLOG("Failed to ExecuteSubJobPostHook,%s", m_jobId.c_str());
        return FAILED;
    }
    DBGLOG("ArchiveExecuteSubJobInner SUCCESS %s", m_jobId.c_str());
    return SUCCESS;
}

/**
 *  @brief 执行子任务_初始化参数
 *
 *  @return SUCCESS 成功，FAILED 失败
 */
int ArchiveRestoreJob::ExcuteSubTaskInitialize()
{
    INFOLOG("Start sub task initialize,%s", m_jobId.c_str());
    // 加载成员变量参数
    if (InitExecJobParams() != SUCCESS) {
        ERRLOG("Failed to create init params,%s", m_jobId.c_str());
        return FAILED;
    }
    std::vector<std::string> reportArgs;
    reportArgs.push_back(m_originVolInfo.m_uuid.c_str());
    reportArgs.push_back(m_targetVolInfo.m_uuid.c_str());
    ReportTaskLabel(reportArgs);
    // 获取写端handler
    if (CreateWriteHandler() != SUCCESS) {
        ERRLOG("Failed to create IO handler,%s", m_jobId.c_str());
        return FAILED;
    }
    if (!InitS3Info()) {
        ERRLOG("Init s3 Info failed %s", m_jobId.c_str());
        return FAILED;
    }
    if (!InitArchiveClient()) {
        ERRLOG("InitArchiveClient failed %s", m_jobId.c_str());
        return FAILED;
    }
    if (InitArchiveReqPara() != SUCCESS) {
        ERRLOG("InitArchiveReqPara failed %s", m_jobId.c_str());
        return FAILED;
    }
    if (InitTaskScheduler() != SUCCESS) {
        ERRLOG("Failed to init task scheduler, %s", m_taskInfo.c_str());
        return FAILED;
    }
    return SUCCESS;
}


/**
 *  @brief 执行子任务_初始化参数
 *
 *  @return SUCCESS 成功，FAILED 失败
 */
int ArchiveRestoreJob::InitArchiveReqPara()
{
    INFOLOG("InitArchiveReqPara Begin %s", m_jobId.c_str());
    m_cloudFileMatchPath = m_cacheRepoPath + VIRT_PLUGIN_VOL_CLOUDPATH_MATCH_INFO;
    if (!m_cacheRepoHandler->Exists(m_cloudFileMatchPath)) { // 还没有创建卷匹配文件，表示还没有到卸载，不需要挂载上去
        ERRLOG("file is not exist m_cloudFileMatchPath %s", m_cloudFileMatchPath.c_str());
        return FAILED;
    }
    if (Utils::LoadFileToStruct(m_cacheRepoHandler, m_cloudFileMatchPath, m_filePathPairInfo) != SUCCESS) {
        ERRLOG("Load cloudpath_match.info to struct failed.");
        return FAILED;
    }
    std::string srcName = m_originVolInfo.m_uuid + ".raw";
    DBGLOG("m_originVolInfo name. %s", srcName.c_str());
    for (const auto& filePathPair:m_filePathPairInfo.filePathPairList) {
        if (filePathPair.srcName == srcName) {
            m_getFileReq.fsID = filePathPair.fsId;
            m_getFileReq.filePath = filePathPair.cloudFilePath;
            break;
        }
    }
    DBGLOG("cloudpath name. %s fsId %s", m_getFileReq.filePath.c_str(), m_getFileReq.fsID.c_str());
    m_getFileReq.taskID = m_jobId;
    m_getFileReq.archiveBackupId = m_copyId;
    m_getFileReq.maxResponseSize = ARCHIVESTREAM_RESPONSE_SIZE_512K; // 6
    return SUCCESS;
}

/**
 * @brief 后置子任务
 * @param
 * @return SUCCESS: 成功, FAILED: 失败
 */
EXTER_ATTACK int ArchiveRestoreJob::PostJob()
{
    int ret = ArchivePostJobInner();
    ReportJobResult(ret, "PostJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return SUCCESS;
}
int ArchiveRestoreJob::ArchivePostJobInner()
{
    if (!CommonInfoInit()) {
        ERRLOG("CommonInfoInit failed.");
        return FAILED;
    }

    if (!InitS3Info()) {
        ERRLOG("Init s3 Info failed.");
        return FAILED;
    }
    if (InitArchiveClient()) {
        if (!EndArchiveTask()) {
            ERRLOG("EndArchiveTask failed.");
            return MP_FAILED;
        }
    }
    PostJobStateInit();
    m_nextState = static_cast<int>(VirtPlugin::State::STATE_POST_PREHOOK);
    return RunStateMachine();
}
/**
 * @brief 通知Archive任务结束
 * @param
 * @return true: 成功, fasle: 失败
 */
bool ArchiveRestoreJob::EndArchiveTask()
{
    INFOLOG("Enter EndArchiveTask.");
    if (m_clientHandler->EndRecover() != SUCCESS) {
        ERRLOG("End whole archive s3 recovery EndRecover failed.");
        return false;
    }
    INFOLOG("End whole archive s3 recovery EndRecover success.");
    return true;
}

bool ArchiveRestoreJob::LoadVmMetaData()
{
    if (!LoadArchiveMetaData()) {
        ERRLOG("Load meta data failed, %s", m_jobId.c_str());
        return false;
    }
    return true;
}

}

#endif
