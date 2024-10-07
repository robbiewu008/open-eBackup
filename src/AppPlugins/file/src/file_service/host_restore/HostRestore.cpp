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
#include "HostRestore.h"
#include <chrono>
#include <thread>
#include <memory>
#include <string>
#include "utils/PluginUtilities.h"
#include "define/Types.h"
#include "ShareResourceManager.h"
#include "BackupMgr.h"
#include "log/Log.h"
#include "File.h"
#include "filter/CtrlFileFilter.h"
#include "ScanMgr.h"
#include "PluginUtilities.h"
#include "config_reader/ConfigIniReader.h"
#include "constant/ErrorCode.h"
#include "system/System.hpp"
#include "common/EnvVarManager.h"
#include "common/Thread.h"
#include "host/OsIdentifier.h"

using namespace std;
namespace FilePlugin {
namespace {
    constexpr int REPORT_PROGRESS_TO_PM_INTERVAL = 30;
    constexpr int REPORT_SPEED_INTERVAL = 10;
    constexpr int MONITOR_SCAN_PROGRESS_INTERVAL = 10;
    constexpr int MONITOR_RESTORE_PROGRESS_INTERVAL = 5;
    constexpr int THREAD_NUM_32 = 32;
    constexpr int RETRY_TIME = 10;
    constexpr uint32_t NUMBER4000 = 4000;
    constexpr uint32_t NUMBER1000 = 1000;
    constexpr uint32_t NUMBER10000 = 10000;
    constexpr uint32_t NUMBER1024 = 1024;
    constexpr uint32_t NUMBER120 = 120;
    constexpr uint32_t NUMBER100 = 100;
    constexpr uint32_t NUMBER10 = 10;
    constexpr uint32_t NUMBER5 = 5;
    constexpr uint32_t NUMBER2 = 2;
    constexpr uint32_t NUMBER1 = 1;
    constexpr uint32_t NUMBER0 = 0;
    constexpr uint32_t REPORT_RUNNING_INTERVAL = 10;
    constexpr uint32_t REPORT_RUNNING_TIMES = 6;
    constexpr uint32_t FIRST_GENERATE_CONTROL_FILE = 0;

    const string  MODULE = "HostRestore";
    const string  SPEED_STR = "_speed_host_restore";
    constexpr auto BACKUP_KEY_SUFFIX = "_backup_stats";
    constexpr auto RESTORE_OPTION_OVERWRITE = "OVERWRITING";
    constexpr auto RESTORE_OPTION_SKIP = "SKIP";
    constexpr auto RESTORE_OPTION_REPLACE = "REPLACE";
    constexpr auto SCANNER_STAT = "restore_scanner_status.json";

    struct RestoreAdvancedparamters {
        string failedScript;
        string postScript;
        string preScript;
        string restoreOption;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(failedScript, failed_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(postScript, post_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(preScript, pre_script)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(restoreOption, restoreOption)
        END_SERIAL_MEMEBER
    };

    struct FileSetInfo {
        string filters;
        string paths;
        string templateId;
        string templateName;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(filters, filters)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(templateId, templateId)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(templateId, templateId)
        END_SERIAL_MEMEBER
    };

    const char SLASH_CH = '/';
    const int MAX_RETRY_CNT = 3;
}

uint32_t HostRestore::m_numberOfSubTask = 0;

int HostRestore::PrerequisiteJob()
{
    HCP_Log(INFO, MODULE) << "Enter HostRestore PrerequisiteJob" << HCPENDLOG;
    m_restoreJobInfo = dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    HCP_Log(INFO, MODULE) << "m_restoreJobInfo->jobId is " << m_restoreJobInfo->jobId << HCPENDLOG;
    int ret = PrerequisiteJobInner();
    if (ret != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100));
        HCP_Log(ERR, MODULE) << "Restore PrerequisiteJob failed jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
        HCP_Log(INFO, MODULE) << "Restore PrerequisiteJob success jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    }
    SetJobToFinish();
    return ret;
}

int HostRestore::GenerateSubJob()
{
    HCP_Log(INFO, MODULE) << "Enter HostRestore restore GenerateSubJob()" << HCPENDLOG;
    m_generateSubjobFinish = false;
    m_restoreJobInfo = dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    GetRestoreType();
    std::thread keepAlive = std::thread(&HostRestore::KeepPluginAlive, this);
    int ret;
    if (m_aggregateRestore) {
        ret = GenerateAggregateSubJobInner();
    } else {
        ret = GenerateSubJobInner();
    }
    if (m_scanner != nullptr) {
        m_scanner->Destroy();
        m_scanner.reset();
    }
    if (ret != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100));
        HCP_Log(ERR, MODULE) << "Restore Generate Sub Job failed jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
        HCP_Log(INFO, MODULE) << "Restore Generate Sub Job success jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    }
    SetJobToFinish();
    m_generateSubjobFinish = true;
    keepAlive.join();
    return ret;
}

int HostRestore::ExecuteSubJob()
{
    HCP_Log(INFO, MODULE) << "Enter HostRestore restore ExecuteSubJob" << HCPENDLOG;
    m_restoreJobInfo = dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    if (GetExecuteSubJobType() != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100));
        HCP_Log(ERR, MODULE) << "Get execute SubJobType failed, jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
        SetJobToFinish();
        return Module::FAILED;
    }
    int ret;
    if (m_finalReportStatistic) {
        ret = FinalReportStatictisInfo();
    } else if (m_checkSubJob) {
        ret = ExecuteCheckSubJobInner();
    } else {
        ret = ExecuteSubJobInner();
    }

    if (ret != Module::SUCCESS) {
        uint64_t errCode = INITIAL_ERROR_CODE;
        if (m_backupStatus == BackupPhaseStatus::FAILED_NOSPACE) {
            errCode = E_BACKUP_FAILED_NOSPACE_ERROR;
        } else if (m_backupStatus == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE) {
            errCode = E_BACKUP_BACKUP_SECONDARY_SERVER_NOT_REACHABLE;
        }
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100),
            "nas_plugin_hetro_restore_data_fail_label", errCode);
        HCP_Log(ERR, MODULE) << "Restore ExecuteSubJob failed jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
        HCP_Log(INFO, MODULE) << "Restore ExecuteSubJob success jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    }
    SetJobToFinish();
    return ret;
}

int HostRestore::PostJob()
{
    HCP_Log(INFO, MODULE) << "Enter HostRestore PostJob" << HCPENDLOG;
    m_restoreJobInfo = dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    int ret = PostJobInner();
    if (ret != Module::SUCCESS) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100));
        HCP_Log(ERR, MODULE) << "Restore PostJob failed jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    } else {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
        HCP_Log(INFO, MODULE) << "Restore PostJob success jobId: " << m_restoreJobInfo->jobId << HCPENDLOG;
    }
    SetJobToFinish();
    return ret;
}

int HostRestore::PrerequisiteJobInner() const
{
    HCP_Log(DEBUG, MODULE) << "prerequisite sub task, task id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    PluginUtils::CreateDirectory(m_failureRecordRoot);
    return Module::SUCCESS;
}

