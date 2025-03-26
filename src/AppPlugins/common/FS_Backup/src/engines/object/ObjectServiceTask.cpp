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
#include "ObjectServiceTask.h"
#include <ctime>
#include <sys/acl.h>
#include <sys/xattr.h>
#include "log/Log.h"
#include "system/System.hpp"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const std::string SYS_DEF_META_DATA_HEAD = "sys:";
    const std::string USER_DEF_META_DATA_HEAD = "user:";
    const int ACL_FIELD_NUM = 5;
}

void ObjectWriteDataCb(uint64_t partSize, int partNum, void *priData)
{
    if (priData == nullptr) {
        return;
    }
    auto writeCbData = (UploadObjectCbData *)priData;
    writeCbData->controlInfo->m_noOfBytesCopied += partSize;
    DBGLOG("Total parts: %d, finish %d parts, file name: %s",
        writeCbData->maxBlockNum, partNum, writeCbData->fileName.c_str());
}

ObjectServiceTask::ObjectServiceTask(ObjectEvent event, std::shared_ptr<BlockBufferMap> bufferMapPtr,
    FileHandle& fileHandle, const ObjectServiceParams& params)
    : m_event(event), m_bufferMapPtr(bufferMapPtr), m_fileHandle(fileHandle), m_params(params)
{
    m_obsCtx = CloudServiceManager::CreateInst(m_params.authArgs);
}

ObjectServiceTask::~ObjectServiceTask()
{
    if (m_bufferMapPtr != nullptr) {
        m_bufferMapPtr.reset();
    }
}

void ObjectServiceTask::Exec()
{
    HCPTSP::getInstance().reset(m_params.reqID);
    if (m_bufferMapPtr == nullptr) {
        return;
    }

    if (m_obsCtx == nullptr) {
        ERRLOG("m_obsCtx is null!");
        m_result = Module::FAILED;
        return;
    }
    switch (m_event) {
        case ObjectEvent::OPEN_DST: {
            HandleOpenDst();
            return;
        }
        case ObjectEvent::READ_DATA: {
            HandleReadData();
            return;
        }
        case ObjectEvent::READ_META: {
            HandleReadMeta();
            return;
        }
        case ObjectEvent::WRITE_DATA: {
            HandleWriteData();
            return;
        }
        case ObjectEvent::WRITE_META: {
            HandleWriteMeta();
            return;
        }
        case ObjectEvent::CLOSE_DST: {
            HandleCloseDst();
            return;
        }
        case ObjectEvent::DELETE_ITEM: {
            HandleDelete();
            return;
        }
        case ObjectEvent::CREATE_DIR: {
            HandleCreateBucket();
            return;
        }
        default:
            break;
    }
    return;
}

bool ObjectServiceTask::IsRestore()
{
    if ((m_params.backupType == BackupType::RESTORE) || (m_params.backupType == BackupType::FILE_LEVEL_RESTORE)) {
        return true;
    }

    return false;
}

bool ObjectServiceTask::IsBucketName(std::string& path)
{
    for (auto &item : m_params.bucketNames) {
        if (path.substr(1) == item.bucketName) {
            return true;
        }
    }
    return false;
}

std::string ObjectServiceTask::GetBucketName()
{
    if (IsRestore() && !m_params.dstBucket.bucketName.empty()) {
        return m_params.dstBucket.bucketName;
    }

    std::string path = m_fileHandle.m_file->m_fileName;
    for (auto &item : m_params.bucketNames) {
        // 不包含首字符"/"路径分割符
        if (path.substr(1, item.bucketName.length()) == item.bucketName) {
            return item.bucketName;
        }
    }
    return "";
}

bool ObjectServiceTask::GetEncode(const std::string& bucketName)
{
    if (IsRestore() && !m_params.dstBucket.bucketName.empty()) {
        return m_params.dstBucket.encodeEnable;
    }
    for (auto &item : m_params.bucketNames) {
        if (item.bucketName == bucketName) {
            return item.encodeEnable;
        }
    }
    return false;
}
/*
 * 如果指定了恢复的prefix，则恢复的key修改为 prefix + delimiter + key
 * 指定的prefix最多只有1个
 */
