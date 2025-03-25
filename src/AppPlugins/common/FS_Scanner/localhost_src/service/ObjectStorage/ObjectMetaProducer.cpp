/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Prouce meta&xmeta for object storage.
* Author: w00444223
* Create: 2023-12-04
*/

#include "ObjectMetaProducer.h"
#include <sys/types.h>
#include "log/Log.h"
#include "ScannerUtils.h"
#include "common/MpString.h"

using namespace std;
using namespace Module;

namespace {
    constexpr uint32_t BLOCKING_TIMEOUT = 100;
    constexpr uint32_t MEM_TRIM_THRESHOLD = 100;
    constexpr uint32_t MAX_RETRY_CNT = 20;
    constexpr int RETRY_INTERVAL_TIME = 5;
    constexpr int MAX_FILES_NUM_OF_DIR = 8000;
    const std::string SUBPREFIX_FILE = "subprefix.txt";
    const std::string OBS_OPERATE_TYPE_DELETE = "DELETE";
}

using namespace FS_SCANNER;

SCANNER_STATUS ObjectMetaProducer::InitContext()
{
    m_obsCtx = CloudServiceManager::CreateInst(m_config.obs.authArgs);
    return SCANNER_STATUS::SUCCESS;
}

SCANNER_STATUS ObjectMetaProducer::DestroyContext()
{
    return SCANNER_STATUS::SUCCESS;
}

void ObjectMetaProducer::Produce(int count)
{
    HCPTSP::getInstance().reset(m_config.reqID);
    DirStat dirStat {};

    if (!m_scanQueue->BlockingPop(dirStat, BLOCKING_TIMEOUT)) {
        return;
    }

    if (!m_config.obs.IncUseLog && !dirStat.m_prefix.empty()) {
        m_scanPreFix = dirStat.m_prefix;
        dirStat.m_prefix = "";
    }

    if (dirStat.m_path.empty() || (SetObjectInode() != Module::SUCCESS)) { // 生成m_genInode
        m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
        return;
    }
    DBGLOG("Produce, count %d, path %s perfix %s", count, dirStat.m_path.c_str(), m_scanPreFix.c_str());

    std::string dirFullPath = GetFullPath(dirStat.m_prefix, dirStat.m_path);
    m_bucket = GetObsBucketConfig(dirFullPath);
    if (m_bucket.bucketName.empty()) {
        ERRLOG("Can not get bucket from path: %s", dirFullPath.c_str());
        m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
        return;
    }
    
    std::string conflictRelativePath = CMpString::StrReplace(dirStat.m_path, m_bucket.delimiter, PATH_SEPERATOR);
    ReadInodeConflict(m_config.metaPath + PATH_SEPERATOR + conflictRelativePath);
    ProcessCurDir(dirStat);
    SaveInodeConflict(m_config.metaPath + PATH_SEPERATOR + conflictRelativePath);
    return;
}

void ObjectMetaProducer::ProcessCurDir(DirStat &dirStat)
{
    DirMetaWrapper dirWrapper {};
    int ret = WrapDirectory(dirWrapper, dirStat);
    if (ret != Module::SUCCESS) {
        m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
        if (ret != ENOENT) {
            RecordFailedScanEntry(dirStat.m_path, ret, m_statsMgr);
            ERRLOG("Wrap directory failed.");
        }
        DBGLOG("return ENOENT.");
        return ;
    }
    DirectoryScan bucketNode {};
    bucketNode.m_dmWrapper = dirWrapper;
    m_fullPath = PATH_SEPERATOR + dirStat.m_path + PATH_SEPERATOR;
    m_directoryMap[m_fullPath] = bucketNode;

    if (m_config.obs.IncUseLog) {
        // 日志增量不需要 执行 PushParentToWriteQueue，拷贝的旧meta已经包含了输入prefix的父前缀
        ScanObjectLogContent(dirWrapper, dirStat);
        m_statsMgr->IncrCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT);
        return;
    }

    ScanDirectory(dirWrapper, dirStat);
    return;
}