int HostRestore::GenerateSubJobInner()
{
    HCP_Log(DEBUG, MODULE) << "Enter GenerateSubJobInner, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    if (InitGenerateJobInfo() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Init job info failed, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }
    PrintImportInfo();
    if (!InitIdGenerator()) {
        return Module::FAILED;
    }
    if (!m_scanRedo) {
        string label = "file_plugin_host_backup_scan_start_label";
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, label);
        for (uint32_t volumeOrder = 0; volumeOrder < m_singleCopyVolumeNumber; ++volumeOrder) {
            if (GenerateSubJobByDcacheAndFcache(volumeOrder) != Module::SUCCESS) {
                HCP_Log(ERR, MODULE) << "Generate subJob by acache and Fcache failed, the volume number is: "
                    << volumeOrder << HCPENDLOG;
                return Module::FAILED;
            }
        }
        if (!WriteScannSuccess()) {
            ERRLOG("Write scanner success failed");
            return Module::FAILED;
        }
        // 增量恢复根据增量备份的统计数据进行上报，普通恢复在GenerateSubJobByDcacheAndFcache中计算
        if (m_incrementalRestore) {
            CalcuScannerStatistic();
        }
        label = "file_plugin_host_restore_scan_data_completed_label";
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, label,
            to_string(m_scannerStatistic.mTotDirsToBackup),
            to_string(m_scannerStatistic.mTotFilesToBackup),
            PluginUtils::FormatCapacity(m_scannerStatistic.mTotalSizeToBackup));
    }
    if (!m_incrementalRestore && !CreateRestoreSubJob()) {
        return Module::FAILED;
    }

    if (GenerateTearDownSubJob() != Module::SUCCESS || GenerateCheckSubJob() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Generate tear down subJob failed" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool HostRestore::WriteScannSuccess()
{
    INFOLOG("Enter Write scanner success");
    RestoreAdvancedparamters restoreAdvancedparamters;
    if (!JsonFileTool::WriteToFile(restoreAdvancedparamters, m_scanStatusPath)) {
        ERRLOG("Write to File failed");
        return false;
    }
    return true;
}

void HostRestore::CheckScanRedo()
{
    INFOLOG("Enter check scanner redo");
    if (PluginUtils::IsFileExist(m_scanStatusPath)) {
        WARNLOG("scanner redo");
        m_scanRedo = true;
    }
    INFOLOG("Exit check scanner redo");
}

void HostRestore::GetRestoreType()
{
    HCP_Log(INFO, MODULE) << "Enter GetRestoreType" << HCPENDLOG;
    if (m_restoreJobInfo->jobParam.restoreType == AppProtect::RestoreJobType::type::FINE_GRAINED_RESTORE) {
        m_fineGrainedRestore = true;
        HCP_Log(INFO, MODULE) << "=====This is FINE_GRAINED_RESTORE=====" << HCPENDLOG;
    }
    if (m_restoreJobInfo->copies[0].formatType == CopyFormatType::type::INNER_DIRECTORY) {
        m_aggregateRestore = true;
        HCP_Log(INFO, MODULE) << "=====This is AGGREGATE_RESTORE=====" << HCPENDLOG;
    }
    if (m_restoreJobInfo->copies[0].dataType == AppProtect::CopyDataType::TAPE_STORAGE_COPY) {
        m_tapeCopy = true;
        HCP_Log(INFO, MODULE) << "=====This is TYPE STORAGE COPY=====" << HCPENDLOG;
    }
    Json::Value paramValue;
    if (Module::JsonHelper::JsonStringToJsonValue(m_restoreJobInfo->extendInfo, paramValue)) {
        if (paramValue.isMember("isAccumulate") && paramValue["isAccumulate"].asString() == "true") {
            m_incrementalRestore = true;
            INFOLOG("=====This is INCREMENTAL_RESTORE=====");
        }
    }
}

int HostRestore::GenerateAggregateSubJobInner()
{
    HCP_Log(DEBUG, MODULE) << "Enter GenerateAggregateSubJobInner, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    if (InitAggregateGenerateJobInfo() != Module::SUCCESS) {
        return Module::FAILED;
    }
    CheckScanRedo();
    if (!InitIdGenerator()) {
        return Module::FAILED;
    }
    string label = "file_plugin_host_backup_scan_start_label";
    ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, label);

    if (!m_scanRedo) {
        // 对于聚合细粒度恢复， 只需要最后一个副本和上一个副本做差异生成控制文件
        if (m_fineGrainedRestore) {
            if (GenerateRestoreExecuteSubJobsForAggregateFineGrained() != Module::SUCCESS) {
                HCP_Log(ERR, MODULE) << "Generate restore execute subJobs for aggregate failed." << HCPENDLOG;
                return Module::FAILED;
            }
        } else {
            if (GenerateRestoreExecuteSubJobsForAggregate() != Module::SUCCESS) {
                HCP_Log(ERR, MODULE) << "Generate restore execute subJobs for aggregate failed." << HCPENDLOG;
                return Module::FAILED;
            }
            // 增量恢复根据增量备份的统计数据进行上报，普通恢复在GenerateSubJobByDcacheAndFcache中计算
            if (m_incrementalRestore) {
                CalcuScannerStatistic();
            }
        }
        if (!WriteScannSuccess()) {
            ERRLOG("Write scanner success failed");
            return Module::FAILED;
        }
        label = "file_plugin_host_restore_scan_data_completed_label";
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, label,
            to_string(m_scannerStatistic.mTotDirsToBackup),
            to_string(m_scannerStatistic.mTotFilesToBackup),
            PluginUtils::FormatCapacity(m_scannerStatistic.mTotalSizeToBackup));
    }
    if (!m_incrementalRestore && !CreateRestoreSubJob()) {
        ERRLOG("Create restore sub job failed");
        return Module::FAILED;
    }
    if (GenerateTearDownSubJob() != Module::SUCCESS || GenerateCheckSubJob() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Generate tear down subJob failed" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::InitAggregateGenerateJobInfo()
{
    HCP_Log(DEBUG, MODULE) << "Enter InitAggregateGenerateJobInfo" << HCPENDLOG;
    m_numberCopies =  m_restoreJobInfo->copies.size();
    if (GetRepoInfoForAggregate() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "GetRepoInfoForAggregate failed." << HCPENDLOG;
        return Module::FAILED;
    }
    if (RecordControlFilePathForAggregate() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "RecordControlFilePathForAggregate failed." << HCPENDLOG;
        return Module::FAILED;
    }
    if (!m_scanRedo && CreateSrcDirForAggregate() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "CreateSrcDirForAggregate failed." << HCPENDLOG;
        return Module::FAILED;
    }
    if (InitialReportSpeedInfo() != Module::SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}


int HostRestore::GetRepoInfoForAggregate()
{
    HCP_Log(DEBUG, MODULE) << "Enter GetRepoInfoForAggregate" << HCPENDLOG;
    // 多个副本共用一个cache仓,agent只填充最后一个副本的路径
    int lastCopyNumber =  m_restoreJobInfo->copies.size() - 1;
    Copy lastCopy = m_restoreJobInfo->copies[lastCopyNumber];
    string dataPath;
    string metaPath;
    for (uint32_t i = 0; i < lastCopy.repositories.size(); i++) {
        if (m_restoreJobInfo->copies[0].repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFsPath = lastCopy.repositories[i].path[0];
        } else if (lastCopy.repositories[i].repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            dataPath = lastCopy.repositories[i].path[0];
        } else if (lastCopy.repositories[i].repositoryType == RepositoryDataType::META_REPOSITORY) {
            metaPath = lastCopy.repositories[i].path[0];
        }
    }
    if (dataPath.empty() || metaPath.empty()) {
        HCP_Log(ERR, MODULE) << "get cachePath and dataPath and metaPath failed.\n"
                                << "\n dataPath is: " << dataPath
                                << "\n metaPath is: " << metaPath << HCPENDLOG;
        return Module::FAILED;
    }
    m_scanStatusPath = PluginUtils::PathJoin(m_cacheFsPath, SCANNER_STAT);
    m_metaOriPath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "all_meta");
    for (uint32_t i = 0; i < m_numberCopies; ++i) {
        AggCopyExtendInfo aggCopyExtendInfo;
        if (GetCopyExtendInfo(i, aggCopyExtendInfo) != Module::SUCCESS) {
            return Module::FAILED;
        }
        // 修正聚合格式下副本的meta、data仓的路径
#ifdef WIN32
        string copyDataPath = dataPath.substr(0, dataPath.rfind('\\')) + '/' + aggCopyExtendInfo.dataPathSuffix;
        string copyMetaPath = metaPath.substr(0, metaPath.rfind('\\')) + '/' + aggCopyExtendInfo.metaPathSuffix;
#else
        string copyDataPath = dataPath + '/' + aggCopyExtendInfo.dataPathSuffix;
        string copyMetaPath = metaPath + '/' + aggCopyExtendInfo.metaPathSuffix;
#endif
        m_dataFsPathList.push_back(copyDataPath);
        m_metaFsPathList.push_back(copyMetaPath);
        
        HCP_Log(INFO, MODULE) << "the StorageRepository["<< i << "]: "
                              << " dataPath is: " << copyDataPath
                              << " metaPath is: " << copyMetaPath
                              << " dataPathSuffix: " << aggCopyExtendInfo.dataPathSuffix
                              << " metaPathSuffix: " << aggCopyExtendInfo.metaPathSuffix << HCPENDLOG;
    }
    return Module::SUCCESS;
}

int HostRestore::GetCopyExtendInfo(const int& copyOrder, AggCopyExtendInfo& aggCopyExtendInfo)
{
    HCP_Log(DEBUG, MODULE) << "Enter GetCopyExtendInfo"<< HCPENDLOG;
    if (m_tapeCopy) {
        TapeCopyExtendInfo tapeCopyExtendInfo;
        if (!Module::JsonHelper::JsonStringToStruct(m_restoreJobInfo->copies[copyOrder].extendInfo,
                                                    tapeCopyExtendInfo)) {
            HCP_Log(ERR, MODULE) << "parse tapeCopyExtendInfo failed. tapeCopyExtendInfo is: "
                << m_restoreJobInfo->copies[copyOrder].extendInfo << m_restoreJobInfo->jobId << HCPENDLOG;
            return Module::FAILED;
        }
        aggCopyExtendInfo = tapeCopyExtendInfo.extendInfo;
        return Module::SUCCESS;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_restoreJobInfo->copies[copyOrder].extendInfo,
                                                aggCopyExtendInfo)) {
        HCP_Log(ERR, MODULE) << "parse aggCopyExtendInfo failed. tapeCopyExtendInfo is: "
            << m_restoreJobInfo->copies[copyOrder].extendInfo << "jobId is: " << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::RecordControlFilePathForAggregate()
{
    HCP_Log(DEBUG, MODULE) << "Enter RecordControlFilePathForAggregate"<< HCPENDLOG;
    m_scanContrlFilePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "scan", "ctrl");
    m_restoreContrlFilePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "restore", "ctrl");
    for (uint32_t i = 0; i < m_numberCopies; i++) {
        string dcaheAndFcachePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId,
            "all_meta", to_string(i));
        m_dcacheAndFcachePathForCopies.push_back(dcaheAndFcachePath);
    }
    return Module::SUCCESS;
}

int HostRestore::CreateSrcDirForAggregate() const
{
    HCP_Log(DEBUG, MODULE) << "Enter CreateSrcDirForAggregate"<< HCPENDLOG;
    if (!PluginUtils::CreateDirectory(m_scanContrlFilePath)) {
        return Module::FAILED;
    }
    if (!PluginUtils::CreateDirectory(m_restoreContrlFilePath)) {
        return Module::FAILED;
    }
    for (uint32_t i = 0; i < m_numberCopies; ++i) {
        string dcaheAndFcachePath = m_dcacheAndFcachePathForCopies[i];
        if (!PluginUtils::CreateDirectory(dcaheAndFcachePath)) {
            return Module::FAILED;
        }
    }
    return Module::SUCCESS;
}

