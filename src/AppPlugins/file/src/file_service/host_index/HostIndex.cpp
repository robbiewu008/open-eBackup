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
#include "HostIndex.h"
#include <algorithm>
#include "PluginUtilities.h"
#include "system/System.hpp"
#include "client/ClientInvoke.h"
#include "constant/PluginConstants.h"
#include "constant/ErrorCode.h"
#include "common/EnvVarManager.h"
#include "common/Thread.h"

using namespace std;
using namespace AppProtect;
using namespace Module;
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "HostIndex";
    constexpr auto INDEXPATHDIVIDERMARK = "DEE-SHARE";
    const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
    constexpr auto RFI = "rfi";
    const int RFI_PREFIX_LENGTH = 6;
    const int REPORT_INTERVAL = 45;
    const char WIN_SEPARATOR = '\\';
    const char POSIX_SEPARATOR = '/';
}

shared_ptr<BuildIndexJob> HostIndex::GetJobInfoBody()
{
    shared_ptr<BuildIndexJob> jobPtr = dynamic_pointer_cast<BuildIndexJob>(m_jobCommonInfo->GetJobInfo());
    return jobPtr;
}

EXTER_ATTACK int HostIndex::PrerequisiteJob()
{
    m_indexPara = GetJobInfoBody();
    if (m_indexPara == nullptr) {
        ERRLOG("HostIndex Prerequisite failed for indexPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostIndex PrerequisiteJob, %s", m_indexPara->jobId.c_str());
    int ret = PrerequisiteJobInner();
    SetJobToFinish();
    INFOLOG("Leave HostIndex PrerequisiteJob, %s", m_indexPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int HostIndex::GenerateSubJob()
{
    m_indexPara = GetJobInfoBody();
    if (m_indexPara == nullptr) {
        ERRLOG("HostIndex generate subjob failed for indexpara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostIndex GenerateSubJob, %s", m_indexPara->jobId.c_str());
    int ret = GenerateSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave HostIndex GenerateSubJob, %s", m_indexPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int HostIndex::ExecuteSubJob()
{
    m_indexPara = GetJobInfoBody();
    if (m_indexPara == nullptr ||
        m_subJobInfo == nullptr) {
        ERRLOG("HostIndex execute subjob failed for indexPara is nullptr or subjob info is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostIndex ExecuteSubJob, %s", m_indexPara->jobId.c_str());
    int ret = ExecuteSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave HostIndex ExecuteSubJob, %s", m_indexPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int HostIndex::PostJob()
{
    ReportJob(SubJobStatus::COMPLETED);
    SetJobToFinish();
    return Module::SUCCESS;
}

int HostIndex::PrerequisiteJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}

int HostIndex::GenerateSubJobInner()
{
    SubJob subJob {};
    subJob.__set_jobId(m_indexPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("hostIndex");
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    vector<SubJob> vec {};
    vec.push_back(subJob);

    ActionResult ret {};
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, vec);
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJob(SubJobStatus::RUNNING);
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
    SubJobDetails subJobDetails {};
    subJobDetails.__set_jobId(m_indexPara->jobId);
    subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    string description = "Generate sub task for index task successfully";
    LogDetail logDetail {};
    vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(ret, subJobDetails);
    return Module::SUCCESS;
}

int HostIndex::ExecuteSubJobInner()
{
    int ret = IdentifyRepos();
    if (ret != Module::SUCCESS) {
        ERRLOG("Bad Identify Repos");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }

    if (m_indexType == HostIndexType::HOST_INDEX_TYPE_FULL) {
        ret = ProcessFullHostIndex();
    } else {
        ret = ProcessIncHostIndex();
    }
    return ret;
}

int HostIndex::IdentifyRepos()
{
    // 识别 cache , meta, data仓
    for (size_t i = 0; i < m_indexPara->copies.size(); i++) {
        for (auto repo : m_indexPara->copies[i].repositories) {
            if (IdentifyRepo(repo) != Module::SUCCESS) {
                return Module::FAILED;
            }
        }
    }
    // 识别 indexRepo
    for (auto repo : m_indexPara->repositories) {
        if (IdentifyRepo(repo) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    m_indexType = m_preRepo != nullptr ? HostIndexType::HOST_INDEX_TYPE_INC : HostIndexType::HOST_INDEX_TYPE_FULL;

    if (m_curRepo == nullptr || m_cacheRepo == nullptr || m_indexRepo == nullptr || m_metaRepo == nullptr) {
        ERRLOG("repo init not complete");
        return Module::FAILED;
    }
    if (m_cacheRepo->path.size() == 0 || m_indexRepo->path.size() == 0 || m_metaRepo->path.size() == 0) {
        ERRLOG("m_cacheRepo->path.size: %d, m_indexRepo->path.size: %d, m_metaRepo->path.size: %d",
            m_cacheRepo->path.size(), m_indexRepo->path.size(), m_metaRepo->path.size());
        return FAILED;
    }
    return Module::SUCCESS;
}

int HostIndex::IdentifyRepo(StorageRepository& repo)
{
    INFOLOG("repo type : %d", static_cast<int>(repo.repositoryType));
    if (repo.repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
        StorageRepositoryExtendInfo extendInfo {};
        if (!JsonHelper::JsonStringToStruct(repo.extendInfo, extendInfo)) {
            ERRLOG("Data storage repository extend info is invalid");
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
        m_indexRepo = make_shared<StorageRepository>(repo);
        INFOLOG("set index repo complete");
    } else if (repo.repositoryType == RepositoryDataType::type::META_REPOSITORY) {
        m_metaRepo = make_shared<StorageRepository>(repo);
        StorageRepositoryExtendInfo extendInfo {};
        DBGLOG("extendInfo : %s", repo.extendInfo.c_str());
        if (!JsonHelper::JsonStringToStruct(repo.extendInfo, extendInfo)) {
            ERRLOG("Meta storage repository extend info is invalid.");
            return Module::FAILED;
        }
        if (extendInfo.isCurrentCopyRepo) {
            m_metaRepo = make_shared<StorageRepository>(repo);
            INFOLOG("set cur meta repo complete!");
        } else {
            m_preMetaRepo = make_shared<StorageRepository>(repo);
            INFOLOG("set pre meta repo complete!");
        }
    }
    if (m_metaRepo != nullptr && !m_metaRepo->path.empty() &&
        m_metaRepo->path[0].find("--checkpoint=") != string::npos) {
        ERRLOG("meta repo path size is %d, or tar is unsafe, because path contains checkpoint=.",
            m_metaRepo->path.size());
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

void HostIndex::ReportJob(SubJobStatus::type status)
{
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    AddLogDetail(logDetail, "", JobLogLevel::TASK_LOG_INFO);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, status);
}

int HostIndex::ProcessFullHostIndex()
{
    if (!CheckDcacheExist(m_metaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR)) {
        // do not have dcache, report fail
        ERRLOG("dcache doesn't exist");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    // unzip and prepare latest and previous dir
    string metaFilePath = m_metaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR +
        dir_sep + METAFILE_ZIP_NAME;
    string prevMetaPath = "";
    string curMetaPath = metaFilePath;
    // prepare里解压缩， 主线程上报
    m_isPreparing = true;
    std::thread prepareThread(&HostIndex::PrepareForGenrateRfi, this, prevMetaPath, curMetaPath);
    time_t lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
    while (m_isPreparing) {
        sleep(1);
        time_t currentTime = PluginUtils::GetCurrentTimeInSeconds();
        if ((currentTime - lastReportTime) <= REPORT_INTERVAL) {
            continue;
        }
        lastReportTime = currentTime;
        ReportJob(SubJobStatus::RUNNING);
    }
    prepareThread.join();
    INFOLOG("Finish prepare %s, %s", prevMetaPath.c_str(), curMetaPath.c_str());

    string workDir = m_cacheRepo->path[0] + dir_sep + META + dir_sep + LATEST;
    INFOLOG("get dirlist in directory %s", workDir.c_str());
    vector<string> dirList;
    PluginUtils::GetDirListInDirectory(workDir, dirList);
    for (auto it = dirList.begin(); it != dirList.end();) {
        if (*it == "failedVolume") {
            it = dirList.erase(it);
        } else {
            ++it;
        }
    }
    for (int i = 0; i < dirList.size(); i++) {
        m_isLastScan = i == (dirList.size() - 1);
        INFOLOG("generate full RFI in path %s", dirList[i].c_str());
        int ret = GenerateFullRfiInPath(workDir + dir_sep + dirList[i], false);
        if (ret != SUCCESS) {
            ReportJob(SubJobStatus::FAILED);
            return Module::FAILED;
        }
    }
    return Module::SUCCESS;
}

int HostIndex::GenerateFullRfiInPath(const string& path, bool isPre)
{
    INFOLOG("Enter GenerateFullRfiInPath: %s, %d", path.c_str(), isPre);
    // unzip to cur dir
    string metafilePath = path + dir_sep + METAFILE_ZIP_NAME;
    m_isPreparing = true;
    promise<int> promiseObj;
    future<int> futureObj = promiseObj.get_future();
    std::thread unzipThread(&HostIndex::UnzipMetafileToCurPathAndRemoveAsync, this, metafilePath, std::ref(promiseObj));
    time_t lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
    while (m_isPreparing) {
        sleep(1);
        if ((PluginUtils::GetCurrentTimeInSeconds() - lastReportTime) <= REPORT_INTERVAL) {
            continue;
        }
        lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
        ReportJob(SubJobStatus::RUNNING);
    }
    unzipThread.join();
    int ret = futureObj.get();
    if (ret != SUCCESS) {
        ERRLOG("unzip failed! %s", path.c_str());
        return FAILED;
    }
    // fill scan config
    FillScanConfigForGenerateRfi("", path, m_isLastScan, isPre);
    // scan and monitor
    if (!StartScanner()) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
        ERRLOG("Start scanner failed!");
        return FAILED;
    }

    SCANNER_STATUS ret1 = MonitorScanner();
    if (static_cast<int>(ret1) < 0) {
        ERRLOG("Scan Failed! %d", static_cast<int>(ret1));
        return FAILED;
    }

    if (m_scanner != nullptr) {
        m_scanner->Destroy();
        m_scanner.reset();
    }
    return SUCCESS;
}

int HostIndex::GenerateIncRfiInPath(const string& prevPath, const string& curPath)
{
    INFOLOG("Enter GenerateIncRfiInPath: %s, %s", prevPath.c_str(), curPath.c_str());
    string metafilePath = curPath + dir_sep + METAFILE_ZIP_NAME;
    string prevMetafilePath = prevPath + dir_sep + METAFILE_ZIP_NAME;
    m_isPreparing = true;
    promise<int> promiseObj;
    future<int> futureObj = promiseObj.get_future();
    std::thread unzipThread(&HostIndex::PrepareForGenerateIncRfiInPath, this, metafilePath, prevMetafilePath,
        std::ref(promiseObj));
    time_t lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
    while (m_isPreparing) {
        sleep(1);
        if ((PluginUtils::GetCurrentTimeInSeconds() - lastReportTime) <= REPORT_INTERVAL) {
            continue;
        }
        lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
        ReportJob(SubJobStatus::RUNNING);
    }
    unzipThread.join();
    int ret = futureObj.get();
    if (ret != SUCCESS) {
        ERRLOG("untar failed! %s", curPath.c_str());
        return FAILED;
    }
    FillScanConfigForGenerateRfi(prevPath, curPath, m_isLastScan, false);
    // scan and monitor
    if (!StartScanner()) {
        if (m_scanner != nullptr) {
            m_scanner->Destroy();
        }
        ERRLOG("Start scanner failed!");
        return FAILED;
    }

    SCANNER_STATUS ret1 = MonitorScanner();
    if (static_cast<int>(ret1) < 0) {
        ERRLOG("Scan Failed! %d", static_cast<int>(ret1));
        return FAILED;
    }

    if (m_scanner != nullptr) {
        m_scanner->Destroy();
        m_scanner.reset();
    }
    return SUCCESS;
}

void HostIndex::PrepareForGenerateIncRfiInPath(const string& metaFilePath, const string& prevMetaFilePath,
    promise<int>& promiseObj)
{
    int ret = UnzipMetafileToCurPathAndRemove(metaFilePath);
    if (ret != SUCCESS) {
        ERRLOG("untar failed! %s", metaFilePath.c_str());
        promiseObj.set_value(FAILED);
        m_isPreparing = false;
        return;
    }
    ret = UnzipMetafileToCurPathAndRemove(prevMetaFilePath);
    if (ret != SUCCESS) {
        ERRLOG("untar failed! %s", prevMetaFilePath.c_str());
        promiseObj.set_value(FAILED);
        m_isPreparing = false;
        return;
    }
    promiseObj.set_value(SUCCESS);
    m_isPreparing = false;
    return;
}

int HostIndex::UnzipMetafileToCurPathAndRemove(const string& path) const
{
    INFOLOG("Enter UnzipMetafileToCurPathAndRemove: %s", path.c_str());
    string parentDir = PluginUtils::GetPathName(path);
#ifdef WIN32
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " -y x " + path + " -o" + parentDir;
    INFOLOG("unzip cmd: %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return FAILED;
    }
#else  // unix
#if defined(_AIX) || defined(SOLARIS)
    string cmd = "cd " + parentDir + " && gunzip -c " + path + " | tar -xf - && cd -";
#else
    string cmd = "tar -zxf " + path + " -C " + parentDir;
#endif
    vector<string> output;
    vector<string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    INFOLOG("unzip cmd: %s", cmd.c_str());
    if (ret != 0) {
        ERRLOG("unzip failed! %d", ret);
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        return FAILED;
    }
#endif
    PluginUtils::RemoveFile(path);
    INFOLOG("remove file : %s", path.c_str());
    return SUCCESS;
}

void HostIndex::UnzipMetafileToCurPathAndRemoveAsync(const string& path, promise<int>& promiseObj)
{
    INFOLOG("Enter UnzipMetafileToCurPathAndRemove: %s", path.c_str());
    string parentDir = PluginUtils::GetPathName(path);
#ifdef WIN32
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " -y x " + path + " -o" + parentDir;
    INFOLOG("unzip cmd: %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        m_isPreparing = false;
        promiseObj.set_value(FAILED);
        return;
    }
#else  // unix
#if defined(_AIX) || defined(SOLARIS)
    string cmd = "cd " + parentDir + " && gunzip -c " + path + " | tar -xf - && cd -";
#else
    string cmd = "tar -zxf " + path + " -C " + parentDir;
#endif
    vector<string> output;
    vector<string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    INFOLOG("unzip cmd: %s", cmd.c_str());
    if (ret != 0) {
        ERRLOG("unzip failed! %d", ret);
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        m_isPreparing = false;
        promiseObj.set_value(FAILED);
        return;
    }
#endif
    PluginUtils::RemoveFile(path);
    INFOLOG("remove file : %s", path.c_str());
    m_isPreparing = false;
    promiseObj.set_value(SUCCESS);
    return;
}

int HostIndex::ProcessIncHostIndex()
{
    if (!CheckDcacheExist(m_metaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR) ||
        !CheckDcacheExist(m_preMetaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR)) {
        ERRLOG("dcache doesn't exist");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    // unzip and prepare latest and previous dir
    string metaFilePath = m_metaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR +
        dir_sep + METAFILE_ZIP_NAME;
    string preMetaFilePath = m_preMetaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR +
        dir_sep + METAFILE_ZIP_NAME;
    m_isPreparing= true;
    std::thread prepareThread(&HostIndex::PrepareForGenrateRfi, this, preMetaFilePath, metaFilePath);
    time_t lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
    while (m_isPreparing) {
        sleep(1);
        if ((PluginUtils::GetCurrentTimeInSeconds() - lastReportTime) <= REPORT_INTERVAL) {
            continue;
        }
        lastReportTime = PluginUtils::GetCurrentTimeInSeconds();
        ReportJob(SubJobStatus::RUNNING);
    }
    prepareThread.join();
    INFOLOG("Finish prepare %s, %s", preMetaFilePath.c_str(), metaFilePath.c_str());
    // dcache generate rfi

    string workDir = m_cacheRepo->path[0] + dir_sep + META + LATEST;
    vector<string> curDirList;
    vector<string> prevDirList;
    PluginUtils::GetDirListInDirectory(workDir, curDirList);
    for (size_t i = 0; i < curDirList.size(); i++) {
        curDirList[i] = workDir + dir_sep + curDirList[i];
        INFOLOG("curDirList: %s", curDirList[i].c_str());
    }
    workDir = m_cacheRepo->path[0] + dir_sep + META + PREVIOUS;
    PluginUtils::GetDirListInDirectory(workDir, prevDirList);
    for (size_t i = 0; i < prevDirList.size(); i++) {
        prevDirList[i] = workDir + dir_sep + prevDirList[i];
        INFOLOG("prevDirList: %s", prevDirList[i].c_str());
    }

    // 先排序， 同名的做增量， 不同名的做全增、全减
    sort(curDirList.begin(), curDirList.end());
    sort(prevDirList.begin(), prevDirList.end());
    return ProcessIncHostIndex2(curDirList, prevDirList);
}

int HostIndex::ProcessIncHostIndex2(const vector<string>& curDirList, const vector<string>& prevDirList)
{
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
            if (ret != SUCCESS) {
                ReportJob(SubJobStatus::FAILED);
                return FAILED;
            }
        } else if (curSnapshotName > prevSnapshotName) {
            // 如果prev是空，应该走进这个分支， curQueue出队
            curQueue.pop();
            CheckIsLastScan(curQueue, prevQueue);
            int ret = GenerateFullRfiInPath(curPath, false);
            if (ret != SUCCESS) {
                ReportJob(SubJobStatus::FAILED);
                return Module::FAILED;
            }
        } else if (curSnapshotName < prevSnapshotName) {
            // 如果cur是空，应该走进这个分支， prevQueue出队
            prevQueue.pop();
            CheckIsLastScan(curQueue, prevQueue);
            int ret = GenerateFullRfiInPath(prevPath, true);
            if (ret != SUCCESS) {
                ReportJob(SubJobStatus::FAILED);
                return Module::FAILED;
            }
        }
    }
    int ret = ProcessIncHostIndex3(curQueue, prevQueue);
    if (ret != SUCCESS) {
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HostIndex::ProcessIncHostIndex3(std::queue<string>& curQueue, std::queue<string>& prevQueue)
{
    INFOLOG("Enter ProcessIncHostIndex3: %d, %d", curQueue.size(), prevQueue.size());
    while (!prevQueue.empty()) {
        string prevPath = prevQueue.front();
        prevQueue.pop();
        CheckIsLastScan(curQueue, prevQueue);
        int ret = GenerateFullRfiInPath(prevPath, true);
        if (ret != SUCCESS) {
            ReportJob(SubJobStatus::FAILED);
            return Module::FAILED;
        }
    }

    while (!curQueue.empty()) {
        string curPath = curQueue.front();
        curQueue.pop();
        CheckIsLastScan(curQueue, prevQueue);
        int ret = GenerateFullRfiInPath(curPath, false);
        if (ret != SUCCESS) {
            ReportJob(SubJobStatus::FAILED);
            return Module::FAILED;
        }
    }
    return SUCCESS;
}

void HostIndex::CheckIsLastScan(const std::queue<string>& curQueue, const std::queue<string>& prevQueue)
{
    if (curQueue.empty() && prevQueue.empty()) {
        m_isLastScan = true;
    }
}

#ifdef WIN32
void HostIndex::PrepareForGenrateRfi(string preMetaFilePath, string curMetafilepath)
{
    DBGLOG("Enter PrepareForGenrateRfi: %s, %s", preMetaFilePath.c_str(),
        curMetafilepath.c_str());

    // 先删掉 cache/meta
    PluginUtils::Remove(m_cacheRepo->path[0] + dir_sep + META);
    // unzip curMetafilepath to workDir/latest/
    string workDir = m_cacheRepo->path[0] + dir_sep + META + LATEST;
    string rfiDir = m_cacheRepo->path[0] + dir_sep + RFI;
    PluginUtils::CreateDirectory(workDir);
    PluginUtils::CreateDirectory(rfiDir);
    string curMetaFileZipFileName = m_metaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR + dir_sep + METAFILE_ZIP_NAME;
    string win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    string cmd = win7z + " -y x " + curMetaFileZipFileName + " -o" + workDir;
    INFOLOG("unzip cmd: %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        ReportJob(SubJobStatus::FAILED);
    }

    if (preMetaFilePath.empty()) {
        m_isPreparing = false;
        return;
    }
    // unzip preMetaFilePath to workDir/previous/
    workDir = m_cacheRepo->path[0] + dir_sep + META + PREVIOUS;
    PluginUtils::CreateDirectory(workDir);
    string preMetaFileZipFileName = m_preMetaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR +
        dir_sep + METAFILE_ZIP_NAME;
    win7z = Module::EnvVarManager::GetInstance()->GetAgentWin7zPath();
    cmd = win7z + " -y x " + preMetaFileZipFileName + " -o" + workDir;
    INFOLOG("unzip cmd: %s", cmd.c_str());
    ret = Module::ExecWinCmd(cmd, errCode);
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        ReportJob(SubJobStatus::FAILED);
    }
    m_isPreparing = false;
}

#else

void HostIndex::PrepareForGenrateRfi(string preMetaFilePath, string curMetafilepath)
{
    DBGLOG("Enter PrepareForGenrateRfi: %s, %s", preMetaFilePath.c_str(),
        curMetafilepath.c_str());
    // 先删掉 cache/meta
    PluginUtils::Remove(m_cacheRepo->path[0] + dir_sep + META);
    // unzip curMetafilepath to workDir/latest/
    string workDir = m_cacheRepo->path[0] + dir_sep + META + LATEST;
    string rfiDir = m_cacheRepo->path[0] + dir_sep + RFI;
    PluginUtils::CreateDirectory(workDir);
    PluginUtils::CreateDirectory(rfiDir);
    string curMetaFileZipFileName = m_metaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR +
        dir_sep + METAFILE_ZIP_NAME;
    
    vector<string> output;
    vector<string> errOutput;
#if defined(_AIX) || defined(SOLARIS)
    string cmd = "cd " + workDir + " && gunzip -c " + curMetaFileZipFileName + " | tar -xf - && cd -";
#else
    string cmd = "tar -zxf " + curMetaFileZipFileName + " -C " + workDir;
#endif
    INFOLOG("unzip metafile to cache repo: %s", cmd.c_str());
    if (PluginUtils::RunShellCmd(cmd, {}) != Module::SUCCESS) {
        ERRLOG("unzip failed!");
        ReportJob(SubJobStatus::FAILED);
    }

    if (preMetaFilePath.empty()) {
        m_isPreparing = false;
        return;
    }
    // unzip preMetaFilePath to workDir/previous/
    workDir = m_cacheRepo->path[0] + dir_sep + META + PREVIOUS;
    PluginUtils::CreateDirectory(workDir);
    string preMetaFileZipFileName = m_preMetaRepo->path[0] + dir_sep + METAFILE_PARENT_DIR +
        dir_sep + METAFILE_ZIP_NAME;
#if defined(_AIX) || defined(SOLARIS)
    cmd = "cd " + workDir + " && gunzip -c " + preMetaFileZipFileName + " | tar -xf - && cd -";
#else
    cmd = "tar -zxf " + preMetaFileZipFileName + " -C " + workDir;
#endif
    INFOLOG("unzip pre metafile to cache repo : %s", cmd.c_str());
    if (PluginUtils::RunShellCmd(cmd, {}) != Module::SUCCESS) {
        ERRLOG("unzip failed!");
        ReportJob(SubJobStatus::FAILED);
    }
    m_isPreparing = false;
}

#endif

void HostIndex::FillScanConfigForGenerateRfi(const string& prevDcachePath,
    const string& curDcachePath, bool isLastScan, bool isPre)
{
    INFOLOG("Enter FillScanConfig: %s, %s", prevDcachePath.c_str(), curDcachePath.c_str());
    m_scanConfig.jobId = m_indexPara->jobId;
    m_scanConfig.subJobId = m_subJobInfo->subJobId;
    m_scanConfig.copyId = m_curRepoExtendInfo->copyId;
    m_scanConfig.scanType = ScanJobType::RFI_GEN;
    m_scanConfig.scanIO = IOEngine::DEFAULT;
    m_scanConfig.lastBackupTime = 0;
    m_scanConfig.usrData = (void*)this;
    m_scanConfig.isPre = isPre;
    m_scanConfig.isLastScan = isLastScan;

    /* config meta path */
    m_scanConfig.metaPath = m_cacheRepo->path[0] + dir_sep + META;
    m_scanConfig.metaPathForCtrlFiles = m_cacheRepo->path[0] + dir_sep + RFI;
    m_scanConfig.curDcachePath = curDcachePath;
    m_scanConfig.prevDcachePath = prevDcachePath;
#ifdef WIN32
    // windows 原生格式只挂载到pvc_dee_share , 要拼接上remotePath后面的路径
    if (m_indexRepo->path[0].find(RFI) == std::string::npos) {
        std::size_t pos = m_indexRepo->remotePath.find(RFI);
        std::string rfiReletivePath = PluginUtils::ReverseSlash(m_indexRepo->remotePath.substr(pos));
        m_scanConfig.indexPath = m_indexRepo->path[0] + dir_sep + rfiReletivePath;
    } else {
        m_scanConfig.indexPath = m_indexRepo->path[0];
    }
#else
    m_scanConfig.indexPath = m_indexRepo->path[0];
#endif
    m_scanConfig.generatorIsFull = true;
    m_scanConfig.maxOpendirReqCount = MAX_OPEN_DIR_REQ_COUNT;
    m_scanConfig.generatorIsFull = prevDcachePath == "";

    // /* 记录线程数 */
    m_scanConfig.maxCommonServiceInstance = 1;
    m_scanConfig.scanCtrlMaxDataSize = to_string(ONE_GB);
    m_scanConfig.scanCtrlMinDataSize = to_string(HALF_GB);
    m_scanConfig.scanCtrlFileTimeSec = SCAN_CTRL_FILE_TIMES_SEC;
    m_scanConfig.scanCtrlMaxEntriesFullBkup = SCAN_CTRL_MAX_ENTRIES_FULL_BACKUP;
    m_scanConfig.scanCtrlMaxEntriesIncBkup = SCAN_CTRL_MAX_ENTRIES_INCBKUP;
    m_scanConfig.scanCtrlMinEntriesFullBkup = SCAN_CTRL_MIN_ENTRIES_FULL_BKUP;
    m_scanConfig.scanCtrlMinEntriesIncBkup = SCAN_CTRL_MIN_ENTRIES_INC_BKUP;
    m_scanConfig.scanMetaFileSize = ONE_GB;
    m_scanConfig.maxWriteQueueSize = SCAN_CTRL_MAX_QUEUE_SIZE;
    m_scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    m_scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;
    m_scanConfig.rfiCtrlCb = GenerateRfiCtrlFileCb;
    if (Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "KEEP_RFI_IN_CACHE_REPO") == "1") {
        INFOLOG("set to keep rfifile, jobId %s", m_indexPara->jobId.c_str());
        m_scanConfig.keepRfiFile = true;
    }
    HCP_Log(INFO, MODULE) << "EXIT FillScanConfig" << HCPENDLOG;
}

bool HostIndex::CheckDcacheExist(string metaFilePath) const
{
    string metaFile = metaFilePath + dir_sep + METAFILE_ZIP_NAME;
    DBGLOG("CheckDcacheExist %s", metaFile.c_str());
    return PluginUtils::IsFileExist(metaFile);
}

bool HostIndex::StartScanner()
{
    DBGLOG("Enter StartScanner");
    m_scanner = ScanMgr::CreateScanInst(m_scanConfig);
    if (!m_scanner || m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        ERRLOG("Start scanner instance failed!");
        return false;
    }
    DBGLOG("leave StartScanner;");
    return true;
}

SCANNER_STATUS HostIndex::MonitorScanner()
{
    INFOLOG("Enter MonitorScanner");
    SCANNER_STATUS status;
    do {
        status = m_scanner->GetStatus();
        if (status == SCANNER_STATUS::COMPLETED) {
            INFOLOG("Scanner complete!");
            break;
        }
        if (status == SCANNER_STATUS::ABORTED) {
            INFOLOG("Scanner aborted!");
            break;
        }
        if (static_cast<int>(status) < 0) {
            ERRLOG("Scan failed!");
            ReportJob(SubJobStatus::FAILED);
            break;
        }
        sleep(SLEEP_TEN_SECONDS);
    } while (true);
    INFOLOG("Exit MonitorScanner");
    return status;
}

void HostIndex::GeneratedCopyCtrlFileCb(void* /* usrData */, string ctrlFile)
{
    DBGLOG("GeneratedCopyCtrlFileCb: %s", ctrlFile.c_str());
}

void HostIndex::GeneratedHardLinkCtrlFileCb(void* /* usrData */, string ctrlFile)
{
    DBGLOG("GenreateHardlinkCtrlFileCb: %s", ctrlFile.c_str());
}

void HostIndex::GenerateRfiCtrlFileCb(void* /* usrData */, RfiCbStruct cbParam)
{
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
}