void ObjectMetaProducer::ScanDirectory(DirMetaWrapper& dirWrapper, DirStat& dirStat)
{
    uint64_t obsTotal = 0;
    DirectoryScan subDirNode {};
    std::string marker = "";
    bool isTruncated = true;
    std::string fullPath = GetObjectDirName(dirWrapper.m_xMeta);
    INFOLOG("fullPath %s", fullPath.c_str());
    std::unique_ptr<ListObjectsRequest> req = std::make_unique<ListObjectsRequest>();
    FillListObjectsReq(req);
    for (uint32_t loopCnt = 0, retryTimes = 0, obsCnt = 0; isTruncated && (m_state != SCANNER_STATUS::ABORTED);) {
        std::unique_ptr<ListObjectsResponse> resp = nullptr;
        req->marker = marker;
        req->encodeEnable = m_config.encodeEnable;
        OBSResult ret = m_obsCtx->ListObjects(req, resp);
        if (!ret.IsSucc()) {
            // 并发量太大，可能会导致服务端限速报错，需要降低请求速度
            if ((ret.GetLinuxErrorCode() == EAGAIN) && (retryTimes < MAX_RETRY_CNT)) {
                retryTimes++;
                WARNLOG("Need retry for %s", dirStat.m_path.c_str());
                Module::SleepFor(std::chrono::seconds(RETRY_INTERVAL_TIME));
                continue;
            }
            m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
            m_statsMgr->RecordErrMessage(ret.GetLinuxErrorCode(), ret.errorCode, ret.errorDesc);
            ERRLOG("Failed to list objects: %s, err:%s, desc:%s", fullPath.c_str(), ret.errorCode.c_str(),
                ret.errorDesc.c_str());
            return;
        }
        DBGLOG("liset object success");

        isTruncated = resp->isTruncated;  // false表示已经列举完毕
        DBGLOG("isTruncated %d", isTruncated);
        HandleObjectContent(resp, isTruncated);

        marker = resp->nextMarker;
        loopCnt++;
        INFOLOG("LoopCnt: %u, isTruncated: %d, marker: %s", loopCnt, isTruncated, marker.c_str());
        if (loopCnt >= MEM_TRIM_THRESHOLD) {
            loopCnt = 0;
            FS_SCANNER::MemoryTrim();
        }
    }
    return;
}

int ObjectMetaProducer::HandleObjectContent(std::unique_ptr<ListObjectsResponse>& resp, bool isTruncated)
{
    int flag = 0;
    for (ListObjectsContent& object : resp->contents) {
        if (object.key.back() == '/') {
            continue;
        }
            FileStat fileStat {};
            SetFileStat(fileStat, object.key, object.etag, object.lastModified, object.size);
            FindFirstDifference(fileStat.m_key);
            FileMetaWrapper fileWrapper {};
            int ret = WrapFile(fileWrapper, fileStat);
            if (ret != Module::SUCCESS) {
                if (ret != ENOENT) {
                    RecordFailedScanEntry(fileStat.m_path, ret, m_statsMgr);
                    m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_FILES);
                }
                return ret;
            }
            size_t lastSlash = fileStat.m_key.rfind('/');
            int size = m_directoryMap[m_fullPath + fileStat.m_key.substr(0, lastSlash+1)].m_fmWrapperList->size();
            if (size >= MAX_FILES_NUM_OF_DIR) {
                std::string path = m_fullPath + fileStat.m_key.substr(0, lastSlash+1);
                CreateNewNode(path);
            }
            m_directoryMap[m_fullPath + fileStat.m_key.substr(0, lastSlash+1)].m_fmWrapperList->push(fileWrapper);
            DBGLOG("filepath %s", (m_fullPath + fileStat.m_key.substr(0, lastSlash+1)).c_str());
            m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FILES);
            m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_SIZE, fileStat.m_size);
            m_lastPath = fileStat.m_key;
    }
    if (!isTruncated) {
        HandleLastPath();
    }
    return 0;
}

void ObjectMetaProducer::CreateNewNode(std::string path)
{
    DBGLOG("path %s", path.c_str());
    DirectoryScan node;
    node.m_dmWrapper = m_directoryMap[path].m_dmWrapper;
    
    m_directoryMap[path].m_isDirScanCompleted = true;
    m_directoryMap[path].m_isResumeCalled = 1;
    m_output->Push(m_directoryMap[path]);
    m_directoryMap.erase(path);
    m_directoryMap[path] = node;
    return;
}

