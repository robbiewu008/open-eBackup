/*
* Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
* Description: Prouce meta&xmeta base log files for object storage.
* Author: w00444223
* Create: 2024-02-19
*/
#include "ObjectLogProcess.h"
#include "log/Log.h"
#include "ScannerUtils.h"
#include "ControlFileUtils.h"

#include "parser/ObjectListParser.h"

using namespace std;
using namespace Module;

namespace {
    constexpr uint32_t BLOCKING_TIMEOUT = 100;
    const std::string SUBPREFIX_FILE = "subprefix.txt";
    const std::string OBS_OPERATE_TYPE_MODIFY = "MODIFY";
    const std::string OBS_OPERATE_TYPE_DELETE = "DELETE";
    const std::string OBS_OPERATE_TYPE_ADD_OR_MODIFY = "UNSURE";
    constexpr auto MODULE = "ObjectLogProcess";
}

ObjectLogProcess::ObjectLogProcess(ScanConfig& config,
    std::shared_ptr<BufferQueue<DirectoryScan>> output, std::shared_ptr<StatisticsMgr> statsMgr)
    : m_config(config), m_output(output), m_statsMgr(statsMgr)
{
    m_logFileDirPath = m_config.obs.buckets[0].logDir;
    m_subPrefixRootDir = m_logFileDirPath + PATH_SEPERATOR + m_config.obs.buckets[0].bucketName;
    m_delimiter = m_config.obs.buckets[0].delimiter;
    m_input = make_shared<BufferQueue<DirCache>>(m_config.maxScanQueueSize);
    m_scanMetaStat = make_shared<MetadataStat>();
}

int ObjectLogProcess::ReadDcache(std::string &metaDir)
{
    ControlFileUtils cfu {};
    std::string dcacheName = cfu.GetDirCacheFileName(metaDir);
    shared_ptr<DirCacheParser> currDcacheObj = cfu.CreateDcacheObj(dcacheName, CTRL_FILE_OPEN_MODE::READ, m_config);
    if (currDcacheObj == nullptr) {
        ERRLOG("dcache obj create fail");
        return Module::FAILED;
    }

    m_readThreads.clear();
    for (int i = 0; i < m_config.producerThreadCount; ++i) {
        auto workerThread = std::make_shared<ObjectMetaReadThread>(m_input, m_output, m_scanMetaStat, m_config,
        m_statsMgr, m_subPrefixRootDir, m_delimiter);
        if (!workerThread->Start()) {
            ERRLOG("Start failed for read meta thread: %d ", i);
            return Module::FAILED;
        }
        m_readThreads.push_back(workerThread);
    }

    int ret = Module::SUCCESS;
    std::queue<DirCache> dcQueue {};
    while (true) {
        if (dcQueue.empty()) {
            currDcacheObj->ReadDirCacheEntries(dcQueue, DCACHE_ENTRY_BATCH_READ_CNT);
        }
        if (dcQueue.empty()) {
            DBGLOG("dcache read completed");
            break;
        }
        while (!dcQueue.empty()) {
            DirCache dcache = dcQueue.front();
            dcQueue.pop();
            m_input->Push(dcache);
        }
    }
    m_input->SetPushFinished();
    currDcacheObj->Close(CTRL_FILE_OPEN_MODE::READ);
    return ret;
}

void ObjectLogProcess::Poll()
{
    uint64_t pushCnt = m_input->GetPushCount();
    uint64_t consumeCnt = m_statsMgr->GetCommStatsByType(CommStatsType::DCACHE_READ_COUNT);
    while (true) {
        if (pushCnt == consumeCnt) {
            break;
        } else {
            DBGLOG("Diff thread running...");
            Module::SleepFor(std::chrono::seconds(1));
        }
        consumeCnt = m_statsMgr->GetCommStatsByType(CommStatsType::DCACHE_READ_COUNT);
    }
}

void ObjectLogProcess::ClearDiffThreads()
{
    if (!m_readThreads.empty()) {
        for (auto workerThread : m_readThreads) {
            workerThread->Exit();
        }
        m_readThreads.clear();
    }
}

bool ObjectLogProcess::ValidateCurrFileCount()
{
    m_scanMetaStat->m_metaVersion = META_HEADER_VERSION_V20;
    if (m_scanMetaStat->m_metaFileCount == 0) {
        ERRLOG("Metafile counters are invalid");
        return false;
    }
    if (m_scanMetaStat->m_fcacheFileCount == 0) {
        ERRLOG("Fcache counters are invalid");
        return false;
    }
    INFOLOG("Meta Version %s, Metafile : %d, xMetafile : %d, fileCache: %d",
        m_scanMetaStat->m_metaVersion.c_str(), m_scanMetaStat->m_metaFileCount, m_scanMetaStat->m_xMetaFileCount,
        m_scanMetaStat->m_fcacheFileCount);
    return true;
}

