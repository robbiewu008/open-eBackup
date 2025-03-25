/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Object storage bucket scanner.
* Author: w30058212
* Create: 2023-12-04
*/

#include "ObjectMetaReadThread.h"

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
    constexpr auto MODULE = "ObjectMetaReadThread";
}

bool ObjectMetaReadThread::Start()
{
    try {
        m_mainThread = std::make_shared<std::thread>(std::bind(&ObjectMetaReadThread::ThreadFunc, this));
    } catch (std::exception &e) {
        HCP_Log(ERR, MODULE) << "Exception when creating ThreadMain is: "
            << e.what() << HCPENDLOG;
        return false;
    }
    return true;
}

void ObjectMetaReadThread::ThreadFunc()
{
    while (true) {
        DirCache dcache;
        if (IsReadCompleted()) {
            DBGLOG("ObjectMetaReadThread IsReadCompleted!");
            break;
        }
        if (!m_input->BlockingPop(dcache, BLOCKING_TIMEOUT)) {
            continue;
        }
        m_statsMgr->IncrCommStatsByType(CommStatsType::DCACHE_READ_COUNT);

        DirectoryScan dirNode {};
        DirMetaWrapper dmWrapper {};

        int ret = FillDirMetaWrapperByMetaFile(dcache.m_fileId, dcache.m_mdataOffset, dmWrapper);
        if (ret != Module::SUCCESS) {
            ERRLOG("Fill dir metawrapper failed, inode: %llu.", dcache.m_inode);
            continue;
        }
        std::string dirPath = GetObjectPath(dmWrapper.m_xMeta);
        DBGLOG("ObjectMetaReadThread read dir: %s", dirPath.c_str());

        if (dcache.m_totalFiles == 0) {
            PushDirToWriteQueue(dirNode, dmWrapper);
            continue;
        }

        dirNode.m_isDirScanCompleted = true;
        m_dirLogExist = false;
        m_flag = true;
        m_fileOperations.clear();
        ret = HandleFcache(dirNode, dcache);
        if (ret != Module::SUCCESS) {
            ERRLOG("Fill dir metawrapper failed, inode: %llu.", dcache.m_inode);
            CleanFileRes();
            continue;
        }
        if (dirNode.m_isDirScanCompleted && IsExistNewKey()) {
            dirNode.m_isDirScanCompleted = false;
        }
        PushDirToWriteQueue(dirNode, dmWrapper);
    }
    CleanFileRes();
}

bool ObjectMetaReadThread::IsExistNewKey()
{
    if (!m_dirLogExist) {
        return false;
    }

    return m_fileOperations.size() != 0;
}

void ObjectMetaReadThread::PushDirToWriteQueue(DirectoryScan &node, const DirMetaWrapper &dirWrapper)
{
    node.m_dmWrapper = dirWrapper;
    node.m_isResumeCalled = 0;
    if (!m_config.enableProduce) {
        DBGLOG("push not enabled");
        return;
    }

    bool pushFlag = false;
    while (!pushFlag) {
        pushFlag = m_output->BlockingPush(node, BLOCKING_TIMEOUT);
    }

    return;
}