bool ObjectMetaProducer::HandleLastPath()
{
    for (auto &it : m_directoryMap) {
        DBGLOG("dirpath %s , %d , %lld", it.first.c_str(), it.second.m_fmWrapperList->size(),
            it.second.m_dmWrapper.m_meta.m_inode);
        DirectoryScan node = it.second;
        node.m_isDirScanCompleted = true;
        node.m_isResumeCalled = 1;
        m_output->Push(node);
        m_statsMgr->IncrCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT);
    }
    m_directoryMap.clear();
    return true;
}

bool ObjectMetaProducer::FindFirstDifference(const std::string comparedpath)
{
    size_t i = MakeDirStat(comparedpath);

    if (m_lastPath.empty()) {
        m_lastPath = comparedpath;
        return true;
    }
 
    size_t lastSlash = GetBucketPath(m_lastPath).find('/', i);
    if (lastSlash == std::string::npos) {
        DBGLOG("m_lastPath %s dont need to close", m_lastPath.c_str());
    }
    
    while (lastSlash != std::string::npos) {
        std::string path = GetBucketPath(m_lastPath).substr(0, lastSlash+1);
        DirectoryScan node = m_directoryMap[GetBucketPath(m_lastPath).substr(0, lastSlash+1)];
        node.m_isDirScanCompleted = true;
        node.m_isResumeCalled = 1;
        DBGLOG("dirpath %s , %d , %lld", path.c_str(), node.m_fmWrapperList->size(), node.m_dmWrapper.m_meta.m_inode);
        m_output->Push(node);
        m_directoryMap.erase(GetBucketPath(m_lastPath).substr(0, lastSlash+1));
        m_statsMgr->IncrCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT);
        size_t Slash = lastSlash;
        lastSlash = GetBucketPath(m_lastPath).find('/', Slash+1);
    }
    return true;
}

size_t ObjectMetaProducer::MakeDirStat(std::string path)
{
    size_t slash;
    size_t i = 0;
    std::string lastPath = GetBucketPath(m_lastPath);
    std::string comparedpath = GetBucketPath(path);
    
    size_t minLength = std::min(lastPath.size(), comparedpath.size());
    while (i < minLength && lastPath[i] == comparedpath[i]) {
        ++i;
    }
    if (comparedpath[i] == '/') {
        slash = comparedpath.rfind('/', i-1);
    } else {
        slash = comparedpath.rfind('/', i);
    }

    std::string diffPath =  comparedpath.substr(slash+1);
    std::string prePath = comparedpath.substr(0, slash+1);

    std::string fullPath;
    if (m_directoryMap.count(prePath) == 0) {
        fullPath = m_fullPath;
        diffPath = path;
        prePath = m_fullPath;
    } else {
        DirMetaWrapper frontDirWrapper = m_directoryMap[prePath].m_dmWrapper;
        fullPath = GetObjectDirName(frontDirWrapper.m_xMeta);
    }

    std::set<std::string> parentPrefix{};
    GetParentPrefix(diffPath, parentPrefix);

    for (auto &path : parentPrefix) {
        DirStat dirStat {};
        dirStat.m_path = path;
        dirStat.m_prefix = fullPath;
        dirStat.flag = static_cast<uint8_t>(DirStatFlag::PREFIX);

        DirMetaWrapper dirWrapper {};
        int ret = WrapDirectory(dirWrapper, dirStat);
        m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_DIRS);
        if (ret != Module::SUCCESS) {
            m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
            ERRLOG("Wrap directory failed. %s", dirStat.m_path.c_str());
        }
        fullPath = GetObjectDirName(dirWrapper.m_xMeta);

        DirectoryScan node {};
        node.m_dmWrapper = dirWrapper;
        m_directoryMap[prePath+dirStat.m_path] = node;
        DBGLOG("create new dirStat %s", (prePath+dirStat.m_path).c_str());
    }

    return i;
}

void ObjectMetaProducer::GetParentPrefix(std::string prefix, std::set<std::string>& parentPrefix)
{
    const std::string it = prefix;
    size_t pos = it.find(m_bucket.delimiter, 0);
    while (pos != std::string::npos) {
        std::string parent = it.substr(0, pos + m_bucket.delimiter.length());
        parentPrefix.insert(parent);
        pos = it.find(m_bucket.delimiter, pos + m_bucket.delimiter.length());
    }
    return;
}

