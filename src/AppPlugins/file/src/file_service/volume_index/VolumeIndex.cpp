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
#include "VolumeIndex.h"
#include <algorithm>
#include <unordered_map>
#include <cstring>
#include <boost/filesystem.hpp>
#include <sys/types.h>
#include <sys/stat.h>
#include "ClientInvoke.h"
#include "define/Types.h"
#include "ApplicationProtectPlugin_types.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "ApplicationProtectFramework_types.h"
#include "VolumeCopyMountProvider.h"
#include "JsonTransUtil.h"
#include "common/CleanMemPwd.h"
#include "system/System.hpp"
#include "PluginConstants.h"
#include "PluginUtilities.h"

#ifdef WIN32
#include "framework/inc/server/ApplicationServiceImp.h"
#include <iostream>
#include <cstdlib>
#include <windows.h>
#include <atlstr.h>
#endif

using namespace std;
using namespace AppProtect;
using namespace Module;
using namespace FilePlugin;
using namespace volumeprotect;
using namespace volumeprotect::mount;

namespace {
    const string MODULE = "VolumeIndex";
    const string RFI = "rfi";
    const char WIN_SEPARATOR = '\\';
    const char POSIX_SEPARATOR = '/';
    const int INDEX_REPORT_INTERVAL = 45;
    const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
    const string LIVEMOUNT_RECORD_JSON_NAME = "volumemount.record.json";
    constexpr uint8_t ERROR_POINT_MOUNTED = 201;
    const std::string DEFAULT_COPY_NAME = "volumeprotect";
#ifdef _WIN32
    const std::string VOLUMEINDEX_MOUNT_PATH_ROOT = R"(C:\databackup\volumeIndex)";
#else
    const std::string VOLUMEINDEX_MOUNT_PATH_ROOT = "/mnt/databackup/volumeIndex";
#endif
    const std::string SYS_BOOT_VOLUME = "boot";
    const std::string ALARM_CODE_INDEXING_FAILED_MOUNT_VOLUME = "0x640340000D";
}

std::shared_ptr<AppProtect::BuildIndexJob> VolumeIndex::GetJobInfoBody()
{
    return dynamic_pointer_cast<AppProtect::BuildIndexJob>(GetJobInfo()->GetJobInfo());
}