std::string ObjectServiceTask::GetObjectKey()
{
    if (!IsRestore()) {
        return m_fileHandle.m_file->m_obsKey;
    }

    std::string newPrefix {};
    if (!m_params.dstBucket.prefix.empty()) {
        newPrefix += m_params.dstBucket.prefix.front() + m_params.dstBucket.delimiter;
    }
    return newPrefix + m_fileHandle.m_file->m_obsKey;
}

bool ObjectServiceTask::IsCriticalError() const
{
    if (m_backupFailReason == BackupPhaseStatus::FAILED_NOACCESS ||
        m_backupFailReason == BackupPhaseStatus::FAILED_NOSPACE ||
        m_backupFailReason == BackupPhaseStatus::FAILED_NOMEMORY ||
        m_backupFailReason == BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE ||
        m_backupFailReason == BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE) {
        ERRLOG("Backup fail readson: %d", static_cast<int>(m_backupFailReason));
        return true;
    }
    return false;
}

void ObjectServiceTask::HandleCreateBucket()
{
    std::string dstBucketName = GetBucketName();
    std::unique_ptr<HeadBucketRequest> req = std::make_unique<HeadBucketRequest>();
    std::unique_ptr<HeadBucketResponse> resp = nullptr;
    req->bucketName = dstBucketName;
    OBSResult ret = m_obsCtx->IsBucketExist(req, resp);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Create bucket(%s) failed. errorCode: %lld, errorDesc: %s",
            req->bucketName.c_str(), errorCode, ret.errorDesc.c_str());
        m_result = Module::FAILED;
        return;
    }

    if (!resp->isExist) {
        m_result = Module::SUCCESS;
        std::unique_ptr<CreateBucketRequest> createReq = std::make_unique<CreateBucketRequest>();
        createReq->bucketName = dstBucketName;
        OBSResult createRet = m_obsCtx->CreateBucket(createReq);
        if (!createRet.IsSucc()) {
            int64_t errorCode = createRet.GetLinuxErrorCode();
            m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
            SetCriticalErrorInfo(errorCode);
            RecordErrMessage(errorCode, createRet.errorDesc);
            ERRLOG("Create bucket name %s failed.", createReq->bucketName.c_str());
            m_result = Module::FAILED;
            return;
        }
    }

    DBGLOG("Create bucket %s success, exist: %d", dstBucketName.c_str(), resp->isExist);

    if (!IsBucketName(m_fileHandle.m_file->m_fileName)) {
        m_result = Module::SUCCESS;
        return;
    }

    if (resp->isExist && m_params.isfineGrainedRestore) {
        m_result = Module::SUCCESS;
        return;
    }
    if (!SetBucketAcl()) {
        m_result = Module::FAILED;
        return;
    }

    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::HandleReadMeta()
{
    if (!m_params.saveMeta) {
        m_result = Module::SUCCESS;
        return;
    }

    std::unique_ptr<GetObjectMetaDataRequest> req = std::make_unique<GetObjectMetaDataRequest>();
    std::unique_ptr<GetObjectMetaDataResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make object meta data request failed.");
        m_result = Module::FAILED;
        return;
    }
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->GetObjectMetaData(req, resp);
    if (ret.result != ResultType::SUCCESS) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to get meta data of %s", req->key.c_str());
        m_result = Module::FAILED;
        return;
    }

    SetObjectMetaIntoXattr(resp->sysDefMetaData, resp->userDefMetaData);
    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::HandleReadData()
{
    // 有可能是空文件
    if (m_fileHandle.m_block.m_size == 0) {
        HandleReadMeta();
        return;
    }

    std::unique_ptr<GetObjectRequest> req = std::make_unique<GetObjectRequest>();
    std::unique_ptr<GetObjectResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make unique failed.");
        m_result = Module::FAILED;
        return;
    }

    DBGLOG("Read data from file %s", m_fileHandle.m_file->m_fileName.c_str());

    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->buffer = m_fileHandle.m_block.m_buffer;
    req->bufferSize = m_fileHandle.m_block.m_size;
    req->startByte = m_fileHandle.m_block.m_offset;
    req->byteCount = m_fileHandle.m_block.m_size;
    req->retryConfig.isRetryable = true;
    req->encodeEnable = GetEncode(req->bucketName);

    OBSResult ret = m_obsCtx->GetObject(req, resp);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Get object %s(offset: %llu, size: %llu) failed, bucket name is %s, filename: %s",
            req->key.c_str(),
            m_fileHandle.m_block.m_offset,
            m_fileHandle.m_block.m_size,
            req->bucketName.c_str(),
            m_fileHandle.m_file->m_fileName.c_str());
        m_result = Module::FAILED;
        return;
    }

    if (m_params.saveMeta) {
        SetObjectMetaIntoXattr(resp->sysDefMetaData, resp->userDefMetaData);
    }
    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::SetCriticalErrorInfo(int64_t err)
{
    if (err == ENOSPC) {
        m_backupFailReason = BackupPhaseStatus::FAILED_NOSPACE;
    }
    if (err == ENETUNREACH) {
        m_backupFailReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
    }
    if (err == EACCES) {
        m_backupFailReason = BackupPhaseStatus::FAILED_NOACCESS;
    }
    if (err == ENOMEM) {
        m_backupFailReason = BackupPhaseStatus::FAILED_NOMEMORY;
    }
    return;
}