std::string ObjectMetaProducer::GetBucketPath(std::string key)
{
    return m_fullPath + key;
}

void ObjectMetaProducer::PushDirToWriteQueue(DirectoryScan& node, const DirMetaWrapper dirWrapper)
{
    node.m_dmWrapper = dirWrapper;

    if (!m_config.enableProduce) {
        DBGLOG("push not enabled");
        m_statsMgr->IncrCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT);
        return;
    }

    bool pushFlag = false;
    while (!pushFlag) {
        if (m_chkPntMgr->GetCheckPointStatus() == CHECKPOINT_STATUS::IN_PROGRESS) {
            m_output->Push(node);
            pushFlag = true;
        } else {
            pushFlag = m_output->BlockingPush(node, BLOCKING_TIMEOUT);
        }
    }

    return;
}

int ObjectMetaProducer::GetObjectMetaData(std::string &bucketName, std::string &key, ObjectMetaData &metaDataInfo)
{
    // 获取对象元数据
    std::unique_ptr<GetObjectMetaDataRequest> req = std::make_unique<GetObjectMetaDataRequest>();
    std::unique_ptr<GetObjectMetaDataResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make object meta data request failed.");
        return Module::FAILED;
    }
    req->bucketName = bucketName;
    req->key = key;
    req->encodeEnable = m_config.encodeEnable;
    OBSResult ret = m_obsCtx->GetObjectMetaData(req, resp);
    if (ret.result != ResultType::SUCCESS) {
        ERRLOG("Failed to get meta data of %s", key.c_str());
        m_statsMgr->RecordErrMessage(ret.GetLinuxErrorCode(), ret.errorCode, ret.errorDesc);
        return ret.GetLinuxErrorCode();
    }

    metaDataInfo.size = resp->size;
    metaDataInfo.lastModified = resp->lastModified;
    metaDataInfo.etag = resp->etag;
    return Module::SUCCESS;
}

int ObjectMetaProducer::GetBucketACL(std::string &bucketName, std::string &aclText)
{
    std::unique_ptr<GetBucketACLRequest> req = std::make_unique<GetBucketACLRequest>();
    std::unique_ptr<GetBucketACLResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make bucket ACL request failed.");
        return Module::FAILED;
    }
    req->bucketName = bucketName;
    OBSResult ret = m_obsCtx->GetBucketACL(req, resp);
    if (ret.result != ResultType::SUCCESS) {
        ERRLOG("GetBucketACL of %s, %s, %s", bucketName.c_str(), ret.errorCode.c_str(), ret.errorDesc.c_str());
        m_statsMgr->RecordErrMessage(ret.GetLinuxErrorCode(), ret.errorCode, ret.errorDesc);
        return ret.GetLinuxErrorCode();
    }

    for (ACLGrant& aclContent : resp->aclGrants) {
        aclText += aclContent.userId + "," + aclContent.userType + "," + std::to_string(aclContent.grantType) + ","
                + std::to_string(aclContent.permission) + "," + std::to_string(aclContent.bucketDelivered) + "\n";
    }

    return Module::SUCCESS;
}

int ObjectMetaProducer::GetObjectACL(std::string &bucketName, std::string &key, std::string &aclText)
{
    std::unique_ptr<GetObjectACLRequest> req = std::make_unique<GetObjectACLRequest>();
    std::unique_ptr<GetObjectACLResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make bucket ACL request failed.");
        return Module::FAILED;
    }
    req->bucketName = bucketName;
    req->key = key;
    req->encodeEnable = m_config.encodeEnable;
    OBSResult ret = m_obsCtx->GetObjectACL(req, resp);
    if (ret.result != ResultType::SUCCESS) {
        ERRLOG("Failed to get object ACL of %s", key.c_str());
        m_statsMgr->RecordErrMessage(ret.GetLinuxErrorCode(), ret.errorCode, ret.errorDesc);
        return ret.GetLinuxErrorCode();
    }

    for (ACLGrant& aclContent : resp->aclGrants) {
        aclText += aclContent.userId + "," + aclContent.userType + "," + std::to_string(aclContent.grantType) + ","
                + std::to_string(aclContent.permission) + "," + std::to_string(aclContent.bucketDelivered) + "\n";
    }

    return Module::SUCCESS;
}