bool ObjectLogProcess::OpenAllFcacheMetaFiles(std::string &metaDir)
{
    ControlFileUtils cfu {};
    string metaFileCountStr = cfu.GetMetaFileCountName(metaDir);
    bool ret = cfu.GetMetaFileCountFromFile(metaFileCountStr, m_scanMetaStat->m_metaFileCount,
        m_scanMetaStat->m_xMetaFileCount, m_scanMetaStat->m_fcacheFileCount, m_config.maxCommonServiceInstance);
    if (!ret) {
        return false;
    }

    if (!ValidateCurrFileCount()) {
        return false;
    }

    string curFname {};
    for (uint32_t i = 0; i < m_scanMetaStat->m_metaFileCount; i++) {
        curFname = cfu.GetMetaFileName(metaDir, i);
        shared_ptr<MetaParser> mcObj = move(cfu.CreateMetaFileObj(curFname, CTRL_FILE_OPEN_MODE::READ, m_config));
        if (mcObj == nullptr) {
            ERRLOG("Failed to open metafile: %s", curFname.c_str());
            return false;
        }
        if (mcObj->GetFileVersion() != m_scanMetaStat->m_metaVersion) {
            ERRLOG("Invalid Current Meta File Version.");
            return false;
        }
        m_scanMetaStat->m_metaFiles.push_back(mcObj);
    }

    for (uint32_t i = 0; i < m_scanMetaStat->m_xMetaFileCount; i++) {
        curFname = cfu.GetXMetaFileName(metaDir, i);
        shared_ptr<XMetaParser> xmcObj = move(cfu.CreateXMetaFileObj(curFname, CTRL_FILE_OPEN_MODE::READ, m_config));
        if (xmcObj == nullptr) {
            ERRLOG("Failed to open xmetafile: %s", curFname.c_str());
            return false;
        }
        m_scanMetaStat->m_xMetaFiles.push_back(xmcObj);
    }

    for (uint32_t i = 0; i < m_scanMetaStat->m_fcacheFileCount; i++) {
        curFname = cfu.GetFileCacheFileName(metaDir, i);
        shared_ptr<FileCacheParser> fcObj = move(cfu.CreateFileCacheObj(curFname, CTRL_FILE_OPEN_MODE::READ, m_config));
        if (fcObj == nullptr) {
            ERRLOG("Failed to open fcachefile: %s", curFname.c_str());
            return false;
        }
        m_scanMetaStat->m_fcacheFiles.push_back(fcObj);
    }

    return true;
}

void ObjectLogProcess::CloseScanMetaFiles()
{
    for (uint32_t i = 0; i < m_scanMetaStat->m_metaFiles.size(); i++) {
        m_scanMetaStat->m_metaFiles[i]->Close(CTRL_FILE_OPEN_MODE::READ);
    }
    for (uint32_t i = 0; i < m_scanMetaStat->m_xMetaFiles.size(); i++) {
        m_scanMetaStat->m_xMetaFiles[i]->Close(CTRL_FILE_OPEN_MODE::READ);
    }
    for (uint32_t i = 0; i < m_scanMetaStat->m_fcacheFiles.size(); i++) {
        m_scanMetaStat->m_fcacheFiles[i]->Close(CTRL_FILE_OPEN_MODE::READ);
    }
}

int ObjectLogProcess::GenNonModifyMeta()
{
    std::string preMetaDir = m_config.metaPath + "/previous/";
    std::string latestMetaDir = m_config.metaPath + "/latest/";
    INFOLOG("Copy unmodified meta data.");
    if (!m_logFileExist) {
        // 没有增量日志，直接使用之前的元数据
        std::vector<std::string> fileList {};
        if (!FS_SCANNER::GetFileListInDirectory(preMetaDir, fileList)) {
            ERRLOG("Get %s meta files failed.", preMetaDir.c_str());
            return Module::FAILED;
        }
        for (auto &fileName : fileList) {
            if (!FS_SCANNER::CopyFile(fileName, latestMetaDir)) {
                ERRLOG("Copy %s failed.", fileName.c_str());
                return Module::FAILED;
            }
        }
        INFOLOG("No increment log, copy all meta data.");
        return Module::SUCCESS;
    }

    if (!OpenAllFcacheMetaFiles(preMetaDir)) {
        CloseScanMetaFiles();
    }

    int ret = ReadDcache(preMetaDir);
    Poll();
    ClearDiffThreads();
    CloseScanMetaFiles();
    
    INFOLOG("End, ret: %d", ret);
    return ret;
}

/*
 * 需要处理的对象满足：1、包含指定的前缀；2、处理的深度为0或包含的深度<=最大深度
 */