void ObjectServiceTask::RecordErrMessage(int64_t errCode, const std::string& errMessage)
{
    if (errCode != ENOSPC && errCode != ENETUNREACH && errCode != EACCES && errCode != ENOMEM) {
        m_fileHandle.m_errMessage = errMessage;
        ERRLOG("Record errorCode: %lld, errorDesc: %s", errCode, errMessage.c_str());
    }
}

bool ObjectServiceTask::IsNeedRestore()
{
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE) {
        return true;
    }

    time_t dstObjectMtime;
    if (GetObjectMtime(dstObjectMtime) != Module::SUCCESS) {
        return true;
    }

    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER) {
        if (m_fileHandle.m_file->m_mtime <= (uint64_t)dstObjectMtime) {
            DBGLOG("skip %s", m_fileHandle.m_file->m_fileName.c_str());
            return false;
        }
    }

    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) {
        DBGLOG("ignore exists file %s", m_fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    return true;
}

void ObjectServiceTask::HandleOpenDst()
{
    if (!IsRestore()) {
        m_result = Module::SUCCESS;
        return;
    }

    if (!IsNeedRestore()) {
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        m_result = Module::SUCCESS;
        return;
    }

    // 对象不存在 或 restoreReplacePolicy为OVERWRITE
    std::unique_ptr<GetUploadIdRequest> req = std::make_unique<GetUploadIdRequest>();
    std::unique_ptr<GetUploadIdResponse> resp = nullptr;
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->GetUploadId(req, resp);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to get uploadid file: %s", m_fileHandle.m_file->m_fileName.c_str());
        m_result = Module::FAILED;
        return;
    }
    m_fileHandle.m_file->m_uploadId = resp->uploadId;
    DBGLOG("open %s success", m_fileHandle.m_file->m_fileName.c_str());
    m_result = Module::SUCCESS;
    return;
}

int ObjectServiceTask::GetObjectMtime(time_t &mtime)
{
    std::unique_ptr<GetObjectMetaDataRequest> req = std::make_unique<GetObjectMetaDataRequest>();
    std::unique_ptr<GetObjectMetaDataResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make object meta data request failed.");
        return Module::FAILED;
    }
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->GetObjectMetaData(req, resp);
    if (!ret.IsSucc()) {
        INFOLOG("No this object or can not get meta data of %s", req->key.c_str());
        RecordErrMessage(ret.GetLinuxErrorCode(), ret.errorDesc);
        return Module::FAILED;
    }

    mtime = resp->lastModified;
    return Module::SUCCESS;
}