int ObjectMetaProducer::FillFileMetaWrapper(FileStat& fileStat, FileMetaWrapper &fileWrapper)
{
    CopyStatToFileMeta(fileWrapper.m_meta, fileStat);
    std::string fileName = GetObjectFileName(fileWrapper.m_meta);
    fileStat.m_path = fileName;

    XMetaField name {};
    name.m_xMetaType = XMETA_TYPE::XMETA_TYPE_NAME;
    name.m_value = fileName;
    fileWrapper.m_xMeta.emplace_back(name);

    DBGLOG("Origin file name: %s, after hash: %s", fileStat.m_key.c_str(), name.m_value.c_str());

    // 对象的key需要保存，恢复时使用
    XMetaField obsKey {};
    obsKey.m_xMetaType = XMETA_TYPE::XMETA_TYPE_KEY;
    obsKey.m_value = fileStat.m_key;

    memset_s(fileWrapper.m_filePathHash.sha1, SHA_DIGEST_LENGTH + 1, 0x0, SHA_DIGEST_LENGTH + 1);
    SHA1(reinterpret_cast<const unsigned char *>(obsKey.m_value.c_str()),
        obsKey.m_value.length(),
        fileWrapper.m_filePathHash.sha1);
    fileWrapper.m_xMeta.emplace_back(obsKey);

    XMetaField etagFld {};
    etagFld.m_xMetaType = XMETA_TYPE::XMETA_TYPE_EXTEND_ATTRIBUTES;
    etagFld.m_value = "etag:" + fileStat.m_etag;
    fileWrapper.m_xMeta.emplace_back(etagFld);

    return Module::SUCCESS;
}

int ObjectMetaProducer::WrapDirectory(DirMetaWrapper &dirWrapper, DirStat& dirStat)
{
    // 设置 meta
    CopyStatToDirMeta(dirWrapper.m_meta, dirStat);
    DBGLOG("Origin dir name: %lld, ", dirWrapper.m_meta.m_inode);

    // 设置 xmeta
    XMetaField nameAttr {};
    nameAttr.m_xMetaType = XMETA_TYPE::XMETA_TYPE_NAME;
    nameAttr.m_value = MakeObjectDirName(dirStat);
    dirWrapper.m_xMeta.emplace_back(nameAttr);

    XMetaField obsKey {};
    obsKey.m_xMetaType = XMETA_TYPE::XMETA_TYPE_KEY;
    obsKey.m_value = dirStat.m_path;
    dirWrapper.m_xMeta.emplace_back(obsKey);

    DBGLOG(" nameAttr.m_value: %s, obsKey.m_value: %s ,dirStat.m_prefix %s, dirStat.m_path %s",
        nameAttr.m_value.c_str(),
        obsKey.m_value.c_str(),
        dirStat.m_prefix.c_str(),
        dirStat.m_path.c_str());

    if (dirStat.flag != static_cast<uint8_t>(DirStatFlag::BUCKET)) {
        return Module::SUCCESS;
    }

    if (!m_config.scanAcl) {
        return Module::SUCCESS;
    }

    std::string aclText;
    int ret = GetBucketACL(m_bucket.bucketName, aclText);
    if (ret != Module::SUCCESS) {
        return ret;
    }

    XMetaField field {};
    field.m_xMetaType = XMETA_TYPE::XMETA_TYPE_ACL;
    field.m_value = aclText;
    dirWrapper.m_xMeta.emplace_back(field);
    return Module::SUCCESS;
}

int ObjectMetaProducer::WrapFile(FileMetaWrapper &fileWrapper, struct FileStat& fileStat)
{
    if (fileStat.m_key.empty()) {
        ERRLOG("Object key is NULL, can not get meta.");
        return Module::FAILED;
    }

    int errCode = FillFileMetaWrapper(fileStat, fileWrapper);
    if (errCode != Module::SUCCESS) {
        return errCode;
    }

    if (!m_config.scanAcl) {
        return Module::SUCCESS;
    }

    std::string aclText;
    int ret = GetObjectACL(m_bucket.bucketName, fileStat.m_key, aclText);
    if (ret != Module::SUCCESS) {
        return ret;
    }

    XMetaField field {};
    field.m_xMetaType = XMETA_TYPE::XMETA_TYPE_ACL;
    field.m_value = aclText;
    fileWrapper.m_xMeta.emplace_back(field);
    return Module::SUCCESS;
}