EXTER_ATTACK int VolumeIndex::PrerequisiteJob()
{
    int ret = PrerequisiteJobInner();
    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int VolumeIndex::GenerateSubJob()
{
    int ret = GenerateSubJobInner();
    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int VolumeIndex::ExecuteSubJob()
{
    int ret = ExecuteSubJobInner();
    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int VolumeIndex::PostJob()
{
    int ret = PostJobInner();
    SetJobToFinish();
    return ret;
}

int VolumeIndex::PrerequisiteJobInner() const
{
    return Module::SUCCESS;
}

int VolumeIndex::GenerateSubJobInner()
{
    m_indexPara = GetJobInfoBody();
    INFOLOG("Generate sub task for build index task, task id is %s", m_indexPara->jobId.c_str());
    SubJob subJob {};
    subJob.__set_jobId(m_indexPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("BuildIndex");
    subJob.__set_policy(ExecutePolicy::ANY_NODE);
    vector<SubJob> vec {};
    vec.push_back(subJob);

    ActionResult result {};
    JobService::AddNewJob(result, vec);
    SubJobDetails subJobDetails {};
    subJobDetails.__set_jobId(m_indexPara->jobId);
    subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    string description = "Generate sub task for build index task successfully";
    LogDetail logDetail {};
    vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(result, subJobDetails);
    SetJobToFinish();
    INFOLOG("Finish to generate sub job , main task id is %s", m_indexPara->jobId.c_str());
    return Module::SUCCESS;
}

int VolumeIndex::ExecuteSubJobInner()
{
    m_indexPara = GetJobInfoBody();
    // 识别cache repo, pre data repo, cur data repo
    int ret = IdentifyRepos();
    if (ret != Module::SUCCESS) {
        ERRLOG("Bad Identify Repos");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    if (IsAbortJob()) {
        ERRLOG("Job aborted, skip scanner.");
        return Module::FAILED;
    }

    ret = ProcessVolumeIndex();
    if (ret != Module::SUCCESS) {
        ReportJob(SubJobStatus::FAILED);
        ERRLOG("Bad set up scanner");
        return Module::FAILED;
    }
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}

int VolumeIndex::PostJobInner()
{
    return Module::SUCCESS;
}

void VolumeIndex::ProcessVolumeScan()
{
    return;
}

void VolumeIndex::FillScanConfigForScan()
{
    return;
}

bool VolumeIndex::CopyPreMetaFileToWorkDir() const
{
    return true;
}

// 识别cache repo , pre data repo , cur data repo
int VolumeIndex::IdentifyRepos()
{
    m_indexPara = GetJobInfoBody();
    for (unsigned int i = 0; i < m_indexPara->copies.size(); i++) {
        for (auto repo : m_indexPara->copies[i].repositories) {
            if (IdentifyRepo(repo) != Module::SUCCESS) {
                return Module::FAILED;
            }
        }
    }
    for (auto repo : m_indexPara->repositories) {
        if (IdentifyRepo(repo) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    m_indexType = m_preRepo != nullptr ? VolumeIndexType::VOLUME_INDEX_TYPE_INC :
        VolumeIndexType::VOLUME_INDEX_TYPE_FULL;
    if (m_indexType == VolumeIndexType::VOLUME_INDEX_TYPE_INC) {
        INFOLOG("index tyep is inc index");
    } else {
        INFOLOG("index tyep is full index");
    }
    
    if (m_curRepo == nullptr || m_cacheRepo == nullptr || m_indexRepo == nullptr) {
        ERRLOG("repo init not complete");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int VolumeIndex::IdentifyRepo(StorageRepository& repo)
{
    PrintRepo(repo);
    if (repo.repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
        StorageRepositoryExtendInfo extendInfo {};
        if (!JsonHelper::JsonStringToStruct(repo.extendInfo, extendInfo)) {
            ERRLOG("Data storage repository extend info is invaild.");
            return Module::FAILED;
        }
        if (extendInfo.isCurrentCopyRepo) {
            m_curRepo = std::make_shared<StorageRepository>(repo);
            m_curRepoExtendInfo = std::make_shared<StorageRepositoryExtendInfo>(extendInfo);
            INFOLOG("set cur repo complete");
        } else {
            m_preRepo = std::make_shared<StorageRepository>(repo);
            m_preRepoExtendInfo = std::make_shared<StorageRepositoryExtendInfo>(extendInfo);
            INFOLOG("set pre repo complete");
        }
    } else if (repo.repositoryType == RepositoryDataType::type::CACHE_REPOSITORY) {
        m_cacheRepo = std::make_shared<StorageRepository>(repo);
        INFOLOG("set cache repo complete");
    } else if (repo.repositoryType == RepositoryDataType::type::INDEX_REPOSITORY) {
        m_indexRepo = std::make_shared<StorageRepository>(repo);
        INFOLOG("set index repo complete");
    } else if (repo.repositoryType == RepositoryDataType::type::META_REPOSITORY) {
        if (SetupMetaPath(repo) != Module::SUCCESS) {
            return Module::FAILED;
        }
    } else {
        ERRLOG("Receive invalid repo type : %d", repo.repositoryType);
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int VolumeIndex::SetupMetaPath(const StorageRepository& repo)
{
    m_metaRepo = std::make_shared<StorageRepository>(repo);
    StorageRepositoryExtendInfo extendInfo {};
    if (!JsonHelper::JsonStringToStruct(repo.extendInfo, extendInfo)) {
        ERRLOG("Meta storage repository extend info is invalid.");
        return Module::FAILED;
    }
    if (extendInfo.isCurrentCopyRepo) {
        m_metaRepo = std::make_shared<StorageRepository>(repo);
        m_metaFsPath = m_metaRepo->path[0];
        INFOLOG("set cur meta repo complete!");
    } else {
        m_preMetaRepo = std::make_shared<StorageRepository>(repo);
        m_preMetaFsPath = m_preMetaRepo->path[0];
        INFOLOG("set pre meta repo complete!");
    }
    return Module::SUCCESS;
}

/**
 * Create folders,
 * | -- CacheFsPath
 *      | -- meta
 *          | -- previous
 *          | -- latest
 *      | -- ctrl
 *      | -- rfi
 */
int VolumeIndex::ProcessVolumeIndex()
{
    INFOLOG("Enter ProcessVolumeIndex");
    // cache/copyId/filemeta/meta/latest/{volumeName}/latest/dcache,fcache....
    m_cacheFsPath = PluginUtils::PathJoin(PluginUtils::GetPathName(m_cacheRepo->path[0]),
        m_curRepoExtendInfo->copyId, METAFILE_PARENT_DIR);
    if (m_indexType == VolumeIndexType::VOLUME_INDEX_TYPE_INC) {
        m_preCacheFsPath = PluginUtils::PathJoin(PluginUtils::GetPathName(m_cacheRepo->path[0]),
        m_preRepoExtendInfo->copyId, METAFILE_PARENT_DIR);
        INFOLOG("process inc index, preCopyId: %s, curCopyId: %s, jobId: %s, m_cacheFsPath: %s, m_preCacheFsPath: %s",
            m_preRepoExtendInfo->copyId.c_str(),
            m_curRepoExtendInfo->copyId.c_str(),
            m_indexPara->jobId.c_str(),
            m_cacheFsPath.c_str(),
            m_preCacheFsPath.c_str());
        return ProcessVolumeIncIndex();
    }
    INFOLOG("process full index, copyId %s, jobId %s, m_cacheFsPath: %s",
        m_curRepoExtendInfo->copyId.c_str(), m_indexPara->jobId.c_str(), m_cacheFsPath.c_str());
    string workDir = PluginUtils::PathJoin(m_cacheFsPath, META, LATEST);
    PluginUtils::CreateDirectory(workDir);
    // Full index
    ProcessVolumeScan();
    PluginUtils::CreateDirectory(PluginUtils::PathJoin(m_cacheFsPath, RFI));
    vector<string> dirList;
    PluginUtils::GetDirListInDirectory(workDir, dirList);
    for (int i = 0; i < dirList.size(); i++) {
        m_isLastScan = (i == (dirList.size() - 1));
        DBGLOG("generate full RFI in path %s", dirList[i].c_str());
        // cacheFs/meta/latest/${volumeName}
        string curDir = PluginUtils::PathJoin(workDir, dirList[i]);
        int ret = GenerateFullRfiInPath(curDir, false);
        if (ret != Module::SUCCESS) {
            ReportJob(SubJobStatus::FAILED);
            return Module::FAILED;
        }
    }
    return Module::SUCCESS;
}
 
void VolumeIndex::CleanIndexMounts()
{
    // 扫描任务完成后卸载卷和共享
    INFOLOG("Enter CleanIndexMounts, jobId %s", m_indexPara->jobId.c_str());
    LoadUmountRecords();
    UmountVolumesFromRecords();
    ForceUmountNasShare(m_nasShareMountTarget);
    std::string livemountTaskDirectory = PluginUtils::PathJoin(VOLUMEINDEX_MOUNT_PATH_ROOT, m_indexPara->jobId);
    INFOLOG("remove volume livemount task directory %s", livemountTaskDirectory.c_str());
    PluginUtils::Remove(livemountTaskDirectory);
    INFOLOG("Exit CleanIndexMounts, jobId %s", m_indexPara->jobId.c_str());
}

bool VolumeIndex::PrepareBasicDirectory()
{
    m_nasShareMountTarget = PluginUtils::PathJoin(VOLUMEINDEX_MOUNT_PATH_ROOT, m_indexPara->jobId, "share");
    INFOLOG("create volume livemount share mount target %s", m_nasShareMountTarget.c_str());
    m_volumesMountTargetRoot = PluginUtils::PathJoin(VOLUMEINDEX_MOUNT_PATH_ROOT, m_indexPara->jobId, "volumes");
    INFOLOG("create volume livemount volumes mount target root %s", m_volumesMountTargetRoot.c_str());
    m_volumesMountRecordRoot = PluginUtils::PathJoin(VOLUMEINDEX_MOUNT_PATH_ROOT, m_indexPara->jobId, "records");
    INFOLOG("create volume livemount volumes mount record root %s", m_volumesMountRecordRoot.c_str());
    if (!PluginUtils::CreateDirectory(m_nasShareMountTarget)
        || !PluginUtils::CreateDirectory(m_volumesMountTargetRoot)
        || !PluginUtils::CreateDirectory(m_volumesMountRecordRoot)) {
        ERRLOG("failed to create basic volume livemount directory");
        return false;
    }
    m_dataPathRoot = m_nasShareMountTarget;
    INFOLOG("using data path %s, cloneCopyId %s", m_dataPathRoot.c_str(), m_indexPara->jobId.c_str());
    return true;
}

bool VolumeIndex::MountNasShare()
{
    JobPermission permission {};
    permission.__set_user("0");
    permission.__set_group("0");

    AppProtect::StorageRepository dataRepo = *m_curRepo.get();
    dataRepo.__set_path(std::vector<std::string> { m_nasShareMountTarget });

    PrepareRepositoryByPlugin repos {};
    repos.__set_repository(std::vector<AppProtect::StorageRepository> { dataRepo });
    repos.__set_permission(permission);
    repos.__set_extendInfo(m_indexPara->extendInfo);

    ActionResult actionResult {};
    JobService::MountRepositoryByPlugin(actionResult, repos);
    if (actionResult.code != 0) {
        ERRLOG("Call MountRepositoryByPlugin failed! action code %u, message %s, jobId: %s",
            actionResult.code, actionResult.message.c_str(), m_indexPara->jobId.c_str());
        if (actionResult.code != ERROR_POINT_MOUNTED) {
            JobService::UnMountRepositoryByPlugin(actionResult, repos);
        }
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_host_livemount_execute_script_failed");
        return false;
    }
    return true;
}

bool VolumeIndex::MountVolumes()
{
    INFOLOG("get volume directories in %s", m_dataPathRoot.c_str());
    bool success = true;
    std::vector<std::string> volumeDirPaths;
    if (!PluginUtils::GetDirListInDirectory(m_dataPathRoot, volumeDirPaths, true)) {
        ERRLOG("failed to get volume directories in %s", m_dataPathRoot.c_str());
        return false;
    }
    for (const std::string& volumeDirName : volumeDirPaths) {
        std::string volumeName = volumeDirName;
        std::string volumeDirPath = PluginUtils::PathJoin(m_dataPathRoot, volumeDirName);
        if (volumeName == SYS_BOOT_VOLUME) {
            INFOLOG("skip mount system boot volume path %s", volumeDirPath.c_str());
            continue;
        }
        std::string volumeMountRecordJsonPath;
        std::string volumeMountTarget = PluginUtils::PathJoin(m_volumesMountTargetRoot, volumeName);
        INFOLOG("peform volume mount (%s) from data dir %s", volumeName.c_str(), volumeDirPath.c_str());
        if (!MountSingleVolumeReadOnly(
            volumeName,
            volumeDirPath,
            volumeMountTarget,
            PluginUtils::PathJoin(m_volumesMountRecordRoot, volumeName),
            volumeMountRecordJsonPath)) {
            ERRLOG("failed to mount volume (%s) from data dir %s", volumeName.c_str(), volumeDirPath.c_str());
            // send alarm
            ERRLOG("send ALARM_CODE_INDEXING_FAILED_MOUNT_VOLUME alarm type %s, subType %s, job %s",
                m_indexPara->indexProtectObject.type.c_str(),
                m_indexPara->indexProtectObject.subType.c_str(), m_indexPara->jobId.c_str());
            AppProtect::AlarmDetails alarm;
            ActionResult result;
            alarm.alarmId = ALARM_CODE_INDEXING_FAILED_MOUNT_VOLUME;
            alarm.parameter = m_indexPara->indexProtectObject.subType + "," + m_indexPara->jobId;
            JobService::SendAlarm(result, alarm);
            success = false;
        }
        m_mountedRecords.push_back(volumeMountRecordJsonPath);
        m_volumesMountPaths.push_back(volumeMountTarget);
    }
    return success;
}

void VolumeIndex::LoadUmountRecords()
{
    INFOLOG("load volume mount record directories in %s", m_volumesMountRecordRoot.c_str());
    std::vector<std::string> volumeMountRecordDirNameList;
    if (!PluginUtils::GetDirListInDirectory(m_volumesMountRecordRoot, volumeMountRecordDirNameList, true)) {
        WARNLOG("failed to get volume mount record directories in %s", m_volumesMountRecordRoot.c_str());
    }
    for (const std::string& volumeMountRecordDirName : volumeMountRecordDirNameList) {
        std::vector<std::string> jsonfileList;
        std::string volumeMountRecordDirPath = PluginUtils::PathJoin(
            m_volumesMountRecordRoot, volumeMountRecordDirName);
        INFOLOG("check json file in %s", volumeMountRecordDirPath.c_str());
        PluginUtils::GetFileListInDirectory(volumeMountRecordDirPath, jsonfileList);
        for (const std::string& jsonfile : jsonfileList) {
            if (jsonfile.find(MOUNT_RECORD_JSON_EXTENSION) != std::string::npos) {
                INFOLOG("detected umount record json file %s", jsonfile.c_str());
                m_volumeMountRecordJsonList.push_back(jsonfile);
            }
        }
    }
}

void VolumeIndex::UmountVolumesFromRecords()
{
    for (const std::string& mountRecordJsonPath : m_volumeMountRecordJsonList) {
        INFOLOG("using %s to umount", mountRecordJsonPath.c_str());
        VolumeMountRecordJsonCommon recordJsonCommon {};
        if (!JsonFileTool::ReadFromFile(mountRecordJsonPath, recordJsonCommon)) {
            ERRLOG("read record json common struct from file: %s failed!", mountRecordJsonPath.c_str());
            continue;
        }
        UmountVolumeFromRecord(mountRecordJsonPath);
        ReportJobLabel(
            JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_livemount_volume_umount_success",
            recordJsonCommon.mountTargetPath);
    }
}

void VolumeIndex::UmountVolumeFromRecord(const std::string& mountRecordJsonPath)
{
    std::unique_ptr<VolumeCopyUmountProvider> umountProvider
        = VolumeCopyUmountProvider::Build(mountRecordJsonPath);
    if (umountProvider == nullptr) {
        ERRLOG("failed to build umount provider, record path %s", mountRecordJsonPath.c_str());
        return;
    }
    if (!umountProvider->Umount()) {
        WARNLOG("volume umount failed with error %s, record path %s",
            umountProvider->GetError().c_str(), mountRecordJsonPath.c_str());
    }
}

int VolumeIndex::ProcessVolumeIncIndex()
{
    INFOLOG("Enter ProcessVolumeIncIndex");
    // cur没有dcache扫描cur
    PluginUtils::CreateDirectory(PluginUtils::PathJoin(m_cacheFsPath, META, PREVIOUS));
    if (!CopyPreMetaFileToWorkDir()) {
        ERRLOG("Copy pre meta file to work dir failed");
        ReportJob(SubJobStatus::FAILED);
    }
    m_isPreparing = true;
    thread prepareThread(&VolumeIndex::PrepareForVolumeIncIndex, this);
    time_t lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
    while (m_isPreparing) {
        Module::SleepFor(std::chrono::seconds(1));
        time_t currentTime = PluginUtils::GetCurrentTimeInSeconds();
        if ((currentTime - lastReportTime) <= INDEX_REPORT_INTERVAL) {
            continue;
        }
        lastReportTime = currentTime;
        ReportJob(SubJobStatus::RUNNING);
    }

    try {
        if (prepareThread.joinable()) {
            prepareThread.join();
        }
    } catch (const std::exception& e) {
        // 捕获标准库异常
        ERRLOG("Exception caught when thread join: %s", e.what());
        return Module::FAILED;
    } catch (...) {
        // 捕获其他任何异常
        ERRLOG("Unknown exception caught when thread join ");
        return Module::FAILED;
    }
    
    // generate rfi.
    PluginUtils::CreateDirectory(m_cacheFsPath + dir_sep + RFI);
    // cache/copyId/filemeta/meta/latest/{volumeName}/latest/dcache,fcache....
    string workDir = PluginUtils::PathJoin(m_cacheFsPath, META, LATEST);
    vector<string> curDirList;
    vector<string> prevDirList;
    PluginUtils::GetDirListInDirectory(workDir, curDirList);
    for (size_t i = 0; i < curDirList.size(); i++) {
        curDirList[i] = PluginUtils::PathJoin(workDir, curDirList[i]);
        INFOLOG("curDirList: %s", curDirList[i].c_str());
    }
    // cache/copyId/filemeta/meta/previous/{volumeName}/latest/dcache,fcache....
    workDir = PluginUtils::PathJoin(m_cacheFsPath, META, PREVIOUS);
    PluginUtils::GetDirListInDirectory(workDir, prevDirList);
    for (size_t i = 0; i < prevDirList.size(); i++) {
        prevDirList[i] = PluginUtils::PathJoin(workDir, prevDirList[i]);
        INFOLOG("prevDirList: %s", prevDirList[i].c_str());
    }
    // 先排序， 同名的做增量， 不同名的做全增、全减
    sort(curDirList.begin(), curDirList.end());
    sort(prevDirList.begin(), prevDirList.end());
    return GenerateRfiWithTwoQueues(curDirList, prevDirList);
}

void VolumeIndex::PrepareForVolumeIncIndex()
{
    ProcessVolumeScan();
    m_isPreparing = false;
}

int VolumeIndex::GenerateRfiWithTwoQueues(const std::vector<std::string>& curDirList,
    const std::vector<std::string>& prevDirList)
{
    DBGLOG("Enter GenerateRfiWithTwoQueues");
    std::queue<string> curQueue;
    std::queue<string> prevQueue;
    for (auto path : curDirList) {
        curQueue.push(path);
    }
    for (auto path : prevDirList) {
        prevQueue.push(path);
    }

    while (!curQueue.empty() && !prevQueue.empty()) {
        string curPath = curQueue.front();
        string prevPath = prevQueue.front();
        string curSnapshotName = curPath.substr(curPath.find_last_of(dir_sep.front()) + 1);
        string prevSnapshotName = prevPath.substr(prevPath.find_last_of(dir_sep.front()) + 1);
        INFOLOG("Compare snapshot name : %s, %s", curSnapshotName.c_str(), prevSnapshotName.c_str());
        if (curSnapshotName == prevSnapshotName) {
            curQueue.pop();
            prevQueue.pop();
            CheckIsLastScan(curQueue, prevQueue);
            int ret = GenerateIncRfiInPath(prevPath, curPath);
            if (ret != Module::SUCCESS) {
                ReportJob(SubJobStatus::FAILED);
                return Module::FAILED;
            }
        } else if (curSnapshotName > prevSnapshotName) {
            // 如果prev是空，应该走进这个分支， curQueue出队
            curQueue.pop();
            CheckIsLastScan(curQueue, prevQueue);
            int ret = GenerateFullRfiInPath(curPath, false);
            if (ret != Module::SUCCESS) {
                ReportJob(SubJobStatus::FAILED);
                return Module::FAILED;
            }
        } else if (curSnapshotName < prevSnapshotName) {
            // 如果cur是空，应该走进这个分支， prevQueue出队
            prevQueue.pop();
            CheckIsLastScan(curQueue, prevQueue);
            int ret = GenerateFullRfiInPath(prevPath, true);
            if (ret != Module::SUCCESS) {
                ReportJob(SubJobStatus::FAILED);
                return Module::FAILED;
            }
        }
    }
    int ret = GenerateRfiWithSingleQueue(curQueue, prevQueue);
    if (ret != Module::SUCCESS) {
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int VolumeIndex::GenerateRfiWithSingleQueue(std::queue<string>& curQueue, std::queue<string>& prevQueue)
{
    INFOLOG("Enter GenerateRfiWithSingleQueue: %d, %d", curQueue.size(), prevQueue.size());
    while (!prevQueue.empty()) {
        string prevPath = prevQueue.front();
        prevQueue.pop();
        CheckIsLastScan(curQueue, prevQueue);
        int ret = GenerateFullRfiInPath(prevPath, true);
        if (ret != Module::SUCCESS) {
            ReportJob(SubJobStatus::FAILED);
            return Module::FAILED;
        }
    }

    while (!curQueue.empty()) {
        string curPath = curQueue.front();
        curQueue.pop();
        CheckIsLastScan(curQueue, prevQueue);
        int ret = GenerateFullRfiInPath(curPath, false);
        if (ret != Module::SUCCESS) {
            ReportJob(SubJobStatus::FAILED);
            return Module::FAILED;
        }
    }
    return Module::SUCCESS;
}

void VolumeIndex::CheckIsLastScan(const std::queue<string>& curQueue, const std::queue<string>& prevQueue)
{
    if (curQueue.empty() && prevQueue.empty()) {
        m_isLastScan = true;
    }
}

int VolumeIndex::GenerateFullRfiInPath(const string& path, bool isPre)
{
    INFOLOG("Enter GenerateFullRfiInPath: %s, %d", path.c_str(), isPre);
    FillScanConfigForGenerateRfi("", path, m_isLastScan, isPre);
    if (!StartScanner()) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
        ERRLOG("Start scanner failed!");
        return Module::FAILED;
    }
    MonitorScanner();
    return Module::SUCCESS;
}

int VolumeIndex::GenerateIncRfiInPath(const string& prevPath, const string& curPath)
{
    INFOLOG("Enter GenerateIncRfiInPath: %s, %s", prevPath.c_str(), curPath.c_str());
    FillScanConfigForGenerateRfi(prevPath, curPath, m_isLastScan, false);
    if (!StartScanner()) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
        ERRLOG("Start scanner failed!");
        return Module::FAILED;
    }
    MonitorScanner();
    return Module::SUCCESS;
}

bool VolumeIndex::StartScanner()
{
    INFOLOG("Enter StartScanner");
    m_scanner = ScanMgr::CreateScanInst(m_scanConfig);
    if (!m_scanner || m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        ERRLOG("Start Scanner Instance failed!");
        return false;
    }
    INFOLOG("Leave Start Scanner!");
    return true;
}

void VolumeIndex::MonitorScanner()
{
    INFOLOG("Enter Monitor Scanner");
    SCANNER_TASK_STATUS scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_INPROGRESS;
    SubJobStatus::type jobStatus = SubJobStatus::FAILED;

    do {
        m_scanStatus = m_scanner->GetStatus();
        FillMonitorScannerVarDetails(scanTaskStatus, jobStatus);
        if (scanTaskStatus != SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_INPROGRESS) {
            INFOLOG("scan status: %d", static_cast<int>(scanTaskStatus));
            break;
        }

        if (IsAbortJob()) {
            INFOLOG("Scanner - Abort is invoked for jobId : %s, subjobId : %s", m_indexPara->jobId.c_str(),
                m_subJobInfo->subJobId.c_str());
            m_scanner->Abort();
            ReportJob(SubJobStatus::ABORTING);
            break;
        }
        ReportJob(SubJobStatus::RUNNING);
        Module::SleepFor(std::chrono::seconds(SLEEP_TEN_SECONDS));
    } while (true);
    if (m_scanner != nullptr) {
        m_scanner->Destroy();
        m_scanner.reset();
    }
}

void VolumeIndex::FillMonitorScannerVarDetails(SCANNER_TASK_STATUS& scanTaskStatus,
    SubJobStatus::type& jobStatus)
{
    if (m_scanStatus == SCANNER_STATUS::COMPLETED) {
        HCP_Log(INFO, MODULE) << "Scan completed" << HCPENDLOG;
        jobStatus = SubJobStatus::COMPLETED;
        scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_SUCCESS;
    } else if (m_scanStatus == SCANNER_STATUS::FAILED) {
        HCP_Log(ERR, MODULE) << "Scan failed" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_FAILED;
    } else if (m_scanStatus == SCANNER_STATUS::ABORT_IN_PROGRESS) {
        HCP_Log(ERR, MODULE) << "Scan abort in progress" << HCPENDLOG;
        jobStatus = SubJobStatus::ABORTING;
    } else if (m_scanStatus == SCANNER_STATUS::ABORTED) {
        HCP_Log(ERR, MODULE) << "Scan aborted" << HCPENDLOG;
        jobStatus = SubJobStatus::ABORTED;
        scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_ABORTED;
    } else if (m_scanStatus == SCANNER_STATUS::SCAN_READ_COMPLETED) {
        jobStatus = SubJobStatus::RUNNING;
    } else if (m_scanStatus == SCANNER_STATUS::CTRL_DIFF_IN_PROGRESS) {
        jobStatus = SubJobStatus::RUNNING;
    } else if (m_scanStatus == SCANNER_STATUS::SECONDARY_SERVER_NOT_REACHABLE) {
        HCP_Log(ERR, MODULE) << "Scan failed as sec nas server is not reachable" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_FAILED;
    } else if (m_scanStatus == SCANNER_STATUS::PROTECTED_SERVER_NOT_REACHABLE) {
        HCP_Log(ERR, MODULE) << "Scan failed as protected nas server is not reachable" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_FAILED;
    } else if (m_scanStatus == SCANNER_STATUS::ERROR_INC_TO_FULL) {
        HCP_Log(ERR, MODULE) << "Scan failed as to change INC to FULL Backup" << HCPENDLOG;
        jobStatus = SubJobStatus::FAILED;
        scanTaskStatus = SCANNER_TASK_STATUS::SCANNER_TASK_STATUS_FAILED;
    }
    return;
}

void VolumeIndex::PrintRepo(const StorageRepository& repo) const
{
    INFOLOG("Enter PrintRepo!");
    INFOLOG("repo type: %d", repo.repositoryType);
    INFOLOG("repo isLocal: %d", repo.isLocal);
    for (auto tmp : repo.path) {
        INFOLOG("repo path: %s", tmp.c_str());
    }
    INFOLOG("repo protocol: %d", repo.protocol);
    INFOLOG("repo remotePath: %s", repo.remotePath.c_str());
    INFOLOG("repo remoteName: %s", repo.remoteName.c_str());
    INFOLOG("repo extendInfo: %s", repo.extendInfo.c_str());
}

void VolumeIndex::ReportJob(SubJobStatus::type status)
{
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    AddLogDetail(logDetail, "", JobLogLevel::TASK_LOG_INFO);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, status);
}

void VolumeIndex::FillScanConfigMetaPath(const std::string& volumeName)
{
    // 最后一层的latest是scanner模块生成的
    m_scanConfig.metaPath = PluginUtils::PathJoin(m_cacheFsPath, META, LATEST, volumeName);
    m_scanConfig.metaPathForCtrlFiles = PluginUtils::PathJoin(m_cacheFsPath, CTRL, volumeName);
    INFOLOG("FillScanConfigMetaPath, metaPath: %s, volumeName: %s", m_scanConfig.metaPath.c_str(), volumeName.c_str());
    PluginUtils::CreateDirectory(m_scanConfig.metaPath);
    PluginUtils::CreateDirectory(m_scanConfig.metaPathForCtrlFiles);
}

void VolumeIndex::SetScanHashType()
{
    if (m_indexType != VolumeIndexType::VOLUME_INDEX_TYPE_INC) {
        return;
    }
    // 判断是否是增量扫描，如果是，则需要设置相同的算法
    string prevDcacheFile = PluginUtils::GetPathName(PluginUtils::GetPathName(m_scanConfig.metaPath))
        + PREVIOUS + dir_sep + DIRCACHE_FILE_NAME;
    DBGLOG("prevDcacheFile path: %s", prevDcacheFile.c_str());
    if (!PluginUtils::IsFileExist(prevDcacheFile)) {
        INFOLOG("prevDcacheFile not exist");
        return;
    }
    INFOLOG("prevDcacheFile exists");
    std::shared_ptr<DirCacheParser> prevDcacheObj = CreateDcacheObj(prevDcacheFile);
    if (prevDcacheObj == nullptr) {
        ERRLOG("dcache obj create fail");
        return;
    }
    if (stoi(prevDcacheObj->GetVersion()) > stoi("2.0")) {
        INFOLOG("preVersion is newer than 2.0, use SHA_1");
        m_scanConfig.scanHashType = SCAN_HASH_TYPE::SHA_1;
    } else {
        INFOLOG("preVersion is 2.0, use CRC");
        m_scanConfig.scanHashType = SCAN_HASH_TYPE::CRC;
    }
    // close file
    CTRL_FILE_RETCODE ret = prevDcacheObj->Close(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Close dcache control file failed, errno: %d", ret);
    }
    return;
}

std::shared_ptr<DirCacheParser> VolumeIndex::CreateDcacheObj(const std::string& fname) const
{
    DBGLOG("Create Dcache obj with only read : %s", fname.c_str());
    std::shared_ptr<DirCacheParser> dirCacheObj = nullptr;
    dirCacheObj = std::make_shared<DirCacheParser>(fname);
    if (dirCacheObj == nullptr) {
        ERRLOG("Create scanner dircache instance failed filename: %s", fname.c_str());
        return nullptr;
    }

    CTRL_FILE_RETCODE ret = dirCacheObj->Open(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Open dcache control file failed");
        return nullptr;
    }
    return dirCacheObj;
}

void VolumeIndex::FillScanConfigForGenerateRfi(const string& prevDcachePath,
    const string& curDcachePath, bool isLastScan, bool isPre)
{
    m_scanConfig.jobId = m_indexPara->jobId;
    m_scanConfig.subJobId = m_subJobInfo->subJobId;
    m_scanConfig.copyId = m_curRepoExtendInfo->copyId;
    m_scanConfig.scanType = ScanJobType::RFI_GEN;
    m_scanConfig.scanIO = IOEngine::DEFAULT;
    m_scanConfig.lastBackupTime = 0;
    m_scanConfig.usrData = (void*)this;
    m_scanConfig.isPre = isPre;
    m_scanConfig.isLastScan = isLastScan;
    if (Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "KEEP_RFI_IN_CACHE_REPO") == "1") {
        INFOLOG("set to keep rfifile, jobId %s", m_indexPara->jobId.c_str());
        m_scanConfig.keepRfiFile = true;
    }
    /* config meta path */
    m_scanConfig.metaPath = PluginUtils::PathJoin(m_cacheFsPath, META);
    m_scanConfig.metaPathForCtrlFiles = PluginUtils::PathJoin(m_cacheFsPath, RFI);
    m_scanConfig.curDcachePath = curDcachePath + LATEST;
    m_scanConfig.prevDcachePath = prevDcachePath + LATEST;
    INFOLOG("FillScanConfigForGenerateRfi, metaPath: %s,curDcachePath: %s, prevDcachePath: %s",
            m_scanConfig.metaPath.c_str(),
            m_scanConfig.curDcachePath.c_str(),
            m_scanConfig.prevDcachePath.c_str());

#ifdef WIN32
    // windows 原生格式只挂载到pvc_dee_share , 要拼接上remotePath后面的路径
    if (m_indexRepo->path[0].find(RFI) == std::string::npos) {
        std::size_t pos = m_indexRepo->remotePath.find(RFI);
        std::string rfiReletivePath = PluginUtils::ReverseSlash(m_indexRepo->remotePath.substr(pos));
        m_scanConfig.indexPath = PluginUtils::PathJoin(m_indexRepo->path[0], rfiReletivePath);
    } else {
        m_scanConfig.indexPath = m_indexRepo->path[0];
    }
#else
    m_scanConfig.indexPath = m_indexRepo->path[0];
#endif
    m_scanConfig.maxOpendirReqCount = MAX_OPEN_DIR_REQ_COUNT;
    m_scanConfig.generatorIsFull = prevDcachePath == "";
    // /* 记录线程数 */
    m_scanConfig.maxCommonServiceInstance = 1;
    m_scanConfig.scanCtrlMaxDataSize = to_string(volumeprotect::ONE_GB);
    m_scanConfig.scanCtrlMinDataSize = to_string(HALF_GB);
    m_scanConfig.scanCtrlFileTimeSec = SCAN_CTRL_FILE_TIMES_SEC;
    m_scanConfig.scanCtrlMaxEntriesFullBkup = SCAN_CTRL_MAX_ENTRIES_FULL_BACKUP;
    m_scanConfig.scanCtrlMaxEntriesIncBkup = SCAN_CTRL_MAX_ENTRIES_INCBKUP;
    m_scanConfig.scanCtrlMinEntriesFullBkup = SCAN_CTRL_MIN_ENTRIES_FULL_BKUP;
    m_scanConfig.scanCtrlMinEntriesIncBkup = SCAN_CTRL_MIN_ENTRIES_INC_BKUP;
    m_scanConfig.scanMetaFileSize = volumeprotect::ONE_GB;
    m_scanConfig.maxWriteQueueSize = SCAN_CTRL_MAX_QUEUE_SIZE;
    m_scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    m_scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    m_scanConfig.rfiCtrlCb = GenerateRfiCtrlFileCb;
}

void VolumeIndex::ScannerCtrlFileCallBack(void *usrData, const string &controlFilePath)
{
    usrData = usrData;
    DBGLOG("Callback Received for control File path: %s", controlFilePath.c_str());
    return;
}

void VolumeIndex::ScannerHardLinkCallBack(void *usrData, const string &controlFilePath)
{
    usrData = usrData;
    DBGLOG("Callback Received for control File path: %s", controlFilePath.c_str());
    return;
}

void VolumeIndex::BackupDirMTimeCallBack(void *usrData, const string &controlFilePath)
{
    usrData = usrData;
    DBGLOG("Callback Received for control File path: %s", controlFilePath.c_str());
    return;
}

void VolumeIndex::BackupDelCtrlCallBack(void *usrData, const string &controlFilePath)
{
    usrData = usrData;
    DBGLOG("Callback Received for control File path: %s", controlFilePath.c_str());
    return;
}

void VolumeIndex::GeneratedCopyCtrlFileCb(void *usrData, string ctrlFile)
{
    (void)usrData;
    DBGLOG("GeneratedCopyCtrlFileCb: %s", ctrlFile.c_str());
}

void VolumeIndex::GeneratedHardLinkCtrlFileCb(void *usrData, string ctrlFile)
{
    (void)usrData;
    DBGLOG("GenreateHardlinkCtrlFileCb: %s", ctrlFile.c_str());
}

void VolumeIndex::GenerateRfiCtrlFileCb(void *usrData, RfiCbStruct cbParam)
{
    (void)usrData;
    INFOLOG("rfi cb : jobId - %s, subjobId - %s, copyId - %s, rfiFileName - %s, isComplete - %d, isFailed %d",
        cbParam.jobId.c_str(), cbParam.subJobId.c_str(), cbParam.copyId.c_str(), cbParam.rfiZipFileName.c_str(),
        cbParam.isComplete, cbParam.isFailed);
    // DEE need RFI zip file name to be like "./index_xxxxx.zip", need to convert backslash on windows
    std::string posixRfiZipFileName =  cbParam.rfiZipFileName;
    std::replace(posixRfiZipFileName.begin(), posixRfiZipFileName.end(), WIN_SEPARATOR, POSIX_SEPARATOR);
    
    ActionResult result;
    LogDetail logDetail;
    logDetail.__set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    logDetail.__set_level(JobLogLevel::TASK_LOG_INFO);
    logDetail.__set_description("generate a rfi file success");
    std::vector<LogDetail> logDetailList;
    logDetailList.push_back(logDetail);
    SubJobDetails subJobDetails;
    subJobDetails.__set_jobId(cbParam.jobId);
    subJobDetails.__set_subJobId(cbParam.subJobId);
    subJobDetails.__set_logDetail(logDetailList);
    if (cbParam.isFailed) {
        subJobDetails.__set_jobStatus(SubJobStatus::FAILED);
        JobService::ReportJobDetails(result, subJobDetails);
        return;
    }
    if (cbParam.isComplete && cbParam.isLastScan) {
        subJobDetails.__set_progress(PROGRESS_COMPLETE);
        subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    } else {
        subJobDetails.__set_progress(0);
        subJobDetails.__set_jobStatus(SubJobStatus::RUNNING);
    }
    string extendInfo {};
    RfiGeneratationParam param;
    param.copyId = cbParam.copyId;
    param.rfiFiles.push_back(posixRfiZipFileName);
    JsonHelper::StructToJsonString(param, extendInfo);
    INFOLOG("Report RFI struct: %s", extendInfo.c_str());
    subJobDetails.__set_extendInfo(extendInfo);
    JobService::ReportJobDetails(result, subJobDetails);
    return;
}