int ObjectServiceTask::HandleWriteHugeData()
{
    // 超大文件没有open操作，是直接写数据，因此要在此判断恢复策略
    if (!IsNeedRestore()) {
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        return Module::SUCCESS;
    }

    std::unique_ptr<MultiPartUploadObjectRequest> req = std::make_unique<MultiPartUploadObjectRequest>();
    std::unique_ptr<MultiPartUploadObjectResponse> resp = nullptr;
    if (req == nullptr) {
        ERRLOG("Make object upload request failed.");
        return Module::FAILED;
    }
    std::string fileName = m_fileHandle.m_file->m_fileName;
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->upLoadTargetPath = m_params.dataPath + m_fileHandle.m_file->m_fileName;
    req->partSize = (m_fileHandle.m_file->m_size - 1) / m_params.maxBlockNum + 1; // 按maxBlockNum分割上传块大小，向上取整
    req->enableCheckPoint = false;
    // 保存断点续传的文件，名称不重复即可，成功后会删除
    req->checkPointFilePath = m_params.cachePath + fileName.substr(fileName.rfind(PATH_SEPARATOR) + 1);
    req->callBack = ObjectWriteDataCb;
    req->callBackData = &m_params.writeCbData;
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->MultiPartUploadObject(req, resp);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to MultiPartUploadObject %s", req->key.c_str());
        return Module::FAILED;
    }

    DBGLOG("write %s success!", m_fileHandle.m_file->m_fileName.c_str());
    return Module::SUCCESS;
}

void ObjectServiceTask::SaveUplodInfo(uint32_t partNumber, std::string& etag)
{
    UploadInfo uploadInfo {};
    uploadInfo.partNumber = partNumber;
    uploadInfo.etag = etag;

    std::lock_guard<std::mutex> lock(m_params.m_uploadInfoMap->uploadMapMtx);
    auto it = m_params.m_uploadInfoMap->updInfo.find(m_fileHandle.m_file->m_fileName);
    if (it != m_params.m_uploadInfoMap->updInfo.end()) {
        it->second.push_back(uploadInfo);
        DBGLOG("Push uploadinfo: uploadId %s, partNumber %u, key %s, file %s",
            m_fileHandle.m_file->m_uploadId.c_str(), m_fileHandle.m_block.m_seq,
            m_fileHandle.m_file->m_obsKey.c_str(), m_fileHandle.m_file->m_fileName.c_str());
    } else {
        std::vector<UploadInfo> vec {uploadInfo};
        auto item = std::pair<std::string, std::vector<UploadInfo>>(m_fileHandle.m_file->m_fileName, vec);
        m_params.m_uploadInfoMap->updInfo.insert(item);
        DBGLOG("Insert uploadinfo: uploadId %s, partNumber %u, key %s, file %s",
            m_fileHandle.m_file->m_uploadId.c_str(), m_fileHandle.m_block.m_seq,
            m_fileHandle.m_file->m_obsKey.c_str(), m_fileHandle.m_file->m_fileName.c_str());
    }
    return;
}