ObjectStorageBucket ObjectMetaProducer::GetObsBucketConfig(std::string dirPath)
{
    for (auto &bucket : m_config.obs.buckets) {
        return bucket;
    }
    return ObjectStorageBucket();
}

void ObjectMetaProducer::SetFileStat(FileStat &fileStat, std::string &key, std::string etag, uint64_t lastModified,
    uint64_t size)
{
    fileStat.m_key =  key;
    fileStat.m_etag = etag;
    fileStat.m_mtime = lastModified;
    fileStat.m_size = size;
}


void ObjectMetaProducer::FillListObjectsReq(const std::unique_ptr<ListObjectsRequest>& req)
{
    if (!m_scanPreFix.empty()) {
        req->prefix = m_scanPreFix;
    } else {
        req->prefix = "";
    }

    req->bucketName = m_bucket.bucketName;
    req->delimiter = "";
    req->maxkeys = m_config.obs.onceListNum;
    req->marker = "";

    INFOLOG("buckname %s delimiter %s perfix %s", m_bucket.bucketName.c_str(), m_bucket.delimiter.c_str(),
        req->prefix.c_str());
}

void ObjectMetaProducer::HandleScannedDir(DirMetaWrapper& dirWrapper, DirStat& dirStat, DirectoryScan& subDirNode,
    uint64_t obsTotal)
{
    if ((dirWrapper.m_meta.m_subDirsCnt != 0) || (obsTotal != 0)) {
        subDirNode.m_isDirScanCompleted = true;
        subDirNode.m_isResumeCalled = (obsTotal >= MAX_FILES_NUM_OF_DIR) ? 1 : 0;
        PushDirToWriteQueue(subDirNode, dirWrapper);
    } else {
        m_statsMgr->IncrCommStatsByType(CommStatsType::FILTER_DISCARD_DIR_COUNT);
    }
    EraseObjFromSIPList(dirStat.m_path, dirStat.m_filterFlag);
    m_statsMgr->IncrCommStatsByType(CommStatsType::OPEN_DIR_REQUEST_COUNT);
    DBGLOG("total files: %llu for %s", obsTotal, dirStat.m_path.c_str());
}

bool ObjectMetaProducer::SkipDirEntry(const std::string &name, const std::string &fullPath) const
{
    if (std::find(m_config.skipDirs.begin(), m_config.skipDirs.end(), name) != m_config.skipDirs.end()) {
        return true;
    }

    if (fullPath.length() > SCANNER_PATH_LEN_MAX) {
        ERRLOG("Skipping dir %s, because path exceeded 4096",  fullPath.c_str());
        return true;
    }

    return false;
}

std::string ObjectMetaProducer::ReplaceSlashWithDelimiter(std::string &path)
{
    std::string delimiter = m_bucket.delimiter;
    std::string dst = path;

    std::size_t pos = 0;
    while ((pos = dst.find(PATH_SEPERATOR, pos)) != std::string::npos) {
        dst.replace(pos, PATH_SEPERATOR.length(), delimiter);
        pos += delimiter.length();
    }

    return dst;
}

int ObjectMetaProducer::HandleFileStat(DirectoryScan& subDirNode, DirMetaWrapper& dirWrapper,
    FileStat &fileStat, uint32_t& obsCnt)
{
    if (fileStat.m_key.back() == '/') {
        return Module::SUCCESS;
    }
    
    fileStat.m_prefix = GetObjectDirName(dirWrapper.m_xMeta);
    m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FILES);
    m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_SIZE, fileStat.m_size);

    FileMetaWrapper fileWrapper {};
    int ret = WrapFile(fileWrapper, fileStat);
    if (ret != Module::SUCCESS) {
        if (ret != ENOENT) {
            RecordFailedScanEntry(fileStat.m_path, ret, m_statsMgr);
            m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_FILES);
        }
        return ret;
    }
    subDirNode.m_fmWrapperList->push(fileWrapper);

    obsCnt++;
    if (obsCnt >= MAX_FILES_NUM_OF_DIR) {
        subDirNode.m_isDirScanCompleted = false;
        if (m_config.enableProduce) {
            m_statsMgr->IncrCommStatsByType(CommStatsType::WRITE_QUEUE_DIRECTLY_PUSH_COUNT);
            PushDirToWriteQueue(subDirNode, dirWrapper);
        }
        obsCnt = 0;
        subDirNode = {};
    }

    return Module::SUCCESS;
}