int HostRestore::HandleSingleCopyForAggregate()
{
    HCP_Log(DEBUG, MODULE) << "Enter GetSingleCopyInfoForAggregate." << HCPENDLOG;
    string dcacheAndFcachePath = m_dcacheAndFcachePathForCopies[m_orderNumberForCopies];
    string metaZipPath = PluginUtils::PathJoin(m_metaFsPathList[m_orderNumberForCopies], "filemeta", METAFILE_ZIP_NAME);
    if (UntarWholePackage(metaZipPath, dcacheAndFcachePath) != Module::SUCCESS) {
            return Module::FAILED;
    }
    if (GetMetaZipDirList(dcacheAndFcachePath) != Module::SUCCESS) {
        return Module::FAILED;
    }
    m_restoreMetaPath = dcacheAndFcachePath;  // 复用原生格式扫描meta路径的参数
    m_singleCopyVolumeNumber = m_currentMetaZipDirFullPathList.size();
    for (uint32_t volumeOrder = 0; volumeOrder < m_singleCopyVolumeNumber; ++volumeOrder) {
        HCP_Log(INFO, MODULE) << "the copy number is:" << m_orderNumberForCopies << ".   the volume number is: "
                              << volumeOrder << HCPENDLOG;
        if (GenerateSubJobByDcacheAndFcache(volumeOrder) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }
    // 保留副本的信息，用于增量副本的恢复
    m_previousMetaZipDirList = m_currentMetaZipDirFullPathList;
    return Module::SUCCESS;
}

int HostRestore::GenerateRestoreExecuteSubJobsForAggregate()
{
    HCP_Log(DEBUG, MODULE) << "Enter GenerateRestoreExecuteSubJobsForAggregate." << HCPENDLOG;
    for (uint32_t i = 0; i < m_numberCopies; ++i) {
        m_orderNumberForCopies = i;
        HCP_Log(INFO, MODULE) << "==========m_orderNumberForCopies is: " << m_orderNumberForCopies << HCPENDLOG;
        if (m_incrementalRestore) {
            if (i != m_numberCopies - 1) {
                continue;
            }
        }
        if (HandleSingleCopyForAggregate() != Module::SUCCESS) {
            HCP_Log(INFO, MODULE) << "failed Handle Single Copy For Aggregate."
                                  << "the copy number is: " << m_orderNumberForCopies << HCPENDLOG;
            return Module::FAILED;
        }
    }
    HCP_Log(INFO, MODULE) << "Generate restore Execute SubJobs For Aggregate completed." << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::GenerateRestoreExecuteSubJobsForAggregateFineGrained()
{
    HCP_Log(DEBUG, MODULE) << "Enter GenerateRestoreExecuteSubJobsForAggregateFineGrained." << HCPENDLOG;
    if (HandleCopyForAggregateFineGrained() != Module::SUCCESS) {
        HCP_Log(INFO, MODULE) << "Handle copy For aggregate fineGrained failed." << HCPENDLOG;
        return Module::FAILED;
    }
    m_singleCopyVolumeNumber = m_currentMetaZipDirFullPathList.size();
    m_orderNumberForCopies = m_numberCopies - 1;
    if (!m_scanRedo) {
        for (uint32_t volumeOrder = 0; volumeOrder < m_singleCopyVolumeNumber; ++volumeOrder) {
            HCP_Log(INFO, MODULE) << "the copy number is:" << m_orderNumberForCopies << ".   the volume number is: "
                                << volumeOrder << HCPENDLOG;
            if (GenerateSubJobByDcacheAndFcache(volumeOrder) != Module::SUCCESS) {
                return Module::FAILED;
            }
            if (m_scanner != nullptr) {
                m_scanner->Destroy();
                m_scanner.reset();
            }
        }
    }
    HCP_Log(DEBUG, MODULE) << "Generate restore Execute SubJobs For Aggregate fineGrainedRestore completed."
                          << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::HandleCopyForAggregateFineGrained()
{
    HCP_Log(DEBUG, MODULE) << "Enter HandleCopyForAggregateFineGrained." << HCPENDLOG;
    if (HandleThePenultimateCopy() != Module::SUCCESS) {
            return Module::FAILED;
    }
    if (HandleTheLastCopy() != Module::SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::HandleThePenultimateCopy()
{
    HCP_Log(DEBUG, MODULE) << "Enter HandleThePenultimateCopy." << HCPENDLOG;
    if (m_numberCopies < NUMBER2) {
        HCP_Log(INFO, MODULE) << "the copy number less than 2." << HCPENDLOG;
        return Module::SUCCESS;
    }
    int copyOrder = m_numberCopies - NUMBER2; // 倒数第二个副本
    string dcacheAndFcachePath = m_dcacheAndFcachePathForCopies[copyOrder];
    string metaZipPath =  PluginUtils::PathJoin(m_metaFsPathList[copyOrder], "filemeta", METAFILE_ZIP_NAME);
    if (UntarWholePackage(metaZipPath, dcacheAndFcachePath) != Module::SUCCESS) {
            return Module::FAILED;
    }
    if (GetMetaZipDirList(dcacheAndFcachePath) != Module::SUCCESS) {
        return Module::FAILED;
    }
    m_singleCopyVolumeNumber = m_currentMetaZipDirFullPathList.size();
    for (uint32_t volumeOrder = 0; volumeOrder < m_singleCopyVolumeNumber; ++volumeOrder) {
        HCP_Log(INFO, MODULE) << "the volumeOrder is:" << volumeOrder  << HCPENDLOG;
        if (UntarDcachefiles(volumeOrder) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }
    m_previousMetaZipDirList = m_currentMetaZipDirFullPathList;
    PrintImportInfo();
    return Module::SUCCESS;
}

int HostRestore::HandleTheLastCopy()
{
    HCP_Log(DEBUG, MODULE) << "Enter HandleTheLastCopy for Aggregate Finegrain restore.." << HCPENDLOG;
    if (m_numberCopies < 1) {
        HCP_Log(INFO, MODULE) << "No copy for Aggregate Finegrain restore." << HCPENDLOG;
        return Module::FAILED;
    }
    int copyOrder = m_numberCopies - 1; // 最后一个副本
    string dcacheAndFcachePath = m_dcacheAndFcachePathForCopies[copyOrder];
    string metaZipPath =  PluginUtils::PathJoin(m_metaFsPathList[copyOrder], "filemeta", METAFILE_ZIP_NAME);
    if (UntarWholePackage(metaZipPath, dcacheAndFcachePath) != Module::SUCCESS) {
            return Module::FAILED;
    }
    if (GetMetaZipDirList(dcacheAndFcachePath) != Module::SUCCESS) {
        return Module::FAILED;
    }
    PrintImportInfo();
    return Module::SUCCESS;
}

int HostRestore::GetSpecialConfigForRestore()
{
    AggCopyExtendInfo aggCopyExtendInfo;
    // 默认从第一个副本中取聚合恢复需要的参数
    uint32_t copyOrder = m_subJobPathsInfo.copyOrder;
    if (GetCopyExtendInfo(copyOrder, aggCopyExtendInfo) != Module::SUCCESS) {
        return Module::FAILED;
    }
    m_maxSizeAfterAggregate = std::stoul(aggCopyExtendInfo.maxSizeAfterAggregate);
    m_maxSizeToAggregate = std::stoul(aggCopyExtendInfo.maxSizeToAggregate);
    return Module::SUCCESS;
}

void HostRestore::PrintImportInfo() const
{
    HCP_Log(DEBUG, MODULE) << "the m_singleCopyVolumeNumber is " << m_singleCopyVolumeNumber << HCPENDLOG;
    int i = 0;
    for (auto iter = m_currentMetaZipDirFullPathList.begin(); iter != m_currentMetaZipDirFullPathList.end(); ++iter) {
        HCP_Log(INFO, MODULE) << "the m_currentMetaZipDirFullPathList[" << i <<"] is " << *iter << HCPENDLOG;
        ++i;
    }
}

bool HostRestore::CreateRestoreSubJob()
{
    vector<string> copyList;
    if (!PluginUtils::GetDirListInDirectory(m_scanContrlFilePath, copyList)) {
        ERRLOG("GetDirListInDirectory failed, controlFilePath: %s", m_scanContrlFilePath.c_str());
        return false;
    }
    sort(copyList.begin(), copyList.end(), [](string x, string y) {return stoi(x) < stoi(y);});
    for (std::string& copy : copyList) {
        vector<string> volumeNameList;
        std::string copyMetaPath = PluginUtils::PathJoin(m_scanContrlFilePath, copy);
        if (!PluginUtils::GetDirListInDirectory(copyMetaPath, volumeNameList)) {
            ERRLOG("GetDirListInDirectory failed, controlFilePath: %s", copyMetaPath.c_str());
            return false;
        }
        for (std::string& volumeName : volumeNameList) {
            vector<string> controlFileList {};
            string srcDir = PluginUtils::PathJoin(copyMetaPath, volumeName);
            INFOLOG("create restore subJob for volume : %s", srcDir.c_str());
            if (!PluginUtils::GetFileListInDirectory(srcDir, controlFileList)) {
                ERRLOG("Get filelist for dir failed: %s", srcDir.c_str());
                return false;
            }
            if (GenerateRestoreExecuteSubJob(controlFileList) != Module::SUCCESS) {
                ERRLOG("generate Restore ExecuteSubJob failed");
                return false;
            }
        }
    }

    return true;
}

int HostRestore::MonitorScannerProgress(int volumeOrder)
{
    HCP_Log(DEBUG, MODULE) << "start to Monitor scanner, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    int failedTime = 0;
    while (true) {
        HCP_Log(DEBUG, MODULE) << "Monitoring scanner ....." << HCPENDLOG;
        m_scannerStatus = m_scanner->GetStatus();
        HCP_Log(INFO, MODULE) << "SCANNER_STATUS is : " << static_cast<int>(m_scannerStatus) << HCPENDLOG;
        if (static_cast<int>(m_scannerStatus) < 0) {
            HCP_Log(ERR, MODULE) << "scaner run failed,status is :" << static_cast<int>(m_scannerStatus) << HCPENDLOG;
            break;
        }
        if (m_scannerStatus == SCANNER_STATUS::COMPLETED) {
            return Module::SUCCESS;
        }
        ReportJobDetails(make_tuple(JobLogLevel::type::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        Module::SleepFor(std::chrono::seconds(SLEEP_TEN_SECONDS));
    }
    return Module::FAILED;
}

int HostRestore::ExecuteSubJobInner()
{
    HCP_Log(DEBUG, MODULE) << "Execute sub task, task id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(m_restoreJobInfo->jobId);
    if (InitExecuteJobInfo() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "init Execute sub job jobInfo failed" << HCPENDLOG;
        ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_restoreJobInfo->jobId);
        return Module::FAILED;
    }
    // 用本地文件保存单个子任务的任务详细信息
    if (InitialRestoreLocalProgressInfo(m_jobId) != Module::SUCCESS) {
        ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_restoreJobInfo->jobId);
        return Module::FAILED;
    }
    m_isRestoreInProgress = true;
    std::thread updateTaskInfoThread = std::thread(&HostRestore::UpdateTaskInfo, this);
    MONITOR_BACKUP_RES_TYPE monitorRet;
    int retryCnt = 0;

    do {
        INFOLOG("Start Backup for %d time.", retryCnt);
        if (StartToRestore() != Module::SUCCESS) {
            ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_restoreJobInfo->jobId);
            m_isRestoreInProgress = false;
            updateTaskInfoThread.join();
            return Module::FAILED;
        }
        monitorRet = MonitorRestoreJobStatus();
        if (m_backup != nullptr) {
            m_backup->Destroy();
            m_backup.reset();
        }
    } while (monitorRet == MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_NEEDRETRY && ++retryCnt < MAX_RETRY_CNT);

    m_isRestoreInProgress = false;
    if (updateTaskInfoThread.joinable()) {
        updateTaskInfoThread.join();
    }

    if (retryCnt >= MAX_RETRY_CNT && monitorRet == MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_NEEDRETRY) {
        // seems this sub job is stuck for some reason , copy this control file to meta repo for further check
        INFOLOG("subjob is stuck, %s, controlFile : %s", m_subJobId.c_str(), m_subJobPathsInfo.controlFileName.c_str());
        PluginUtils::CopyFile(m_subJobPathsInfo.controlFileName, m_metaFsPath);
        monitorRet = MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_SUCCESS;
    }
    ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_restoreJobInfo->jobId);
    if (monitorRet != MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}


int HostRestore::PostJobInner()
{
    HCP_Log(DEBUG, MODULE) << "Post sub task, task id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    MergeBackupFailureRecords();
    GetCacheRepositoryPath();
    DeleteReportSpeedInfo();
    DeleteSrcDirForRestore();  // 删除子任务的
    ReportJobDetails(make_tuple(JobLogLevel::type::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100));
    return Module::SUCCESS;
}

int HostRestore::InitGenerateJobInfo()
{
    if (GetRepoInfo() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "failed to get Repo info, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }
    HCP_Log(DEBUG, MODULE) << "Enter InitJobInfo, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    m_scanContrlFilePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "scan", "ctrl");
    m_restoreContrlFilePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "restore", "ctrl");
    m_restoreMetaPath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "all_meta", "0");
    // 如果扫描重新下发，跳过创建目录和解压阶段
    CheckScanRedo();
    if (!m_scanRedo) {
        if (CreateSrcDir() != Module::SUCCESS) {
            ERRLOG("failed to create src directory, job id is %s", m_restoreJobInfo->jobId.c_str());
            return Module::FAILED;
        }
        string zipPackagePath = PluginUtils::PathJoin(m_metaFsPath, "filemeta", METAFILE_ZIP_NAME);
        if (UntarWholePackage(zipPackagePath, m_restoreMetaPath) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    if (GetMetaZipDirList(m_restoreMetaPath) != Module::SUCCESS) {
        return Module::FAILED;
    }
    m_singleCopyVolumeNumber = m_currentMetaZipDirFullPathList.size();
    if (InitialReportSpeedInfo() != Module::SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::GetExecuteSubJobType()
{
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, m_subJobPathsInfo)) {
        HCP_Log(ERR, MODULE) << "Get restore subjob info failed" << HCPENDLOG;
        return Module::FAILED;
    }

    if (m_subJobPathsInfo.subJobType == SUBJOB_TYPE_DATACOPY_COPY_PHASE) {
        m_backupPhase = BackupPhase::COPY_STAGE;
    } else if (m_subJobPathsInfo.subJobType == SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE) {
        m_backupPhase = BackupPhase::HARDLINK_STAGE;
    } else if (m_subJobPathsInfo.subJobType == SUBJOB_TYPE_DATACOPY_DELETE_PHASE) {
        m_backupPhase = BackupPhase::DELETE_STAGE;
    } else if (m_subJobPathsInfo.subJobType == SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE) {
        m_backupPhase = BackupPhase::DIR_STAGE;
    } else if (m_subJobPathsInfo.subJobType == SUBJOB_TYPE_TEARDOWN_PHASE) {
        m_finalReportStatistic = true;
    } else if (m_subJobPathsInfo.subJobType == SUBJOB_TYPE_CHECK_SUBJOB_PHASE) {
        m_checkSubJob = true;
    } else {
        HCP_Log(ERR, MODULE) << "subJobInfo.subJobType is invalid" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::ExecuteCheckSubJobInner()
{
    INFOLOG("Enter check subjob status");
    if (GetCacheRepositoryPath() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "failed to get Repo info, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }
    BackupStatistic mainStats {};
    if (!CalcuMainBackupStats(mainStats)) {
        HCP_Log(ERR, MODULE) << "get main backup stats failed." << HCPENDLOG;
        return Module::FAILED;
    }
    if (mainStats.noOfDirFailed == 0 && mainStats.noOfFilesFailed == 0) {
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS0));
    } else {
        WARNLOG("some of files or dirs failed, set main job to partial success");
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::FAILED, PROGRESS0));
    }
    return Module::SUCCESS;
}

int HostRestore::FinalReportStatictisInfo()
{
    HCP_Log(DEBUG, MODULE) << "Execute FinalReportStatictisInfo, task id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    if (GetCacheRepositoryPath() != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "failed to get Repo info, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }
    return ReportRestoreCompletionStatus();
}

int HostRestore::ReportRestoreCompletionStatus()
{
    BackupStatistic mainBackupStats {};
    if (!CalcuMainBackupStats(mainBackupStats)) {
        HCP_Log(ERR, MODULE) << "get main backup stats failed." << HCPENDLOG;
        return Module::FAILED;
    }
    m_dataSize = CalculateSizeInKB(mainBackupStats.noOfBytesCopied);
    INFOLOG("noOfDirsFailed: %llu, noOfFilesFailed: %llu, m_jobId: %s",
        mainBackupStats.noOfDirFailed, mainBackupStats.noOfFilesFailed, m_restoreJobInfo->jobId.c_str());
    if (mainBackupStats.noOfDirFailed != 0 || mainBackupStats.noOfFilesFailed != 0) {
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_WARNING, SubJobStatus::RUNNING, PROGRESS0,
                             "file_plugin_host_restore_data_completed_with_warn_label",
                             to_string(mainBackupStats.noOfDirCopied),
                             to_string(mainBackupStats.noOfFilesCopied),
                             PluginUtils::FormatCapacity(mainBackupStats.noOfBytesCopied),
                             to_string(mainBackupStats.noOfDirFailed),
                             to_string(mainBackupStats.noOfFilesFailed));
    } else {
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0,
                             "file_plugin_host_restore_data_completed_label",
                             to_string(mainBackupStats.noOfDirCopied),
                             to_string(mainBackupStats.noOfFilesCopied),
                             PluginUtils::FormatCapacity(mainBackupStats.noOfBytesCopied));
    }
    return Module::SUCCESS;
}

int HostRestore::InitExecuteJobInfo()
{
    HCP_Log(DEBUG, MODULE) << "Enter InitExecuteJobInfo, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
    GetRestoreType();
    int ret;
    if (m_aggregateRestore) {
        ret = GetRepoInfoForAggregate();
    } else {
        ret = GetRepoInfo();
    }
    if (ret != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "failed to get Repo info, job id is " << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }
    // 防止进程重启导致保存任务详情的文件的路径丢失；
    string statictisResourcePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "statistic");
    INFOLOG("statictisResourcePath: %s", statictisResourcePath.c_str());
    ShareResourceManager::GetInstance().SetResourcePath(statictisResourcePath, m_restoreJobInfo->jobId + SPEED_STR);

    if (GetRestoreCoverPolicy() != Module::SUCCESS) {
        return Module::FAILED;
    }

    m_restorePath = m_restoreJobInfo->targetObject.name;
#ifdef WIN32
    if (m_restorePath == "/") {
        m_restorePath = "";
    }
#endif
    HCP_Log(INFO, MODULE) << "m_restorePath is: " << m_restorePath << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::GetRestoreCoverPolicy()
{
    HCP_Log(DEBUG, MODULE) << "Enter GetRestoreCoverPolicy" << HCPENDLOG;
    RestoreAdvancedparamters restoreAdvancedparamters;
    if (!Module::JsonHelper::JsonStringToStruct(m_restoreJobInfo->extendInfo, restoreAdvancedparamters)) {
        HCP_Log(ERR, MODULE) << "Convert to RestoreAdvancedparamters json failed." << HCPENDLOG;
        return Module::FAILED;
    }
    if (restoreAdvancedparamters.restoreOption == RESTORE_OPTION_OVERWRITE) {
        m_coveragePolicy = RestoreReplacePolicy::OVERWRITE;
    } else if (restoreAdvancedparamters.restoreOption == RESTORE_OPTION_SKIP) {
        m_coveragePolicy = RestoreReplacePolicy::IGNORE_EXIST;
    } else if (restoreAdvancedparamters.restoreOption == RESTORE_OPTION_REPLACE) {
        m_coveragePolicy = RestoreReplacePolicy::OVERWRITE_OLDER;
    } else {
        HCP_Log(ERR, MODULE) << "restoreAdvancedparamters.restoreOption is: "
                             << restoreAdvancedparamters.restoreOption << HCPENDLOG;
        HCP_Log(ERR, MODULE) << "Get Restore Cover Policy failed. restoreAdvancedparamters is: "
                            << m_restoreJobInfo->extendInfo << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
};

int HostRestore::GetRepoInfo()
{
    HCP_Log(DEBUG, MODULE) << "Enter GetRepoInfo" << HCPENDLOG;
    
    if (m_restoreJobInfo->copies.empty()) {
        HCP_Log(ERR, MODULE) << "the copies info is empty" << HCPENDLOG;
        return Module::FAILED;
    }
    for (unsigned int i = 0; i < m_restoreJobInfo->copies[0].repositories.size(); i++) {
        if (m_restoreJobInfo->copies[0].repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFs = m_restoreJobInfo->copies[0].repositories[i];
        } else if (m_restoreJobInfo->copies[0].repositories[i].repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            m_dataFs = m_restoreJobInfo->copies[0].repositories[i];
        } else if (m_restoreJobInfo->copies[0].repositories[i].repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaFs = m_restoreJobInfo->copies[0].repositories[i];
        }
    }
    if (m_cacheFs.path.size() == 0 || m_dataFs.path.size() == 0 || m_metaFs.path.size() == 0) {
        HCP_Log(ERR, MODULE) << "Received info is wrong "
                             << "m_cacheFs.path.size() " << m_cacheFs.path.size()
                             << "m_dataFsIp.path.size() " << m_dataFs.path.size()
                             << "m_metaFs.path.size() " << m_metaFs.path.size() << HCPENDLOG;
        return Module::FAILED;
    }
    m_cacheFsPath = m_cacheFs.path[0];
    m_dataFsPath = m_dataFs.path[(m_numberOfSubTask++) % m_dataFs.path.size()];
    m_metaFsPath = m_metaFs.path[0];
    m_scanStatusPath = PluginUtils::PathJoin(m_cacheFsPath, SCANNER_STAT);
    m_metaOriPath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "all_meta");
    INFOLOG("cachePath: %s, dataPath: %s, metaPath: %s, scanStatusPath: %s", m_cacheFsPath.c_str(),
        m_dataFsPath.c_str(), m_metaFsPath.c_str(), m_scanStatusPath.c_str());

    HCP_Log(INFO, MODULE) << "Exit GetRepoInfo" << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::UntarDcachefiles(int volumeOrder) const
{
    // 恢复通过dcache生成控制文件
    string dcachZipFullPath = PluginUtils::PathJoin(m_currentMetaZipDirFullPathList[volumeOrder], METAFILE_ZIP_NAME);
#ifndef WIN32
    vector<string> output;
    vector<string> errOutput;
#if defined(_AIX) || defined(SOLARIS)
    string cmd = "cd " + m_currentMetaZipDirFullPathList[volumeOrder] +
        " && gunzip -c " + dcachZipFullPath + " | tar -xf - && cd -";
#else
    string cmd = "tar -zxf " + dcachZipFullPath + " -C " + m_currentMetaZipDirFullPathList[volumeOrder];
#endif
    HCP_Log(INFO, MODULE) << "the untar cmd is : " << cmd << HCPENDLOG;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "untar metafile.tar.gz failed! " << cmd
                             <<"the ret is: " << ret << HCPENDLOG;
        for (size_t i = 0; i < errOutput.size(); ++i) {
            HCP_Log(ERR, MODULE) << errOutput[i] << HCPENDLOG;
        }
        return Module::FAILED;
    }
#else
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " -y -aoa x " + PluginUtils::ReverseSlash(dcachZipFullPath) +
        " -o" + PluginUtils::ReverseSlash(m_currentMetaZipDirFullPathList[volumeOrder]);
    INFOLOG("compress cmd : %s", cmd.c_str());
    uint32_t errCode = 0;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! ret: %d, cmd : %s, error code: %d", ret, cmd.c_str(), errCode);
        return Module::FAILED;
    }
#endif
    HCP_Log(INFO, MODULE) << "uncompress metafile "<< dcachZipFullPath << "success." << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::UntarWholePackage(string zipPath, string targetPath) const
{
    // 恢复通过dcache生成控制文件
#ifndef WIN32
    vector<string> output;
    vector<string> errOutput;
#if defined(_AIX) || defined(SOLARIS)
    string cmd = "cd " + targetPath + " && gunzip -c " + zipPath + " | tar -xf - && cd -";
#else
    string cmd = "tar -zxf " + zipPath + " -C " + targetPath;
#endif
    HCP_Log(INFO, MODULE) << "the untar cmd is : " << cmd << HCPENDLOG;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        HCP_Log(ERR, MODULE) << "untar metafile.tar.gz failed! " << cmd
                             <<"the ret is: " << ret << HCPENDLOG;
        for (size_t i = 0; i < errOutput.size(); ++i) {
            HCP_Log(ERR, MODULE) << errOutput[i] << HCPENDLOG;
        }
        return Module::FAILED;
    }
#else
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " -y -aoa x " + PluginUtils::ReverseSlash(zipPath) + " -o" +
        PluginUtils::ReverseSlash(targetPath);
    INFOLOG("compress cmd : %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return Module::FAILED;
    }
#endif
    HCP_Log(INFO, MODULE) << "untar metafile.tar.gz success." << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::CreateSrcDir() const
{
    HCP_Log(DEBUG, MODULE) << "Enter CreateSrcDir." << HCPENDLOG;
    if (!PluginUtils::CreateDirectory(m_scanContrlFilePath)) {
        return Module::FAILED;
    }
    if (!PluginUtils::CreateDirectory(m_restoreContrlFilePath)) {
        return Module::FAILED;
    }
    if (!PluginUtils::CreateDirectory(m_restoreMetaPath)) {
        return Module::FAILED;
    }
    HCP_Log(DEBUG, MODULE) << "create scanContrlFilePath and restoreContrlFilePath success." << HCPENDLOG;
    return Module::SUCCESS;
}

int HostRestore::GetMetaZipDirList(string targetPath)
{
    HCP_Log(DEBUG, MODULE) << "Enter GetMetaZipDirList" << HCPENDLOG;
    vector<string> tempMetaZipDirList;
    if (!PluginUtils::GetDirListInDirectory(targetPath, tempMetaZipDirList)) {
        HCP_Log(ERR, MODULE) << "Get meta zip list failed, the m_restoreMetaPath: " << targetPath << HCPENDLOG;
        return Module::FAILED;
    }
    m_currentMetaZipDirFullPathList.clear();
    m_currentMetaZipDirNameList.clear();
    for (size_t i = 0; i < tempMetaZipDirList.size(); ++i) {
        string metaZipPath = tempMetaZipDirList[i];
        INFOLOG("Get meta zip path: %s", metaZipPath.c_str());
        if (metaZipPath.find("Volume") != string::npos) {
            if (metaZipPath.find("failed") != string::npos) {
                INFOLOG("skip failedVolume.");
                continue;
            }
            string dirPath = PluginUtils::PathJoin(targetPath, tempMetaZipDirList[i]);
            m_currentMetaZipDirNameList.push_back(tempMetaZipDirList[i]);
            m_currentMetaZipDirFullPathList.push_back(dirPath);
        }
    }
    return Module::SUCCESS;
}

int HostRestore::GenerateSubJobByDcacheAndFcache(int volumeOrder)
{
    HCP_Log(DEBUG, MODULE) << "Enter GenerateSubJobByDcacheAndFcache" << HCPENDLOG;
    if (UntarDcachefiles(volumeOrder) != Module::SUCCESS) {
        return Module::FAILED;
    }
    if (m_incrementalRestore) {
        return GeneralSubJobByLastControl(volumeOrder);
    }
    ScanConfig scanConfig;
    FillScanConfig(scanConfig, volumeOrder);
    SpecialHandelScanConfig(scanConfig, volumeOrder);
    if (AddFilterRule(scanConfig) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "failed to Add Filter Rule." << HCPENDLOG;
        return Module::FAILED;
    }

    if (StartToScan(scanConfig) != Module::SUCCESS) {
        return Module::FAILED;
    }
    if (MonitorScannerProgress(volumeOrder) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "generate resrore execute subjob failed, m_job: "
                             << m_restoreJobInfo->jobId << HCPENDLOG;
        return Module::FAILED;
    }

    CalcuScannerStatistic();

    if (m_scanner != nullptr) {
        m_scanner->Destroy();
        m_scanner.reset();
    }
    return Module::SUCCESS;
}

int HostRestore::GeneralSubJobByLastControl(int volumeOrder)
{
    // lastCtrl/sourcePrimaryVolume or lastCtrl/Volume_{ID} or lastCtrl/SubVolume_{ID}
    string lastControlDir = PluginUtils::PathJoin(GetLastCtrlPath(), m_currentMetaZipDirNameList[volumeOrder]);
    INFOLOG("General sub job from control dir: %s", lastControlDir.c_str());
    vector<string> tempFileList;
    vector<string> controlFileList;
    if (!PluginUtils::GetFileListInDirectory(lastControlDir, tempFileList)) {
        HCP_Log(ERR, MODULE) << "Get filelist for dir failed: " << m_scanContrlFilePath << HCPENDLOG;
        return Module::FAILED;
    }
    for (const auto& tempControlFile : tempFileList) {
        if (tempControlFile.find("delete_control_") != string::npos) {
            continue;
        }
        if (IsValidCtrlFile(tempControlFile)) {
            controlFileList.push_back(tempControlFile);
        }
    }
    if (controlFileList.empty()) {
        return Module::SUCCESS;
    }
    // 保留控制文件，再次增量恢复还需要使用
    if (GenerateRestoreExecuteSubJob(controlFileList, false) != Module::SUCCESS) {
        ERRLOG("Generate Restore sub job failed!");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

void HostRestore::CalcuScannerStatistic()
{
    HCP_Log(DEBUG, MODULE) << "Enter CalcuScannerStatistic" << HCPENDLOG;
    if (m_incrementalRestore) {
        BackupStatistic preBackupStats{};
        std::string path = PluginUtils::PathJoin(GetLastCtrlPath(), "statistic.json");
        std::string info;
        bool ret = PluginUtils::ReadFile(path, info);
        if ((!ret) || (!Module::JsonHelper::JsonStringToStruct(info, preBackupStats))) {
            return;
        }
        m_scannerStatistic.mTotDirsToBackup += preBackupStats.noOfDirCopied;
        m_scannerStatistic.mTotFilesToBackup += preBackupStats.noOfFilesCopied;
        m_scannerStatistic.mTotalSizeToBackup += preBackupStats.noOfBytesCopied;
    } else {
        ScanStatistics singleStatistic = m_scanner->GetStatistics();
        m_scannerStatistic.mScanDuration += singleStatistic.mScanDuration;
        m_scannerStatistic.mTotDirs += singleStatistic.mTotDirs;
        m_scannerStatistic.mTotFiles += singleStatistic.mTotFiles;
        m_scannerStatistic.mTotalSize += singleStatistic.mTotalSize;
        m_scannerStatistic.mTotDirsToBackup += singleStatistic.mTotDirsToBackup;
        m_scannerStatistic.mTotFilesToBackup += singleStatistic.mTotFilesToBackup;
        m_scannerStatistic.mTotFilesDeleted += singleStatistic.mTotFilesDeleted;
        m_scannerStatistic.mTotDirsDeleted += singleStatistic.mTotDirsDeleted;
        m_scannerStatistic.mTotalSizeToBackup += singleStatistic.mTotalSizeToBackup;
        m_scannerStatistic.mTotalControlFiles += singleStatistic.mTotalControlFiles;
        m_scannerStatistic.mTotFailedDirs += singleStatistic.mTotFailedDirs;
        m_scannerStatistic.mTotFailedFiles += singleStatistic.mTotFailedFiles;
    }
    return;
}

int HostRestore::StartToScan(ScanConfig& scanConfig)
{
    HCP_Log(DEBUG, MODULE) << "Enter StartToScan" << HCPENDLOG;
    m_scanner = ScanMgr::CreateScanInst(scanConfig);
    if (m_scanner == nullptr) {
        HCP_Log(ERR, MODULE) << "failed to creat m_scanner." << HCPENDLOG;
        return Module::FAILED;
    }
    if (m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Start scanner instance failed!" << HCPENDLOG;
        return Module::FAILED;
    }
    HCP_Log(DEBUG, MODULE) << "Start scanner instance success!" << HCPENDLOG;
    return Module::SUCCESS;
}

void HostRestore::GeneratedCopyCtrlFileCb(void* /* usrData */, string ctrlFile)
{
    DBGLOG("Generated copy ctrl file: %s", ctrlFile.c_str());
}

void HostRestore::GeneratedHardLinkCtrlFileCb(void* /* usrData */, string ctrlFile)
{
    DBGLOG("Generated hard link ctrl file: %s", ctrlFile.c_str());
}

std::string HostRestore::GetDiffDir(std::string& path)
{
    int index1 = path.find_last_of(dir_sep.c_str());
    int index2 = path.substr(0, index1).find_last_of(dir_sep.c_str());
    return path.substr(index2 + 1, path.length() - index2);
}

void HostRestore::FillScanConfig(ScanConfig& scanConfig, int volumeOrder)
{
    HCP_Log(DEBUG, MODULE) << "Enter FillScanConfig" << HCPENDLOG;

    scanConfig.reqID  = PluginUtils::GenerateHash(m_restoreJobInfo->jobId);
    scanConfig.jobId = m_restoreJobInfo->jobId;
  
    /* config meta path */
    scanConfig.curDcachePath  = m_currentMetaZipDirFullPathList[volumeOrder];
    scanConfig.metaPathForCtrlFiles = PluginUtils::PathJoin(m_scanContrlFilePath, GetDiffDir(scanConfig.curDcachePath));

    PluginUtils::CreateDirectory(scanConfig.metaPathForCtrlFiles);
    
    scanConfig.scanType = ScanJobType::CONTROL_GEN;
    scanConfig.scanIO = IOEngine::DEFAULT;
    scanConfig.generatorIsFull = true;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.scanSparseFile = true;

    scanConfig.maxOpendirReqCount = NUMBER4000;

    /* 记录线程数 */
    scanConfig.maxCommonServiceInstance = 1;

    scanConfig.usrData = (void*)this;
    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;

    scanConfig.scanCopyCtrlFileSize = Module::ConfigReader::getInt("FilePluginConfig", "PosixCopyCtrlFileSize");
    scanConfig.scanCtrlMaxDataSize = Module::ConfigReader::getString("FilePluginConfig", "PosixMaxCopyCtrlDataSize");
    scanConfig.scanCtrlMinDataSize = Module::ConfigReader::getString("FilePluginConfig", "PosixMinCopyCtrlDataSize");
    scanConfig.scanCtrlFileTimeSec = NUMBER5;
    scanConfig.scanCtrlMaxEntriesFullBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMaxCopyCtrlEntriesFullBackup");
    scanConfig.scanCtrlMaxEntriesIncBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMaxCopyCtrlEntriesIncBackup");
    scanConfig.scanCtrlMinEntriesFullBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMinCopyCtrlEntriesFullBackup");
    scanConfig.scanCtrlMinEntriesIncBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMinCopyCtrlEntriesIncBackup");
    scanConfig.scanMetaFileSize = NUMBER1024 * NUMBER1024 * NUMBER1024; // one GB

    scanConfig.maxWriteQueueSize = NUMBER10000;
    scanConfig.triggerTime = PluginUtils::GetCurrentTimeInSeconds();
    HCP_Log(DEBUG, MODULE) << "EXIT FillScanConfig" << HCPENDLOG;
}

void HostRestore::SpecialHandelScanConfig(ScanConfig& scanConfig, int volumeOrder)
{
    HCP_Log(DEBUG, MODULE) << "Enter SpecialHandelScanConfig" << HCPENDLOG;
    if (m_aggregateRestore) {
        int previousVolumeOrder = 0;
        if (ExistPreviousVolume(volumeOrder, previousVolumeOrder)) {
            HCP_Log(DEBUG, MODULE) << "Generating Control Files Through Differences" << HCPENDLOG;
            scanConfig.generatorIsFull = false;
            scanConfig.prevDcachePath  = m_previousMetaZipDirList[previousVolumeOrder];
        } else {
            HCP_Log(DEBUG, MODULE) << "Directly generate control files in full mode." << HCPENDLOG;
            scanConfig.generatorIsFull = true;
        }
    }
}

bool HostRestore::ExistPreviousVolume(int volumeOrder, int& previousVolumeOrder) const
{
    HCP_Log(DEBUG, MODULE) << "Enter ExistPreviousVolume" << HCPENDLOG;
    string dirName = m_currentMetaZipDirNameList[volumeOrder];
    for (uint32_t i = 0; i < m_previousMetaZipDirList.size(); ++i) {
        string dirFullPath = m_previousMetaZipDirList[i];
        if (dirFullPath.find(dirName) != string::npos) {
            previousVolumeOrder = i;
            return true;
        }
    }
    return false;
}

int HostRestore::AddFilterRule(ScanConfig& scanConfig)
{
    HCP_Log(DEBUG, MODULE) << "Enter AddFilterRule" << HCPENDLOG;
    // 不是细粒度恢复，不用添加过滤规则
    if (!m_fineGrainedRestore) {
        return Module::SUCCESS;
    }
    vector<string> dirFilterRule;
    vector<string> fileFilterRule;
    for (size_t i = 0; i < m_restoreJobInfo->restoreSubObjects.size(); ++i) {
        string path = m_restoreJobInfo->restoreSubObjects[i].name;
        // windows filter path format to lowwer
#ifdef WIN32
            transform(path.begin(), path.end(), path.begin(), ::tolower);
#endif
        // is dir or file , the last char is not '/' is file
        if (path.back() != SLASH_CH) { // file
            HCP_Log(INFO, MODULE) << "file filter rule: " << path << HCPENDLOG;
            fileFilterRule.push_back(path);
        } else { // dir
            HCP_Log(INFO, MODULE) << "dir filter rule: " << path << HCPENDLOG;
            dirFilterRule.push_back(path);
        }
    }
    scanConfig.dCtrlFltr = dirFilterRule;
    scanConfig.fCtrlFltr = fileFilterRule;
    return Module::SUCCESS;
}

int HostRestore::GetControlFileList(string controlFilePath, vector<string>&  contrlFileList)
{
    HCP_Log(DEBUG, MODULE) << "Enter get GetControlFileList" << HCPENDLOG;
    string preControlPath = controlFilePath;
    vector<string> tempFileList;
    if (!PluginUtils::GetFileListInDirectory(preControlPath, tempFileList)) {
        HCP_Log(ERR, MODULE) << "Get filelist for dir failed: " << m_scanContrlFilePath << HCPENDLOG;
        return Module::FAILED;
    }
    for (size_t i = 0; i < tempFileList.size(); ++i) {
        INFOLOG("Get controlfile: %s", tempFileList[i].c_str());
        if (IsValidCtrlFile(tempFileList[i])) {
            contrlFileList.push_back(tempFileList[i]);
        }
    }
    
    return Module::SUCCESS;
}

int HostRestore::GenerateRestoreExecuteSubJob(vector<string> contrlFileList, bool isDelControlFile)
{
    HCP_Log(DEBUG, MODULE) << "start General SubJob By ControlFile" << HCPENDLOG;
    vector<SubJob> subJobList;
    vector<string> TempContrlFileList;
    for (size_t i = 0; i < contrlFileList.size(); i++) {
        string contrlFileFullPath = contrlFileList[i];
        int pos = contrlFileFullPath.find_last_of(dir_sep.c_str());
        string fileName = std::string(contrlFileFullPath.substr(pos + 1));
        std::string contrlFileParentPath = contrlFileFullPath.substr(0, pos);
        // 增量恢复的控制文件位于meta仓，恢复期望的是{0}/sourcePrimaryVolume
        // 而GetDiffDir获取到的是lastCtrl/sourcePrimaryVolume，需特殊适配
        string metaPath = !m_incrementalRestore ? PluginUtils::PathJoin(m_metaOriPath, GetDiffDir(contrlFileParentPath))
            : PluginUtils::PathJoin(m_restoreMetaPath, PluginUtils::GetFileName(contrlFileParentPath));
        string restoreControlfileFullpath = PluginUtils::PathJoin(m_restoreContrlFilePath, fileName);
        INFOLOG("scan contrlFileFullPath is: %s, restore ControlfileFullpath: %s, metaFile:%s",
            contrlFileFullPath.c_str(), restoreControlfileFullpath.c_str(), metaPath.c_str());
        if (!PluginUtils::CopyFile(contrlFileFullPath, restoreControlfileFullpath)) {
            HCP_Log(ERR, MODULE) << "restoreControlfileFullpath is." << restoreControlfileFullpath << HCPENDLOG;
            HCP_Log(ERR, MODULE) << "Copy control file failed." << HCPENDLOG;
            return Module::FAILED;
        }
        SubJob subJob;

        if (InitSubJobInfo(subJob, restoreControlfileFullpath, metaPath) != Module::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Init subJOb failed by contrlFileFullPath" << HCPENDLOG;
            return Module::FAILED;
        }
        subJobList.push_back(subJob);
        TempContrlFileList.push_back(contrlFileFullPath);

        // We create 10 Jobs at a time. If 10 is not accumulated, continue
        if (subJobList.size() % NUMBER10 != 0)
            continue;

        if (ReportSubJobToAgent(subJobList, TempContrlFileList, isDelControlFile) != Module::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Exit CreateSubTasksFromCtrlFile, Create subtask failed" << HCPENDLOG;
            return Module::FAILED;
        }
    }
    if (ReportSubJobToAgent(subJobList, TempContrlFileList, isDelControlFile) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Exit CreateSubTasksFromCtrlFile, Create subtask failed" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::GetCopyOrder(const std::string& metaPath)
{
    int index1 = metaPath.find_last_of(dir_sep.c_str());
    int index2 = metaPath.substr(0, index1).find_last_of(dir_sep.c_str());
    return stoi(metaPath.substr(index2 + 1, index2 - index1 - 1));
}

int HostRestore::InitSubJobInfo(SubJob &subJob, const string restoreControlfileFullpath, const std::string& metaPath)
{
    HCP_Log(DEBUG, MODULE) << "Enter InitSubJobInfo" <<HCPENDLOG;
    string subJobName;
    int subJobType = 0;
    int subJobPrio = 0;
    string restoreSubJobInfoStr;
    SubJobInfo restoreSubJobInfo;
    // 通过metaPath路径解析出副本顺序，用于计算子任务优先级和meta、data仓路径
    m_orderNumberForCopies = GetCopyOrder(metaPath);
    GetSubJobInfoByFileName(restoreControlfileFullpath, subJobName, subJobType, subJobPrio);

    restoreSubJobInfo.controlFileName = restoreControlfileFullpath;
    restoreSubJobInfo.dcacheAndFcachePath = metaPath;
    restoreSubJobInfo.subJobType = subJobType;
    if (m_aggregateRestore) {
        DBGLOG("InitSubJobInfo for aggregate restore, copy order: %d", m_orderNumberForCopies);
        restoreSubJobInfo.dataCachePath = m_dataFsPathList[m_orderNumberForCopies];
        restoreSubJobInfo.metaFilePath = m_metaFsPathList[m_orderNumberForCopies];
        restoreSubJobInfo.copyOrder = m_orderNumberForCopies;
    } else {
        restoreSubJobInfo.dataCachePath = m_dataFsPath;
        restoreSubJobInfo.metaFilePath = m_metaFsPath;
    }

    if (!Module::JsonHelper::StructToJsonString(restoreSubJobInfo, restoreSubJobInfoStr)) {
        HCP_Log(ERR, MODULE) << "Convert to json failed for subJob info: " << HCPENDLOG;
        return Module::FAILED;
    }
    HCP_Log(INFO, MODULE) << "restoreSubJobInfoStr is : "  << restoreSubJobInfoStr << HCPENDLOG;
    subJob.__set_jobId(m_restoreJobInfo->jobId);
    subJob.__set_jobName(subJobName);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(restoreSubJobInfoStr);
    subJob.__set_jobPriority(subJobPrio);
    INFOLOG("ignoreFailed: %d", GetIgnoreFailed());
    subJob.__set_ignoreFailed(GetIgnoreFailed());
    return Module::SUCCESS;
}

void HostRestore::GetSubJobInfoByFileName(const string &fileName, string &subTaskName,
                                          int &subTaskType, int &subTaskPrio)
{
    if (m_idGenerator == nullptr) {
        InitIdGenerator();
    }
    string uniqueId = to_string(m_idGenerator->GenerateId());
    if (fileName.find(HARDLINK_CTRL_PREFIX) != string::npos) {
        subTaskName = "HostRestore_HardlinkCtrlFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_HARDLINK_PHASE_PRIO;
    } else if (fileName.find(MTIME_CTRL_PREFIX) != string::npos) {
        subTaskName = "HostRestore_DirMtimeFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_DIRMTIME_PHASE_PRIO;
    } else if (fileName.find(DELETE_CTRL_PREFIX) != string::npos) {
        subTaskName = "HostRestore_DelFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_DELETE_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_DELETE_PHASE_PRIO;
    } else if (fileName.find(CONTROL_CTRL_PREFIX) != string::npos) {
        subTaskName = "HostRestore_CtrlFile_" + uniqueId;
        subTaskType = SUBJOB_TYPE_DATACOPY_COPY_PHASE;
        subTaskPrio = SUBJOB_TYPE_DATACOPY_COPY_PHASE_PRIO;
    } else {
        HCP_Log(ERR, MODULE) << "Get SubJob Type By FileName failed" << HCPENDLOG;
    }
    if (m_aggregateRestore) {
        subTaskPrio = m_orderNumberForCopies * SUBJOB_TYPE_AGGREGATE_BASE_PRIO + subTaskPrio;
    }
    return;
}

int HostRestore::GenerateCheckSubJob()
{
    INFOLOG("Enter GenerateCheckSubJob");
    string restoreSubJobInfoStr;
    SubJobInfo restoreSubJobInfo;
    restoreSubJobInfo.subJobType = SUBJOB_TYPE_CHECK_SUBJOB_PHASE;
    if (!Module::JsonHelper::StructToJsonString(restoreSubJobInfo, restoreSubJobInfoStr)) {
        ERRLOG("Convert to json failed for subJob info");
        return Module::FAILED;
    }
    int subJobPrio = SUBJOB_TYPE_CHECK_SUBJOB_PHASE_PRIO;
    SubJob subJob;
    subJob.__set_jobId(m_restoreJobInfo->jobId);
    subJob.__set_jobName(SUBJOB_TYPE_CHECK_SUBJOB_JOBNAME);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(restoreSubJobInfoStr);
    subJob.__set_jobPriority(subJobPrio);
    subJob.__set_ignoreFailed(true);

    vector<SubJob> subJobList;
    subJobList.push_back(subJob);
    ActionResult ret;
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (ret.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::GenerateTearDownSubJob()
{
    HCP_Log(DEBUG, MODULE) << "Enter GenerateTearDownSubJob" << HCPENDLOG;

    string restoreSubJobInfoStr;
    SubJobInfo restoreSubJobInfo;
    restoreSubJobInfo.subJobType = SUBJOB_TYPE_TEARDOWN_PHASE;
    if (!Module::JsonHelper::StructToJsonString(restoreSubJobInfo, restoreSubJobInfoStr)) {
        HCP_Log(ERR, MODULE) << "Convert to json failed for subJob info: " << HCPENDLOG;
        return Module::FAILED;
    }
    int subJobPrio;
    if (m_aggregateRestore) {
        subJobPrio = (m_numberCopies + 1) * SUBJOB_TYPE_AGGREGATE_BASE_PRIO + SUBJOB_TYPE_TEARDOWN_PHASE_PRIO;
    } else {
        subJobPrio = SUBJOB_TYPE_TEARDOWN_PHASE_PRIO;
    }
    SubJob subJob;
    subJob.__set_jobId(m_restoreJobInfo->jobId);
    subJob.__set_jobName("FINAL_REPORT_STATISTIC");
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(restoreSubJobInfoStr);
    subJob.__set_jobPriority(subJobPrio);
    INFOLOG("ignoreFailed: %d", GetIgnoreFailed());
    subJob.__set_ignoreFailed(GetIgnoreFailed());

    vector<SubJob> subJobList;
    subJobList.push_back(subJob);
    ActionResult ret;
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (ret.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::ReportSubJobToAgent(vector<SubJob> &subJobList, vector<string> &contrlFileList, bool isDelControlFile)
{
    HCP_Log(DEBUG, MODULE) << "Enter ReportSubJobToAgent" << HCPENDLOG;

    if (contrlFileList.empty()) {
        return Module::SUCCESS;
    }
    ActionResult ret;
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        INFOLOG("AddNewJob, retry : %d", retryTimes);
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (ret.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return Module::FAILED;
    }
    if (isDelControlFile) {
        for (size_t i = 0; i < contrlFileList.size(); i++) {
            PluginUtils::RemoveFile(contrlFileList[i]);
        }
    }

    subJobList.clear();
    contrlFileList.clear();
    return Module::SUCCESS;
}

int HostRestore::StartToRestore()
{
    HCP_Log(DEBUG, MODULE) << "Enter StartToRestore." << HCPENDLOG;
    BackupParams backupParams;
    FillRestoreConfig(backupParams);
    if (SpecialDealRestoreConfig(backupParams) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Special deal restore config failed" << HCPENDLOG;
        return Module::FAILED;
    }
    m_backup = FS_Backup::BackupMgr::CreateBackupInst(backupParams);
    if (m_backup == nullptr) {
        HCP_Log(ERR, MODULE) << "Create backup instance failed" << HCPENDLOG;
        return Module::FAILED;
    }

    if (m_backup->Enqueue(m_subJobPathsInfo.controlFileName) != BackupRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE) << "enqueue backup instance failed" << HCPENDLOG;
        return Module::FAILED;
    }
    if (m_backup->Start() != BackupRetCode::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Start backup task failed" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

#ifdef WIN32
void HostRestore::FillRestoreConfigForWin32(
    BackupParams& backupParams,
    const std::string& resourcePath,
    const std::string& destinationPath)
{
    backupParams.srcEngine = BackupIOEngine::WIN32_IO;
    backupParams.dstEngine = BackupIOEngine::WIN32_IO;
    HostBackupAdvanceParams win32BackupAdvanceParams{};
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(win32BackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(win32BackupAdvanceParams);
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = resourcePath;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = destinationPath;
    backupParams.scanAdvParams.metaFilePath = m_subJobPathsInfo.dcacheAndFcachePath;
    INFOLOG("backupParams.srcAdvParams.dataPath: %s, backupParams.dstAdvParams.dataPath: %s, "
        "backupParams.scanAdvParams.metaFilePath: %s", resourcePath.c_str(), destinationPath.c_str(),
        backupParams.scanAdvParams.metaFilePath.c_str());
}
#endif

void HostRestore::FillRestoreConfigForPosix(
    BackupParams& backupParams,
    const std::string& resourcePath,
    const std::string& destinationPath)
{
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;
    HostBackupAdvanceParams posixBackupAdvanceParams {};
    posixBackupAdvanceParams.threadNum = Module::ConfigReader::getInt("FilePluginConfig", "PosixReaderThreadNum");
    posixBackupAdvanceParams.maxMemory = Module::ConfigReader::getInt("FilePluginConfig", "PosixMaxMemory");
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    posixBackupAdvanceParams.threadNum = Module::ConfigReader::getInt("FilePluginConfig", "PosixWriterThreadNum");
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    backupParams.scanAdvParams.metaFilePath = m_subJobPathsInfo.dcacheAndFcachePath;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = resourcePath;
    dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = destinationPath;

    HCP_Log(INFO, MODULE) << "srcAdvParams threadNum is: "
                          << dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->threadNum
                          << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "srcAdvParams maxMemory is: "
                          << dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->maxMemory
                          << HCPENDLOG;
    HCP_Log(INFO, MODULE) << "dstAdvParams threadNum is: "
                          << dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->threadNum
                          << HCPENDLOG;
}

void HostRestore::FillRestoreConfig(BackupParams& backupParams)
{
    HCP_Log(DEBUG, MODULE) << "Enter FillRestoreConfig." << HCPENDLOG;
    size_t subJobRequestId = PluginUtils::GenerateHash(m_restoreJobInfo->jobId + m_subJobInfo->subJobId);
    backupParams.backupType = BackupType::RESTORE;
    backupParams.phase = m_backupPhase;
    string resourcePath = m_subJobPathsInfo.dataCachePath;
    string destinationPath = m_restorePath;
#ifdef WIN32
    FillRestoreConfigForWin32(backupParams, resourcePath, destinationPath);
#else
    FillRestoreConfigForPosix(backupParams, resourcePath, destinationPath);
#endif

    CommonParams commonParams {};
#ifdef WIN32
    commonParams.writeExtendAttribute = false;
#else
    commonParams.writeExtendAttribute = true;
#endif
    commonParams.metaPath = m_subJobPathsInfo.metaFilePath;
    commonParams.jobId = m_restoreJobInfo->jobId;
    commonParams.subJobId = m_subJobInfo->subJobId;
    commonParams.reqID = subJobRequestId;
    commonParams.failureRecordRootPath = m_failureRecordRoot;
    commonParams.restoreReplacePolicy = m_coveragePolicy;
    commonParams.skipFailure = true;
    commonParams.writeSparseFile = true; // if have sparse info in metafile then restore with sparse way
    commonParams.writeMeta = true;
    commonParams.writeAcl = true;

#ifdef SOLARIS
    if (!OsIdentifier::CheckSolarisVersionCompatible(m_metaFsPath)) {
        WARNLOG("copy imcompatible, disable writeAcl");
        commonParams.writeAcl = false;
    }
#endif

    backupParams.commonParams = commonParams;
    HCP_Log(INFO, MODULE) << "resourcePath is: " << resourcePath <<
        "destinationPath is:" << destinationPath <<
        "dcacheAndFcachePath is:" << m_subJobPathsInfo.dcacheAndFcachePath <<
        "metaFilePath is:" << m_subJobPathsInfo.metaFilePath <<
        "subJobRequestId:" << subJobRequestId << HCPENDLOG;
}

int HostRestore::SpecialDealRestoreConfig(BackupParams& backupParams)
{
    HCP_Log(DEBUG, MODULE) << "Enter SpecialDealRestoreConfig." << HCPENDLOG;
    if (m_aggregateRestore) {
        if (GetSpecialConfigForRestore() != Module::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Get Special Config For Aggregate failed." << HCPENDLOG;
            return Module::FAILED;
        }
        backupParams.commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
        backupParams.commonParams.maxAggregateFileSize = m_maxSizeAfterAggregate;
        backupParams.commonParams.maxFileSizeToAggregate = m_maxSizeToAggregate;
        backupParams.commonParams.aggregateThreadNum =
            Module::ConfigReader::getInt("FilePluginConfig", "PosixAggregatorThreadNum");
        HCP_Log(DEBUG, MODULE) << "commonParams aggregateThreadNum is: "
                          << backupParams.commonParams.aggregateThreadNum << HCPENDLOG;
    } else {
        backupParams.commonParams.backupDataFormat = BackupDataFormat::NATIVE;
        backupParams.commonParams.maxBufferCnt = NUMBER10;
        backupParams.commonParams.maxBufferSize = NUMBER10 * NUMBER1024; // 10kb
        backupParams.commonParams.maxErrorFiles = NUMBER100;
    }
    return Module::SUCCESS;
}

void HostRestore::ReportCopyPhaseStart()
{
    // 通过从ShareResource中读取HostTaskInfo，startTime为零表示未上报开始恢复
    string speedId = m_restoreJobInfo->jobId + SPEED_STR;
    HostTaskInfo hostTaskInfo;
    ShareResourceManager::GetInstance().Wait(ShareResourceType::GENERAL, speedId);
    if (!ShareResourceManager::GetInstance().QueryResource(ShareResourceType::GENERAL, speedId, hostTaskInfo)) {
        ERRLOG("get restore start time failed.");
        ShareResourceManager::GetInstance().Signal(ShareResourceType::GENERAL, speedId);
        return;
    }
    if (hostTaskInfo.startTime != 0) {
        DBGLOG("Not first sub restore job, don't need report!");
        ShareResourceManager::GetInstance().Signal(ShareResourceType::GENERAL, speedId);
        m_startTime = hostTaskInfo.startTime;  // set start time to calculate speed
        return;
    }
    // startTime等于0，上报恢复开始
    string label = "file_plugin_host_restore_data_start_label";
    ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, label);
    hostTaskInfo.startTime = PluginUtils::GetCurrentTimeInSeconds();
    m_startTime = hostTaskInfo.startTime;  // set start time to calculate speed
    if (!ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::GENERAL, speedId, hostTaskInfo)) {
        ERRLOG("Update hostTaskInfo failed.");
    }
    ShareResourceManager::GetInstance().Signal(ShareResourceType::GENERAL, speedId);
}

HostCommonService::MONITOR_BACKUP_RES_TYPE HostRestore::MonitorRestoreJobStatus()
{
    HCP_Log(DEBUG, MODULE) << "Enter MonitorRestoreJobStatus." << HCPENDLOG;
    ReportCopyPhaseStart();
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    BackupStats tmpStats;
    time_t statLastUpdateTime = PluginUtils::GetCurrentTimeInSeconds();
    while (true) {
        m_backupStatus = m_backup->GetStatus();
        HCP_Log(INFO, MODULE) << "restoreSubJobStatus:" << static_cast<int>(m_backupStatus) << HCPENDLOG;
        tmpStats = m_backup->GetStats();
        if (m_backupStats != tmpStats) {
            statLastUpdateTime = PluginUtils::GetCurrentTimeInSeconds();
            INFOLOG("backup statistics last update time: %ld", statLastUpdateTime);
            m_backupStats = tmpStats;
        } else if ((m_backupStatus == BackupPhaseStatus::INPROGRESS) &&
            (tmpStats.noOfFilesCopied + tmpStats.noOfFilesFailed != tmpStats.noOfFilesToBackup) &&
            (PluginUtils::GetCurrentTimeInSeconds() - statLastUpdateTime >
            Module::ConfigReader::getInt("FilePluginConfig", "BACKUP_STUCK_TIME"))) {
            UpdateSubBackupStats(true);
            HandleMonitorStuck(jobStatus);
            return MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_NEEDRETRY;
        }
        if (!IsBackupStatusInprogress(jobStatus)) {
            break;
        }
        if (IsAbortJob()) {
            HCP_Log(INFO, MODULE) << "Backup - Abort is invocked for taskid: " << m_jobId
                << ", subtaskid: " << m_jobId << HCPENDLOG;
            if (BackupRetCode::SUCCESS != m_backup->Abort()) {
                HCP_Log(ERR, MODULE) << "backup Abort is failed" << HCPENDLOG;
            }
        }
        ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0));
        Module::SleepFor(std::chrono::seconds(EXECUTE_SUBTASK_MONITOR_DUR_IN_SEC));
    }
    UpdateSubBackupStats(false);
    HCP_Log(DEBUG, MODULE) << "Exit Monitor Restore." << HCPENDLOG;
    if (jobStatus != SubJobStatus::COMPLETED) {
        return MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_FAILED;
    }
    // 备份完成计算速度，防止出现任务完成未计算速度的情况
    BackupStatistic mainBackupStats;
    CalcuSpeed(mainBackupStats, m_startTime);
    return MONITOR_BACKUP_RES_TYPE::MONITOR_BACKUP_RES_TYPE_SUCCESS;
}

void HostRestore::CalcuSpeed(BackupStatistic& mainBackupStats, time_t startTime)
{
    if (CalcuMainBackupStats(mainBackupStats)) {
        time_t backDuration = PluginUtils::GetCurrentTimeInSeconds() - startTime;
        uint64_t dataInKB = mainBackupStats.noOfBytesCopied / NUMBER1024;
        if (backDuration != 0) {
            m_jobSpeed = dataInKB / backDuration;
        }
    }
}

void HostRestore::HandleMonitorStuck(SubJobStatus::type &jobStatus)
{
    WARNLOG("backup statistic has not been update for 300s");
    if (BackupRetCode::SUCCESS != m_backup->Abort()) {
        HCP_Log(ERR, MODULE) << "backup Abort is failed" << HCPENDLOG;
    }
    jobStatus = SubJobStatus::COMPLETED;
}

bool HostRestore::UpdateSubBackupStats(bool forceComplete)
{
    if (m_jobId.empty()) {
        DBGLOG("UpdateBackupTaskStats - subJobId is empty, main jobId: %s", m_jobId.c_str());
        return true;
    }
    BackupStats subBackupStats = m_backup->GetStats();
    if (forceComplete) {
        // 目录阶段计算目录的失败数
        if (m_backup->m_backupParams.phase == BackupPhase::DIR_STAGE) {
            subBackupStats.noOfDirFailed = subBackupStats.noOfDirToBackup - subBackupStats.noOfDirCopied;
        } else {
            subBackupStats.noOfFilesFailed = subBackupStats.noOfFilesToBackup - subBackupStats.noOfFilesCopied;
        }
    }
    SerializeBackupStats(subBackupStats, m_subBackupStats);
    ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, m_jobId);
    bool ret = ShareResourceManager::GetInstance().UpdateResource(
        ShareResourceType::BACKUP, m_jobId, m_subBackupStats);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, m_jobId);
    if (!ret) {
        ERRLOG("Update sub job stats failed, jobId: %s, sub jobId: %s",
            m_restoreJobInfo->jobId.c_str(), m_jobId.c_str());
        return false;
    }
    return true;
}

void HostRestore::UpdateSpeedAndReport()
{
    // 优先校验，如果不能上报，则直接退出，否则子任务抢占读取子任务状态，拉远场景超时
    if (!ShareResourceManager::GetInstance().CanReportStatToPM(m_restoreJobInfo->jobId)) {
        // no need to report main backup stats, keepalive is required.
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, "");
        return;
    }
    BackupStatistic mainBackupStats;
    CalcuSpeed(mainBackupStats, m_startTime);
    ReportBackupRunningStatus(mainBackupStats);
}

bool HostRestore::CalcuMainBackupStats(BackupStatistic& mainBackupStats) const
{
    // 通过遍历累加子任务数据得到总的数据
    vector<string> statsList;
    string statisticsPath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "statistic");
    if (!PluginUtils::GetFileListInDirectory(statisticsPath, statsList)) {
        ERRLOG("Get backup stats list failed, jobId: %s", m_restoreJobInfo->jobId.c_str());
        return false;
    }
    BackupStatistic subStats;
    for (const string& path : statsList) {
        string::size_type pos = path.find(BACKUP_KEY_SUFFIX);
        if (pos == string::npos) {
            continue;
        }
        string subJobId = path.substr(statisticsPath.length() + NUMBER1, m_jobId.length());
        DBGLOG("UpdateMainBackupStats, path: %s, jobId: %s, subJobId: %s",
            path.c_str(), m_restoreJobInfo->jobId.c_str(), subJobId.c_str());
        ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, subJobId);
        bool ret = ShareResourceManager::GetInstance().QueryResource(path, subStats);
        ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, subJobId);
        DBGLOG("UpdateMainBackupStats, noOfBytesCopied: %llu, speed: %llu, main: %llu, speed: %llu",
            subStats.noOfBytesCopied, subStats.backupspeed,
            mainBackupStats.noOfBytesCopied, mainBackupStats.backupspeed);
        if (!ret) {
            ERRLOG("Query failed, jobId: %s, subJobId: %s, path: %s",
                m_restoreJobInfo->jobId.c_str(), path.c_str(), subJobId.c_str());
            return false;
        }
        mainBackupStats = CalcuSumStructBackupStatistic(mainBackupStats, subStats);
    }
    return true;
}

bool HostRestore::ReportBackupRunningStatus(BackupStatistic& mainBackupStats)
{
    if (mainBackupStats.noOfBytesCopied != 0) {
        DBGLOG("ReportBackupRunningStatus, noOfBytesCopied: %d", mainBackupStats.noOfBytesCopied);
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0,
                             "file_plugin_host_restore_data_inprogress_label",
                             to_string(mainBackupStats.noOfDirCopied),
                             to_string(mainBackupStats.noOfFilesCopied),
                             PluginUtils::FormatCapacity(mainBackupStats.noOfBytesCopied));
        return true;
    } else {
        ReportJobMoreDetails(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0, "");
        return false;
    }
}

template<typename... Args>
bool HostRestore::ReportJobMoreDetails(const JobLogLevel::type &logLevel, SubJobStatus::type jobStatus,
                                       int32_t jobProgress, string logLabel,  Args... logArgs)
{
    SubJobDetails subJobDetails {};
    ActionResult result {};
    vector<LogDetail> logDetailList;
    LogDetail logDetail{};

    if (logLabel != "") {
        AddLogDetail(logDetail, logLabel, logLevel, logArgs...);
    }
    INFOLOG("Enter ReportJobDetails: JobId: %s, jobStatus: %d, logLabel: %s, speed: %d",
            m_jobId.c_str(), static_cast<int>(jobStatus), logLabel.c_str(), m_jobSpeed);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, jobProgress, m_jobSpeed, jobStatus);
    if (result.code != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Report job details to agent failed: " << result.code <<  HCPENDLOG;
        return false;
    }
    return true;
}