int ObjectServiceTask::HandleWriteSmallData()
{
    // 小文件没有open操作，是直接写数据，因此要在此判断恢复策略
    if (!IsNeedRestore()) {
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        return Module::SUCCESS;
    }

    std::unique_ptr<PutObjectPartRequest> req = std::make_unique<PutObjectPartRequest>();
    std::unique_ptr<PutObjectPartResponse> resp = nullptr;
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->startByte = 0;
    req->partSize = m_fileHandle.m_block.m_size;
    req->bufPtr = (char *)m_fileHandle.m_block.m_buffer;
    if (m_params.writeMeta) {
        GetObjectMetaFromXattr(req->sysDefMetaData, req->userDefMetaData);
    }
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->PutObject(req, resp);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to PutObject file: %s", req->key.c_str());
        return Module::FAILED;
    }
    DBGLOG("write %s success!", m_fileHandle.m_file->m_fileName.c_str());

    if (!SetObjectAcl()) {
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

void ObjectServiceTask::HandleWriteData()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        m_result = HandleWriteSmallData();
        return;
    }

    if (m_fileHandle.m_file->IsFlagSet(HUGE_OBJECT_FILE)) {
        m_result = HandleWriteHugeData();
        return;
    }

    if (m_fileHandle.m_file->m_uploadId == "") {
        m_errDetails = {m_fileHandle.m_file->m_obsKey, EBADF};
        ERRLOG("not opened %s errno %d", m_fileHandle.m_file->m_fileName.c_str(), m_errDetails.second);
        m_result = Module::FAILED;
        return;
    }

    std::unique_ptr<PutObjectPartRequest> req = std::make_unique<PutObjectPartRequest>();
    std::unique_ptr<PutObjectPartResponse> resp = nullptr;

    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->partNumber = m_fileHandle.m_block.m_seq; // partNumber范围是 1-10000
    req->uploadId = m_fileHandle.m_file->m_uploadId;
    req->startByte = 0;
    req->partSize = m_fileHandle.m_block.m_size;
    req->bufPtr = (char *)m_fileHandle.m_block.m_buffer;
    DBGLOG("Object key %s, uploadId %s, partNumber %u, startByte %llu, partSize %llu",
        req->key.c_str(), req->uploadId.c_str(), req->partNumber, req->startByte, req->partSize);
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->PutObjectPart(req, resp);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to PutObjectPart file: %s", req->key.c_str());
        m_result = Module::FAILED;
        return;
    }

    if (resp->startByte < m_fileHandle.m_block.m_size) {
        ERRLOG("PutObjectPart write %llu and not finish for key %s", resp->startByte, req->key.c_str());
        m_result = Module::FAILED;
        return;
    }

    SaveUplodInfo(req->partNumber, resp->etag);

    DBGLOG("write %s %d %llu %llu success!", m_fileHandle.m_file->m_fileName.c_str(),
        m_fileHandle.m_block.m_seq, m_fileHandle.m_block.m_offset, m_fileHandle.m_block.m_size);
    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::HandleCloseDst()
{
    std::unique_ptr<CompletePutObjectPartRequest> req = std::make_unique<CompletePutObjectPartRequest>();
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->uploadId = m_fileHandle.m_file->m_uploadId;
    // 写失败时，不需要填充uploadInfo
    if (m_fileHandle.m_file->GetDstState() == FileDescState::WRITE_FAILED) {
        req->isFailed = true;
    } else {
        auto it = m_params.m_uploadInfoMap->updInfo.find(m_fileHandle.m_file->m_fileName);
        if (it == m_params.m_uploadInfoMap->updInfo.end()) {
            ERRLOG("No upload info for %s", req->key.c_str());
            req->isFailed = true;
        } else {
            req->uploadInfo = it->second;
        }
    }
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->CompletePutObjectPart(req);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to close file: %s", m_fileHandle.m_file->m_fileName.c_str());
        m_result = Module::FAILED;
        return;
    }
    DBGLOG("close %s success!", m_fileHandle.m_file->m_fileName.c_str());
    std::lock_guard<std::mutex> lock(m_params.m_uploadInfoMap->uploadMapMtx);
    m_params.m_uploadInfoMap->updInfo.erase(m_fileHandle.m_file->m_fileName);
    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::HandleWriteMeta()
{
    if (!IsRestore()) {
        m_result = Module::SUCCESS;
        return;
    }

    DBGLOG("write meta %s", m_fileHandle.m_file->m_obsKey.c_str());

    if (!SetObjectAcl()) {
        m_result = Module::FAILED;
        return;
    }

    if (!SetObjectMeta()) {
        m_result = Module::FAILED;
        return;
    }

    DBGLOG("write meta %s success!", m_fileHandle.m_file->m_obsKey.c_str());
    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::HandleDelete()
{
    std::unique_ptr<DeleteObjectRequest> req = std::make_unique<DeleteObjectRequest>();
    if (req == nullptr) {
        m_errDetails = {m_fileHandle.m_file->m_obsKey, ENOMEM};
        SetCriticalErrorInfo(ENOMEM);
        ERRLOG("Failed to make delete object request: %s", m_fileHandle.m_file->m_fileName.c_str());
        m_result = Module::FAILED;
        return;
    }
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->DeleteObject(req);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to delete object: %s", req->key.c_str());
        m_result = Module::FAILED;
        return;
    }
    DBGLOG("delete object %s success!", req->key.c_str());
    m_result = Module::SUCCESS;
    return;
}