int ObjectMetaProducer::HandleSubPrefixFile(std::string pathName, DirMetaWrapper &dirWrapper, std::string &parentPath)
{
    std::ifstream readFd {};
    std::string fileName = parentPath + PATH_SEPERATOR + pathName;
    readFd.open(fileName.c_str(), std::ios::in);
    if (!readFd.is_open()) {
        char errmsg[MAX_ERROR_MSG_LEN];
        strerror_r(errno, errmsg, MAX_ERROR_MSG_LEN);
        ERRLOG("Read %s failed, ERR: %s", fileName.c_str(), errmsg);
        return Module::FAILED;
    }
    DirectoryScan node {};
    uint32_t obsCnt = 0;
    std::string curLine {};
    while (getline(readFd, curLine)) {
        if (m_state == SCANNER_STATUS::ABORTED) {
            WARNLOG("Abort scan.");
            break;
        }
        size_t pos = curLine.rfind(",");
        std::string key = curLine.substr(0, pos);
        std::string operType = curLine.substr(pos + 1);
        if (operType == OBS_OPERATE_TYPE_DELETE) {
            continue;
        }

        ObjectMetaData metaDataInfo {};
        GetObjectMetaData(m_bucket.bucketName, key, metaDataInfo);

        FileStat fileStat {};
        SetFileStat(fileStat, key, metaDataInfo.etag, metaDataInfo.lastModified, metaDataInfo.size);
        HandleFileStat(node, dirWrapper, fileStat, obsCnt);
    }
    readFd.close();
    
    if ((dirWrapper.m_meta.m_subDirsCnt != 0) || (obsCnt != 0)) {
        node.m_isDirScanCompleted = true;
        node.m_isResumeCalled = 1; // 固定设置为1不影响
        PushDirToWriteQueue(node, dirWrapper);
        DBGLOG("HandleSubPrefixFile %s file count %u", GetObjectPath(dirWrapper.m_xMeta).c_str(), obsCnt);
        m_curDirIsPushed = true;
    }
    return Module::SUCCESS;
}

int ObjectMetaProducer::ReadDirEntry(const struct dirent *direntry, DirStat& dirStat, DirMetaWrapper &dirWrapper,
    std::string &parentPath)
{
    std::string logPath = parentPath + PATH_SEPERATOR + direntry->d_name;
    std::string dataPath = GetObjectDirName(dirWrapper.m_xMeta);
    if (SkipDirEntry(direntry->d_name, logPath)) {
        return Module::SUCCESS;
    }
    struct stat statbuf;
    if (::lstat(logPath.c_str(), &statbuf) < 0) {
        ERRLOG("Failed to stat path: %s", logPath.c_str());
        return Module::FAILED;
    }
    if (S_ISDIR(statbuf.st_mode)) {
        m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_DIRS);
        DirStat subDirStat {};
        std::string subDirPath;
        if (dirStat.m_originPrefix.empty()) {
            subDirPath = direntry->d_name;
        } else {
            subDirPath = GetFullPath(dirStat.m_originPrefix, direntry->d_name);
        }
        // 由对象key： a_b_c_1.txt 按"_"分割产生目录a、b、c，然后遍历形成的全路径为a/b/c/1.txt，但实际为a_b_c_1.txt
        subDirStat.m_path = ReplaceSlashWithDelimiter(subDirPath) + m_bucket.delimiter;
        subDirStat.m_prefix = dataPath;
        subDirStat.m_originPrefix = subDirPath;
        subDirStat.flag = static_cast<uint8_t>(DirStatFlag::PREFIX);
        m_scanQueue->Push(subDirStat);
        dirWrapper.m_meta.m_subDirsCnt++;
    } else { // 只会有一个文件为 SUBPREFIX_FILE
        HandleSubPrefixFile(direntry->d_name, dirWrapper, parentPath);
        EraseObjFromSIPList(dirStat.m_path, dirStat.m_filterFlag);
        m_statsMgr->IncrCommStatsByType(CommStatsType::OPEN_DIR_REQUEST_COUNT);
    }
    return Module::SUCCESS;
}