int ObjectMetaReadThread::FillDirMetaWrapperByMetaFile(uint16_t metaId, uint64_t metaOffset, DirMetaWrapper &dmWrapper)
{
    if (metaId >= m_scanMetaStat->m_metaFiles.size()) {
        ERRLOG("Invalid meta index. metaid: %d, metafile num: %d", metaId, m_scanMetaStat->m_metaFiles.size());
        return Module::FAILED;
    }

    auto metaFileObj = m_scanMetaStat->m_metaFiles[metaId];
    if (metaFileObj->GetFileVersion() != FCACHE_HEADER_VERSION_V20) {
        ERRLOG("Invalid Current Meta File Version.");
        return Module::FAILED;
    }

    if (metaFileObj->ReadDirectoryMeta(dmWrapper.m_meta, metaOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read meta file.");
        return Module::FAILED;
    }

    ReadXMeta(dmWrapper.m_meta.m_xMetaFileIndex, dmWrapper.m_meta.m_xMetaFileOffset, dmWrapper.m_xMeta);
    return Module::SUCCESS;
}

int ObjectMetaReadThread::FillFileMetaWrapperByMetaFile(uint16_t metaId, uint64_t metaOffset, FileMetaWrapper &fmWrapper)
{
    if (metaId >= m_scanMetaStat->m_metaFiles.size()) {
        ERRLOG("Invalid meta index. metaid: %d, metafile num: %d", metaId, m_scanMetaStat->m_metaFiles.size());
        return Module::FAILED;
    }

    auto metaFileObj = m_scanMetaStat->m_metaFiles[metaId];
    if (metaFileObj->GetFileVersion() != FCACHE_HEADER_VERSION_V20) {
        ERRLOG("Invalid Current Meta File Version.");
        return Module::FAILED;
    }

    if (metaFileObj->ReadFileMeta(fmWrapper.m_meta, metaOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read meta file");
        return Module::FAILED;
    }

    ReadXMeta(fmWrapper.m_meta.m_xMetaFileIndex, fmWrapper.m_meta.m_xMetaFileOffset, fmWrapper.m_xMeta);
    return Module::SUCCESS;
}

int ObjectMetaReadThread::ReadXMeta(uint64_t xMetaId, uint64_t xMetaOffset, std::vector<XMetaField> &xMeta)
{
    if (xMetaId >= m_scanMetaStat->m_xMetaFiles.size()) {
        ERRLOG("Invalid xmeta index. xmetaid: %d, xmetafile num: %d", xMetaId, m_scanMetaStat->m_xMetaFiles.size());
        return Module::FAILED;
    }

    auto xMetaFileObj = m_scanMetaStat->m_xMetaFiles[xMetaId];
    if (xMetaFileObj->ReadXMeta(xMeta, xMetaOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read xmeta file");
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

int ObjectMetaReadThread::SetDirDcanNode(DirectoryScan &dirNode, FileCache& fcache)
{
    FileMetaWrapper fmWrapper {};
    int ret = FillFileMetaWrapperByMetaFile(fcache.m_fileId, fcache.m_mdataOffset, fmWrapper);
    if (ret != Module::SUCCESS) {
        ERRLOG("Fill file metawrapper failed, inode: %llu.", fcache.m_inode);
        return ret;
    }

    std::string key = ParserUtils::ParseObjectKey(fmWrapper.m_xMeta);
    if (m_flag) {
        ReadFromLogFile(key);
        m_flag = false;
    }
    if (m_dirLogExist && (m_fileOperations.find(key) != m_fileOperations.end())) {
        if (m_fileOperations[key] != OBS_OPERATE_TYPE_DELETE) {
            DBGLOG("SetDirDcanNode skip %s", key.c_str());
            dirNode.m_isDirScanCompleted = false;
        }
        m_fileOperations.erase(key);
    } else {
        memset_s(fmWrapper.m_filePathHash.sha1, SHA_DIGEST_LENGTH + 1, 0x0, SHA_DIGEST_LENGTH + 1);
        SHA1(reinterpret_cast<const unsigned char *>(key.c_str()), key.length(), fmWrapper.m_filePathHash.sha1);
        dirNode.m_fmWrapperList->push(fmWrapper);
    }
    return Module::SUCCESS;
}

std::string ObjectMetaReadThread::GetObjectLogFile(std::string &key)
{
    std::string subDir = m_subPrefixRootDir;
    std::vector<std::string> subPrefix {};
    boost::algorithm::split(subPrefix, key, boost::is_any_of(m_delimiter), boost::token_compress_off);
    for (size_t i = 0; i < subPrefix.size() - 1; i++) {
        subDir = subDir + PATH_SEPERATOR + subPrefix[i];
    }
    return subDir + PATH_SEPERATOR + SUBPREFIX_FILE;
}


void ObjectMetaReadThread::CleanFileRes()
{
    m_fileOperations.clear();
}

int ObjectMetaReadThread::HandleFcache(DirectoryScan& dirNode, DirCache &dcache)
{
    uint64_t fcacheOffset = dcache.m_fcacheOffset;
    uint16_t metaFileIndex = dcache.m_fileId;
    uint16_t facheIndex = dcache.m_fcacheFileId;
    uint32_t totalCount = dcache.m_totalFiles;
    uint64_t nextOffset = 0;

    if (facheIndex >= m_scanMetaStat->m_fcacheFiles.size()) {
        ERRLOG("Invalid fcache index. fcacheid: %d, fcache num: %d", facheIndex, m_scanMetaStat->m_fcacheFiles.size());
        return Module::FAILED;
    }

    std::queue<FileCache> fcQueue = {};
    auto fileObj = m_scanMetaStat->m_fcacheFiles[facheIndex];
    if (fileObj->ReadFileCacheEntries(fcQueue, fcacheOffset, totalCount, metaFileIndex, nextOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        return Module::FAILED;
    }

    while (true) {
        uint64_t curOffset = nextOffset;
        if (fcQueue.empty() && totalCount > 0) {
            if (fileObj->ReadFileCacheEntries(fcQueue, curOffset, totalCount, metaFileIndex, nextOffset) != CTRL_FILE_RETCODE::SUCCESS) {
                ERRLOG("Read filecache entries failed. metaFileIndex = %d", metaFileIndex);
                return Module::FAILED;
            }
        }
        if (fcQueue.empty()) {
            break;
        }
        FileCache fcache = fcQueue.front();
        fcQueue.pop();
        totalCount--;
        if (SetDirDcanNode(dirNode, fcache) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    return Module::SUCCESS;
}

void ObjectMetaReadThread::ReadFromLogFile(std::string &key)
{
    std::string subLogDir = GetObjectLogFile(key);
    if (!FS_SCANNER::PathExist(subLogDir)) {
        DBGLOG("Not exist file: %s", subLogDir.c_str());
        return;
    }
    
    std::shared_ptr<SubPrefixParser> subPrefixFile = nullptr;
    subPrefixFile = std::make_shared<SubPrefixParser>();
    subPrefixFile->fd.open(subLogDir.c_str(), std::ios::out | std::ios::in);
    if (!subPrefixFile->fd.is_open()) {
        char errmsg[MAX_ERROR_MSG_LEN];
        strerror_r(errno, errmsg, MAX_ERROR_MSG_LEN);
        ERRLOG("Read %s failed, ERR: %s", subLogDir.c_str(), errmsg);
        return;
    }

    subPrefixFile->fd.seekg(0, std::ios::beg);
    std::string curLine {};
    while (true) {
        std::streampos curFilePos = subPrefixFile->fd.tellg();
        getline(subPrefixFile->fd, curLine);
        if (curLine.empty()) {
            break;
        }
        size_t pos = curLine.rfind(",");
        if (pos == std::string::npos) {
            ERRLOG("Get key failed from %s", subLogDir.c_str());
            break;
        }
        m_fileOperations.emplace(curLine.substr(0, pos), curLine.substr(pos + 1));
    }
    m_dirLogExist = true;
    return;
}

void ObjectMetaReadThread::Exit()
{
    m_exit = true;
    if (m_mainThread->joinable()) {
        m_mainThread->join();
    }
}

bool ObjectMetaReadThread::IsReadCompleted()
{
    uint64_t pushCnt  = m_input->GetPushCount();
    uint64_t popCnt = m_input->GetPopCount();

    return (pushCnt == popCnt) && m_input->IsPushFinished();
}