int HostRestore::InitialRestoreLocalProgressInfo(string jobId) const
{
    HCP_Log(DEBUG, MODULE) << "Enter InitialRestoreLocalProgressInfo" << HCPENDLOG;
    BackupStatistic backupStatistic;
    string resourcePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "statistic");
    ShareResourceManager::GetInstance().SetResourcePath(resourcePath, jobId);
    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::BACKUP,
                                                                jobId, backupStatistic);
    if (!ret) {
        HCP_Log(ERR, MODULE) << "Initial Local Progress Info failed." << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::InitialReportSpeedInfo() const
{
    // 用于保存恢复任务开始时间，计算速度
    HCP_Log(DEBUG, MODULE) << "Enter InitialReportSpeedInfo" << HCPENDLOG;
    BackupStatistic backupStatistic;
    string resourcePath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId, "statistic");
    string speedId = m_restoreJobInfo->jobId + SPEED_STR;
    HostTaskInfo hostTaskInfo;
    ShareResourceManager::GetInstance().SetResourcePath(resourcePath, speedId);
    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::GENERAL,
                                                                speedId, hostTaskInfo);
    if (!ret) {
        HCP_Log(ERR, MODULE) << "Initial Local Progress Info failed." << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::DeleteReportSpeedInfo() const
{
    HCP_Log(DEBUG, MODULE) << "Enter DeleteReportSpeedInfo" << HCPENDLOG;
    string speedId = m_restoreJobInfo->jobId + SPEED_STR;
    bool ret = ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::GENERAL, speedId);
    if (!ret) {
        HCP_Log(ERR, MODULE) << "Delete report speed Info failed." << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostRestore::GetCacheRepositoryPath()
{
    HCP_Log(DEBUG, MODULE) << "Enter GetCacheRepositoryPath" << HCPENDLOG;
    int lastCopyNumber = m_restoreJobInfo->copies.size() - 1;
    for (uint32_t i = 0; i < m_restoreJobInfo->copies[lastCopyNumber].repositories.size(); i++) {
        if (m_restoreJobInfo->copies[0].repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFsPath = m_restoreJobInfo->copies[lastCopyNumber].repositories[i].path[0];
            HCP_Log(INFO, MODULE) << "Get cache repository path suucessful! the path is "
                                 << m_cacheFsPath << HCPENDLOG;
            return Module::SUCCESS;
        }
    }
    HCP_Log(ERR, MODULE) << "Failed  get cache repository path" << HCPENDLOG;
    return Module::FAILED;
}

void HostRestore::KeepPluginAlive()
{
    HCP_Log(DEBUG, MODULE) << "Enter KeepPluginAlive" << HCPENDLOG;
    // 没有中止 并且 生成子任务没有结束就走进去
    uint32_t reportCnt = 0;
    while (!m_isAbort && !m_generateSubjobFinish) {
        // 10s 检查一次退出条件
        Module::SleepFor(chrono::seconds(REPORT_RUNNING_INTERVAL));
        // 60s 上报一次
        if (reportCnt % REPORT_RUNNING_TIMES == 0) {
            ReportJobDetails(make_tuple(JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS100));
        }
        reportCnt++;
    }
}

int HostRestore::DeleteSrcDirForRestore() const
{
    HCP_Log(DEBUG, MODULE) << "Enter DeleteSrcDirForRestore" << HCPENDLOG;
    string createdPath = PluginUtils::PathJoin(m_cacheFsPath, m_restoreJobInfo->jobId);
    HCP_Log(INFO, MODULE) << "the createdPath is: " << createdPath << HCPENDLOG;
    if (PluginUtils::IsDirExist(createdPath)) {
        PluginUtils::Remove(createdPath);
    }
    if (PluginUtils::IsDirExist(createdPath)) {
        HCP_Log(INFO, MODULE) << "delete createdPath failed " << HCPENDLOG;
    }
    return Module::SUCCESS;
}

string HostRestore::GetLastCtrlPath()
{
    string path;
    if (m_aggregateRestore) {
        int lastCopyNumber = m_restoreJobInfo->copies.size() - 1;
        path = PluginUtils::PathJoin(m_metaFsPathList[lastCopyNumber], "lastCtrl");
    } else {
        path = PluginUtils::PathJoin(m_metaFsPath, "lastCtrl");
    }
    return path;
}

void HostRestore::UpdateTaskInfo()
{
    INFOLOG("==== UpdateTaskInfo thread start ====");
    while (m_isRestoreInProgress) {
        if (m_backup != nullptr) {
            UpdateSubBackupStats(false);
            UpdateSpeedAndReport();
            std::this_thread::sleep_for(std::chrono::seconds(NUMBER10));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    INFOLOG("==== UpdateTaskInfo thread join ====");
}

inline bool HostRestore::GetIgnoreFailed()
{
    int ig = Module::ConfigReader::getInt("FilePluginConfig", "RESTORE_SUBJOB_IGNORE_FAILED");
    bool ignoreFailed = ig > 0 ? true : false;
    return ignoreFailed;
}
}