void ObjectMetaProducer::HandleUserPrefix(DirMetaWrapper& dirWrapper, DirStat& dirStat)
{
    // 不需要增加 CommStatsType::TOTAL_DIRS，原来的已经加过了
    DirStat subDirStat {};
    subDirStat.m_path = dirStat.m_path + m_bucket.delimiter;
    subDirStat.m_prefix = GetObjectDirName(dirWrapper.m_xMeta);
    subDirStat.m_originPrefix = dirStat.m_path;
    subDirStat.flag = static_cast<uint8_t>(DirStatFlag::PREFIX);
    m_scanQueue->Push(subDirStat);
    dirWrapper.m_meta.m_subDirsCnt++;

    DirectoryScan node {};
    node.m_isDirScanCompleted = true;
    node.m_isResumeCalled = 1; // 固定设置为1不影响
    DBGLOG("HandleUserPrefix %s", GetObjectPath(dirWrapper.m_xMeta).c_str());
    m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_DIRS);
    PushDirToWriteQueue(node, dirWrapper);
    m_statsMgr->IncrCommStatsByType(CommStatsType::OPEN_DIR_REQUEST_COUNT);
}

void ObjectMetaProducer::ScanObjectLogContent(DirMetaWrapper& dirWrapper, DirStat& dirStat)
{
    std::string dirPath = m_bucket.logDir + PATH_SEPERATOR + m_bucket.bucketName;
    if (dirStat.m_originPrefix.empty()) {
        if (dirStat.flag != static_cast<uint8_t>(DirStatFlag::BUCKET)) {
            // 用 prefix+delimiter替换prefix
            if (!FS_SCANNER::IsEndsWith(dirStat.m_path, m_bucket.delimiter)) {
                HandleUserPrefix(dirWrapper, dirStat);
                return;
            }
            size_t len = dirStat.m_path.length() - m_bucket.delimiter.length();
            std::string pathWithoutDelimiter = dirStat.m_path.substr(0, len);
            dirPath += PATH_SEPERATOR + pathWithoutDelimiter;
            dirStat.m_originPrefix = pathWithoutDelimiter;
        }
    } else {
        dirPath += PATH_SEPERATOR + dirStat.m_originPrefix;
    }

    INFOLOG("Scan object log content for %s", dirPath.c_str());

    if (!FS_SCANNER::PathExist(dirPath)) {
        INFOLOG("No log key for prefix %s", dirPath.c_str());
        m_statsMgr->IncrCommStatsByType(CommStatsType::FILTER_DISCARD_DIR_COUNT);
        return;
    }

    DIR *dir = ::opendir(dirPath.c_str());
    if (dir == nullptr) {
        m_statsMgr->IncrCommStatsByType(CommStatsType::TOTAL_FAILED_DIRS);
        RecordFailedScanEntry(dirPath, errno, m_statsMgr);
        ERRLOG("Failed to open dir: %s", dirPath.c_str());
        return;
    }

    m_curDirIsPushed = false;
    struct dirent *direntry = nullptr;
    while ((direntry = readdir(dir)) != nullptr) {
        if (m_state == SCANNER_STATUS::ABORTED) {
            WARNLOG("Abort scan.");
            break;
        }
        ReadDirEntry(direntry, dirStat, dirWrapper, dirPath);
    }
    if (::closedir(dir) < 0) {
        WARNLOG("Cannot close directory %s", dirPath.c_str());
    }
    if (!m_curDirIsPushed) {
        m_statsMgr->IncrCommStatsByType(CommStatsType::FILTER_DISCARD_DIR_COUNT);
    }
    EraseObjFromSIPList(dirStat.m_path, dirStat.m_filterFlag);
    m_statsMgr->IncrCommStatsByType(CommStatsType::OPEN_DIR_REQUEST_COUNT);
    return;
}