void ObjectServiceTask::FillAclGrant(std::vector<ACLGrant>& aclGrants)
{
    vector<string> aclText;
    boost::algorithm::split(aclText, m_fileHandle.m_file->m_aclText, boost::is_any_of("\n"), boost::token_compress_off);
    for (auto item : aclText) {
        if (item.empty()) { // 如果分割空串或分割符是最后一个，split结果则会包含空串
            continue;
        }
        vector<string> aclContent;
        boost::algorithm::split(aclContent, item, boost::is_any_of(","), boost::token_compress_off);
        if (aclContent.size() != ACL_FIELD_NUM) {
            ERRLOG("Wrong ACL for %s", m_fileHandle.m_file->m_fileName.c_str());
            continue;
        }
        int idx = 0;
        ACLGrant aclGrant {};
        aclGrant.userId = aclContent[idx++];
        aclGrant.userType = aclContent[idx++];
        aclGrant.grantType = std::stoi(aclContent[idx++]);
        aclGrant.permission = std::stoi(aclContent[idx++]);
        aclGrant.bucketDelivered = std::stoi(aclContent[idx++]);
        aclGrants.emplace_back(aclGrant);
    }
}

int ObjectServiceTask::GetBucketAcl(std::unique_ptr<GetBucketACLResponse>& newAcl)
{
    std::unique_ptr<GetBucketACLRequest> req = std::make_unique<GetBucketACLRequest>();
    if (req == nullptr) {
        ERRLOG("Make bucket ACL request failed.");
        return Module::FAILED;
    }

    req->bucketName = GetBucketName();
    req->isNewGet = true;
    OBSResult ret = m_obsCtx->GetBucketACL(req, newAcl);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to get bucket ACL of %s", req->bucketName.c_str());
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

int ObjectServiceTask::GetObjectAcl(std::unique_ptr<GetObjectACLResponse>& newAcl)
{
    std::unique_ptr<GetObjectACLRequest> req = std::make_unique<GetObjectACLRequest>();
    if (req == nullptr) {
        ERRLOG("Make bucket ACL request failed.");
        return Module::FAILED;
    }
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    req->isNewGet = true;
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->GetObjectACL(req, newAcl);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to get object ACL of %s", req->key.c_str());
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

bool ObjectServiceTask::SetBucketAcl()
{
    if (!m_params.writeAcl) {
        return true;
    }

    if (m_fileHandle.m_file->m_aclText.empty()) {
        ERRLOG("file %s, acl text is empty", m_fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    std::unique_ptr<GetBucketACLResponse> newAcl = nullptr;
    if (GetBucketAcl(newAcl) != Module::SUCCESS) {
        ERRLOG("Get bucket %s acl failed.", m_fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    // call obs api to restore acl
    std::unique_ptr<SetBucketACLRequest> req = std::make_unique<SetBucketACLRequest>();
    req->bucketName = GetBucketName();
    req->ownerId = newAcl->ownerId;
    req->ownerDisplayName = newAcl->ownerDisplayName;
    req->aclGrants.assign(newAcl->aclGrants.begin(), newAcl->aclGrants.end());
    FillAclGrant(req->aclGrants);
    OBSResult ret = m_obsCtx->SetBucketACL(req);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to set bucket ACL for %s", m_fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    DBGLOG("Set bucket %s ACL success.", req->bucketName.c_str());
    return true;
}

bool ObjectServiceTask::SetObjectAcl()
{
    if (!m_params.writeAcl) {
        return true;
    }

    if (m_fileHandle.m_file->m_aclText.empty()) {
        ERRLOG("file %s, acl text is empty", m_fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    std::unique_ptr<GetObjectACLResponse> newAcl = nullptr;
    if (GetObjectAcl(newAcl) != Module::SUCCESS) {
        ERRLOG("Get object %s acl failed.", m_fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    // call obs api to restore acl
    std::unique_ptr<SetObjectACLRequest> req = std::make_unique<SetObjectACLRequest>();
    req->bucketName = GetBucketName();
    req->ownerId = newAcl->ownerId;
    req->ownerDisplayName = newAcl->ownerDisplayName;
    req->key = GetObjectKey();
    req->aclGrants.assign(newAcl->aclGrants.begin(), newAcl->aclGrants.end());
    FillAclGrant(req->aclGrants);
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->SetObjectACL(req);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to set object ACL for %s", req->key.c_str());
        m_result = Module::FAILED;
        return false;
    }

    DBGLOG("Set object %s ACL success.", req->key.c_str());
    return true;
}

bool ObjectServiceTask::IsSkipMetaData(const vector<std::string> &excludeItems, const std::string &meta)
{
    for (auto it : excludeItems) {
        if (it.find(meta) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void ObjectServiceTask::SetObjectMetaIntoXattr(
    std::unordered_map<std::string, std::string> &sysDefMetaData,
    std::unordered_map<std::string, std::string> &userDefMetaData)
{
    m_fileHandle.m_file->LockCommonMutex();
    vector<string> excludeItems;
    boost::algorithm::split(excludeItems, m_params.excludeMeta, boost::is_any_of(","), boost::token_compress_off);
    DBGLOG("Backup exclude meta configuration: %s", m_params.excludeMeta.c_str());
    if (m_fileHandle.m_file->m_xattr.empty()) {
        for (auto &item : sysDefMetaData) {
            if (IsSkipMetaData(excludeItems, item.first)) {
                continue;
            }
            m_fileHandle.m_file->m_xattr.emplace_back(item.first, item.second);
        }
        // 增加一个空数据作为sysDefMetaData与userDefMetaData的分割，实际的元数据不会为空
        m_fileHandle.m_file->m_xattr.emplace_back("", "");
        for (auto &item : userDefMetaData) {
            if (IsSkipMetaData(excludeItems, item.first)) {
                continue;
            }
            m_fileHandle.m_file->m_xattr.emplace_back(item.first, item.second);
        }
    }
    m_fileHandle.m_file->UnlockCommonMutex();
}

void ObjectServiceTask::GetObjectMetaFromXattr(
    std::unordered_map<std::string, std::string> &sysDefMetaData,
    std::unordered_map<std::string, std::string> &userDefMetaData)
{
    DBGLOG("Restore meta for key %s", m_fileHandle.m_file->m_obsKey.c_str());
    bool isSysDef = true;
    for (auto &item : m_fileHandle.m_file->m_xattr) {
        DBGLOG("meta info: (%s,%s)", item.first.c_str(), item.second.c_str());
        if (item.first.empty() && item.second.empty()) {
            isSysDef = false;
            continue;
        }
        if (isSysDef) {
            sysDefMetaData.emplace(item);
        } else {
            userDefMetaData.emplace(item);
        }
    }
    return;
}

bool ObjectServiceTask::SetObjectMeta()
{
    if (!m_params.writeMeta) {
        return true;
    }
    // call obs api to restore meta
    std::unique_ptr<SetObjectMetaDataRequest> req = std::make_unique<SetObjectMetaDataRequest>();
    req->bucketName = GetBucketName();
    req->key = GetObjectKey();
    GetObjectMetaFromXattr(req->sysDefMetaData, req->userDefMetaData);
    req->encodeEnable = GetEncode(req->bucketName);
    OBSResult ret = m_obsCtx->SetObjectMetaData(req);
    if (!ret.IsSucc()) {
        int64_t errorCode = ret.GetLinuxErrorCode();
        m_errDetails = {m_fileHandle.m_file->m_obsKey, errorCode};
        SetCriticalErrorInfo(errorCode);
        RecordErrMessage(errorCode, ret.errorDesc);
        ERRLOG("Failed to meta data for file %s", req->key.c_str());
        return false;
    }

    return true;
}