bool ObjectLogProcess::NeedHandleObject(std::string& key)
{
    uint32_t maxDepth = m_config.obs.buckets[0].prefixSplitDepth;
    std::vector<std::string> &prefix = m_config.obs.buckets[0].prefix;

    if (key.empty()) {
        return false;
    }

    bool flag = false;
    for (auto &item : prefix) {
        if (key.find(item) == 0) {
            flag = true;
            break;
        }
    }
    if (!prefix.empty() && !flag) {
        return false;
    }

    if (maxDepth == 0) {
        return true;
    }

    std::vector<std::string> subPrefix {};
    boost::algorithm::split(subPrefix, key, boost::is_any_of(m_delimiter), boost::token_compress_off);
    if (subPrefix.size() - 1 > maxDepth) {
        return false;
    }

    return true;
}

int ObjectLogProcess::HandleLogKey(std::string &key, std::string &operType)
{
    if (!NeedHandleObject(key)) {
        return Module::SUCCESS;
    }

    std::vector<std::string> &prefix = m_config.obs.buckets[0].prefix;
    std::string subDir = m_subPrefixRootDir;
    if (m_delimiter.empty()) {
        for (auto &item : prefix) {
            if (key.find(item) == 0) {
                subDir = subDir + PATH_SEPERATOR + item;
                break;
            }
        }
    } else {
        std::vector<std::string> subPrefix {};
        boost::algorithm::split(subPrefix, key, boost::is_any_of(m_delimiter), boost::token_compress_off);
        for (size_t i = 0; i < subPrefix.size() - 1; i++) {
            subDir = subDir + PATH_SEPERATOR + subPrefix[i];
        }
    }
    if (!FS_SCANNER::CreateDirRecurve(subDir)) {
        return Module::FAILED;
    }

    std::string fileName = subDir + PATH_SEPERATOR + SUBPREFIX_FILE;
    std::ofstream writeFd {};
    writeFd.open(fileName.c_str(), std::ios::out | std::ios::app);
    if (!writeFd.is_open()) {
        char errmsg[MAX_ERROR_MSG_LEN];
        strerror_r(errno, errmsg, MAX_ERROR_MSG_LEN);
        ERRLOG("Write %s failed, ERR: %s", fileName.c_str(), errmsg);
        return Module::FAILED;
    }

    writeFd << key << "," << operType << std::endl;
    writeFd.close();
    return Module::SUCCESS;
}

int ObjectLogProcess::SortByPrefix()
{
    if (FS_SCANNER::PathExist(m_subPrefixRootDir)) {
        FS_SCANNER::RemoveDir(m_subPrefixRootDir);
    }

    std::vector<std::string> fileList {};
    if (!FS_SCANNER::GetFileListInDirectory(m_logFileDirPath, fileList)) {
        ERRLOG("Get %s log files failed.", m_logFileDirPath.c_str());
        return Module::FAILED;
    }

    if (fileList.empty()) {
        INFOLOG("No logfile under dir %s.", m_logFileDirPath.c_str());
        m_logFileExist = false;
        return Module::SUCCESS;
    }

    if (!FS_SCANNER::CreateDir(m_subPrefixRootDir)) {
        ERRLOG("Create temp dir failed.");
        return Module::FAILED;
    }

    ObjectListParserParams args {};
    for (auto &fileName : fileList) {
        DBGLOG("logfile: %s", fileName.c_str());
        args.originalObjectListFilePath= fileName;
        ObjectListParser obsLogParser(args);
        if (obsLogParser.Open(CTRL_FILE_OPEN_MODE::READ) == CTRL_FILE_RETCODE::FAILED) {
            return Module::FAILED;
        }
        while (true) {
            std::string operType = OBS_OPERATE_TYPE_ADD_OR_MODIFY;
            BucketLogInfo bucketLogInfo {};
            if (obsLogParser.ReadEntry(bucketLogInfo) == CTRL_FILE_RETCODE::READ_EOF) {
                break;
            }
            // objectOperation 只会有 ObjectOperation::DELETE 或 ADD_OR_MODIFY，不能区分 ADD与MODIFY
            if (bucketLogInfo.objectOperation == ObjectOperation::DELETE) {
                DBGLOG("%s is delete", bucketLogInfo.objectName.c_str());
                operType = OBS_OPERATE_TYPE_DELETE;
            }
            if (HandleLogKey(bucketLogInfo.objectName, operType) != Module::SUCCESS) {
                obsLogParser.Close(CTRL_FILE_OPEN_MODE::READ);
                return Module::FAILED;
            }
        }
        obsLogParser.Close(CTRL_FILE_OPEN_MODE::READ);
    }

    return Module::SUCCESS;
}
