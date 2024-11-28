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
#include "UploadFile.h"
#include<sys/stat.h>
#include <thread>
#include <openssl/md5.h>
#include "S3Base.h"
#include "config_reader/ConfigIniReader.h"

using namespace std;
using namespace Module;

namespace {
const string MODULE = "UploadFile";
constexpr int UPLOAD_WITHMD5 = 1;
constexpr int S3_RETRY_TIMES = 5;
constexpr int MD5_BUFFER_SIZE = 16;
constexpr int MD5_BASE64_SIZE = 64;
constexpr int INTERVAL_TIMES = 60;
constexpr int BUFFER_SIZE = 1024;
constexpr int UPLOADID_RETURN_SIZE = 255;
constexpr uint64_t MAX_PART_NUM = 10000;
constexpr uint64_t UPLOAD_PART_SIZE = 5L * 1024 * 1024;
constexpr uint64_t UPLOAD_FILE_SIZE_50000M = 5ULL * 1024 * 1024 * 10000; // 50000M
};  // namespace

UploadPartData UploadQueue::Pop()
{
    unique_lock<std::mutex>mlock(m_mutex);
    while (m_queue.empty()) {
        m_cond.wait(mlock);
    }
    UploadPartData val = m_queue.front();
    m_queue.pop();
    mlock.unlock();
    m_cond.notify_one();
    return val;
}

void UploadQueue::Push(const UploadPartData &uploadData)
{
    unique_lock<std::mutex>mlock(m_mutex);
    while (m_queue.size() >= QUEUE_SIZE) {
        m_cond.wait(mlock);
    }
    m_queue.push(uploadData);
    mlock.unlock();
    m_cond.notify_one();
}

UploadFile::UploadFile(const IODeviceInfo &deviceInfo)
{
    CreateDeviceInfo(deviceInfo);
}

UploadFile::~UploadFile()
{}

void UploadFile::CreateDeviceInfo(const IODeviceInfo& deviceInfo)
{
    m_s3IOInfo.bucketFullName = deviceInfo.path_prefix;
    m_s3IOInfo.passWord = deviceInfo.password;
    m_s3IOInfo.userName = deviceInfo.user_name;
    m_s3IOInfo.protocol = deviceInfo.using_https ? OBS_PROTOCOL_HTTPS : OBS_PROTOCOL_HTTP;
    m_s3IOInfo.uriStyle = deviceInfo.style;
    HCP_Log(DEBUG, MODULE) << "The style is " << m_s3IOInfo.uriStyle << HCPENDLOG;
    string keyStr(":/");
    string::size_type pos =  m_s3IOInfo.bucketFullName.find(keyStr);
    m_s3IOInfo.host = m_s3IOInfo.bucketFullName.substr(0, pos);
    m_s3IOInfo.bucket = m_s3IOInfo.bucketFullName.substr(pos + keyStr.size());
    m_s3IOInfo.cert = deviceInfo.cert;

    m_s3IOInfo.HttpProxyInfo = deviceInfo.HttpProxyInfo;
    m_s3IOInfo.SpeedUpInfo = deviceInfo.SpeedUpInfo;
}

bool UploadFile::SplitToPart(const string &localFile, const string &remoteFile, const string &uploadId)
{
    struct stat statInfo;
    if (stat(localFile.c_str(), &statInfo) != 0) {
        HCP_Log(ERR, MODULE) << "Invoke stat failed." << HCPENDLOG;
        return false;
    }

    // Split policy
    if (statInfo.st_size <= UPLOAD_FILE_SIZE_50000M) {
        m_partInfo.partSize = UPLOAD_PART_SIZE;
        m_partInfo.partNum = (statInfo.st_size % UPLOAD_PART_SIZE == 0) ? (statInfo.st_size / UPLOAD_PART_SIZE)
                                                                        : (statInfo.st_size / UPLOAD_PART_SIZE + 1);
    } else {
        m_partInfo.partSize = statInfo.st_size / MAX_PART_NUM;
        m_partInfo.partNum = MAX_PART_NUM;
    }
    m_partInfo.contentLength = statInfo.st_size;
    m_partInfo.key = remoteFile;
    m_partInfo.uploadId = uploadId;
    return true;
}

void UploadFile::InitPutProperties(obs_put_properties &putProperties)
{
    init_put_properties(&putProperties);
    int fullControl = ConfigReader::getInt("BackupNode", "S3BucketFullControl");
    if (fullControl > 0) {
        putProperties.canned_acl = OBS_CANNED_ACL_BUCKET_OWNER_FULL_CONTROL;
        putProperties.az_redundancy = OBS_REDUNDANCY_1AZ;
    }
}

bool UploadFile::ShouldRetry(int &retryTimes)
{
    HCP_Log(DEBUG, MODULE) << "May will retry later, retryTimes: " << retryTimes << ", interval: " << INTERVAL_TIMES
                           << HCPENDLOG;
    if (retryTimes-- > 0) {
        std::this_thread::sleep_for(chrono::seconds(INTERVAL_TIMES));
        return true;
    }
    return false;
}

obs_status UploadFile::ResponsePropertiesCallback(const obs_response_properties *properties, void *callbackData)
{
    (void)properties;
    (void)callbackData;
    return OBS_STATUS_OK;
}

void UploadFile::ResponseCompleteCallback(obs_status status, const obs_error_details *error, void *callbackData)
{
    if (callbackData == nullptr) {
        HCP_Log(ERR, MODULE) << "CallbackData is null. " << HCPENDLOG;
        return;
    }
    obs_status *ret_status = (obs_status *)callbackData;
    *ret_status = status;
}

obs_status UploadFile::ConcurrentResponsePropertiesCallback(
    const obs_response_properties *properties, void *callbackData)
{
    if (callbackData == nullptr) {
        HCP_Log(ERR, MODULE) << "CallbackData is null. " << HCPENDLOG;
        return OBS_STATUS_InvalidParameter;
    }
    UploadPartData *data = (UploadPartData *)callbackData;
    if (properties->etag) {
        data->etag = properties->etag;
        HCP_Log(DEBUG, MODULE) << "Etag: " << string(properties->etag) << HCPENDLOG;
    }
    for (int i = 0; i < properties->meta_data_count; i++) {
        HCP_Log(DEBUG, MODULE) << "X-amz-meta-" << properties->meta_data[i].name << ": "
                               << properties->meta_data[i].value << HCPENDLOG;
    }
    return OBS_STATUS_OK;
}

obs_status UploadFile::CompleteMultipartUploadCallback(const char *location, const char *bucket, const char *key,
    const char* eTag, void *callbackData)
{
    (void)callbackData;
    HCP_Log(DEBUG, MODULE) << "Location = " << location << ", Bucket = " << bucket << ", Key = " << key
                           << ", ETag = " << eTag << HCPENDLOG;
    return OBS_STATUS_OK;
}

void UploadFile::ConcurrentUploadFileCompleteCallback(
    obs_status status, const obs_error_details *error, void *callbackData)
{
    if (callbackData == nullptr) {
        HCP_Log(ERR, MODULE) << "CallbackData is null. " << HCPENDLOG;
        return;
    }
    UploadPartData *data = (UploadPartData *)callbackData;
    data->retStatus = status;
}

int UploadFile::ConcurrentUploadPartDataCallback(int bufferSize, char *buffer, void *callbackData)
{
    if (callbackData == nullptr) {
        HCP_Log(ERR, MODULE) << "CallbackData is null. " << HCPENDLOG;
        return 0;
    }
    UploadPartData *data = (UploadPartData *)callbackData;

    int iRet = fseek(data->handle.get(), data->partStartPos + data->offsetInPart, SEEK_SET);
    if (iRet != 0) {
        HCP_Log(ERR, MODULE) << "Fseek file failed. errno: " << strerror(errno) << HCPENDLOG;
        return 0;
    }
    int toRead = ((data->partSize > (unsigned)bufferSize) ? (unsigned)bufferSize : data->partSize);
    size_t ret = fread(buffer, 1, toRead, data->handle.get());
    data->offsetInPart += static_cast<uint64_t>(ret);
    if (data->callbackHandle.callBackFunc != nullptr) {
        ((WriteFileCallback)data->callbackHandle.callBackFunc)(
            LayoutRetCode::SUCCESS, ret, 0, data->callbackHandle.callBackData);
    }
    return (int)ret;
}

void UploadFile::DistributeThreadPoc()
{
    uint32_t partCount = m_partInfo.partNum;

    UploadPartData tmpCallbackData;
    for (uint32_t part = 0; part < partCount; part++) {
        tmpCallbackData.partNum = part + 1;
        if ((part != 0) && (ULONG_LONG_MAX / part < m_partInfo.partSize)) {
            HCP_Log(ERR, MODULE) << "Overflow error. " << HCPENDLOG;
            return;
        }
        tmpCallbackData.partStartPos = (m_partInfo.partSize) * part;
        tmpCallbackData.offsetInPart = 0;

        if (part == partCount - 1) {
            tmpCallbackData.partSize = m_partInfo.contentLength - (m_partInfo.partSize) * part;
        } else {
            tmpCallbackData.partSize = m_partInfo.partSize;
        }
        m_uploadQueue.Push(tmpCallbackData);
    }
    tmpCallbackData.isPill = true;
    m_uploadQueue.Push(tmpCallbackData);
}

bool UploadFile::PrepareUploadPartData(const string &localFile, UploadPartData &partData)
{
    FILE *fp = fopen(localFile.c_str(), "rb");
    if (fp == nullptr) {
        HCP_Log(ERR, MODULE) << "Open file failed." << HCPENDLOG;
        return false;
    }
    partData.handle = FilePtrT(fp, FileDeleter);
    partData.callbackHandle = m_callbackHandle;
    return true;
}

void UploadFile::UploadThreadProc(const string &localFile)
{
    S3BucketContextProxy bucketContext(m_s3IOInfo);
    int retryTimes = S3_RETRY_TIMES;
    while (true) {
        UploadPartData uploadData = m_uploadQueue.Pop();
        if (uploadData.isPill) {
            HCP_Log(INFO, MODULE) << "Upload queue is empty, thread exit." << HCPENDLOG;
            m_uploadQueue.Push(uploadData);
            return;
        }
        if (!PrepareUploadPartData(localFile, uploadData)) {
            HCP_Log(ERR, MODULE) << "Prepare UploadPartData failed." << HCPENDLOG;
            return;
        }

        obs_upload_part_info uploadPartInfo = {uploadData.partNum, const_cast<char *>(m_partInfo.uploadId.c_str())};
        obs_upload_handler handler = {{&ConcurrentResponsePropertiesCallback, &ConcurrentUploadFileCompleteCallback},
            &ConcurrentUploadPartDataCallback};
        obs_put_properties putProperties;
        InitPutProperties(putProperties);

        do {
            uploadData.offsetInPart = 0;
            upload_part(&bucketContext, const_cast<char *>(m_partInfo.key.c_str()), &uploadPartInfo,
                uploadData.partSize, &putProperties, 0, &handler, &uploadData);
        } while (obs_status_is_retryable(uploadData.retStatus) && ShouldRetry(retryTimes));
        if (uploadData.retStatus != OBS_STATUS_OK) {
            HCP_Log(ERR, MODULE) << "Upload failed, part: " << uploadPartInfo.part_number
                                 << ", status: " << obs_get_status_name(uploadData.retStatus) << HCPENDLOG;
            // Abort multi part upload
            if (!MultipartUploadCancel(m_partInfo.key, m_partInfo.uploadId)) {
                HCP_Log(ERR, MODULE) << "MultipartUploadCancel failed." << HCPENDLOG;
            }
            InsertUploadPartStatus(uploadData);
            return; // Task Failed need exit.
        } else {
            if (m_callbackHandle.callBackFunc != nullptr) {
                ((WriteFileCallback)m_callbackHandle.callBackFunc)(
                    LayoutRetCode::SUCCESS, 0, uploadData.partSize, m_callbackHandle.callBackData);
            }
            HCP_Log(DEBUG, MODULE) << "Upload successfully, part: " << uploadPartInfo.part_number << HCPENDLOG;
            InsertUploadPartStatus(uploadData);
        }
    }
}

void UploadFile::StartUploadThreads(const string &localFile, int threadNum)
{
    m_threadg.create_thread(boost::bind(&UploadFile::DistributeThreadPoc, this));

    for (int i = 0; i < threadNum; i++) {
        m_threadg.create_thread(boost::bind(&UploadFile::UploadThreadProc, this, localFile));
    }
}

void UploadFile::JoinAllThread()
{
    m_threadg.join_all();
}

bool UploadFile::CheckUploadPartIsComplete()
{
    if (m_uploadPartStatus.size() != m_partInfo.partNum) {
        HCP_Log(ERR, MODULE) << "Upload part failed." << HCPENDLOG;
        return false;
    }
    for (const auto &part : m_uploadPartStatus) {
        if (part.retStatus != OBS_STATUS_OK) {
            HCP_Log(ERR, MODULE) << "Upload part: " << part.partNum << " failed." << HCPENDLOG;
            return false;
        }
    }
    return true;
}

bool UploadFile::ConcurrentUploadPart(const string &remoteFile, const string &localFile, int threadNum,
    CallBackHandle &handle)
{
    RegisterCallbackHandle(handle);
    string uploadId;
    if (!InitiateMultiPartUpload(remoteFile, uploadId)) {
        HCP_Log(ERR, MODULE) << "Init upload part failed." << HCPENDLOG;
        return false;
    }

    if (!SplitToPart(localFile, remoteFile, uploadId)) {
        HCP_Log(ERR, MODULE) << "Split to part failed." << HCPENDLOG;
        return false;
    }

    // Concurrent upload
    StartUploadThreads(localFile, threadNum);

    JoinAllThread();
    if (!CheckUploadPartIsComplete()) {
        HCP_Log(ERR, MODULE) << "Upload part failed." << HCPENDLOG;
        return false;
    }
    // Merge parts
    if (!CompleteMultiPartUpload(uploadId)) {
        HCP_Log(ERR, MODULE) << "Complete upload part failed." << HCPENDLOG;
        return false;
    }
    return true;
}

bool UploadFile::InitiateMultiPartUpload(const string &remoteFile, string &uploadId)
{
    S3BucketContextProxy bucketContext(m_s3IOInfo);
    obs_status status = OBS_STATUS_BUTT;

    // Initialize putProperties
    obs_put_properties putProperties;
    InitPutProperties(putProperties);

    obs_response_handler Handler = {
        &ResponsePropertiesCallback, &ResponseCompleteCallback
    };

    int retryTimes = S3_RETRY_TIMES;
    char uploadIdReturn[UPLOADID_RETURN_SIZE + 1] = {0};
    int uploadIdReturnSize = UPLOADID_RETURN_SIZE;
    do {
        initiate_multi_part_upload(
            &bucketContext, const_cast<char *>(remoteFile.c_str()), uploadIdReturnSize,
            uploadIdReturn, &putProperties, 0, &Handler, &status);
    } while (obs_status_is_retryable(status) && ShouldRetry(retryTimes));
    if (status != OBS_STATUS_OK) {
        HCP_Log(ERR, MODULE) << "Init upload part failed: " << obs_get_status_name(status) << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, MODULE) << "Init upload part success, uploadId: " << string(uploadIdReturn) << HCPENDLOG;
    uploadId = uploadIdReturn;
    return true;
}

bool UploadFile::CompleteMultiPartUpload(const string &uploadId)
{
    S3BucketContextProxy bucketContext(m_s3IOInfo);
    obs_status status = OBS_STATUS_BUTT;

    // Initialize putProperties
    obs_put_properties putProperties;
    InitPutProperties(putProperties);

    obs_complete_multi_part_upload_handler completeMultiHandler = {
        {&ResponsePropertiesCallback, &ResponseCompleteCallback}, &CompleteMultipartUploadCallback};

    unique_ptr<obs_complete_upload_Info[]> uploadInfoPtr =
        make_unique<obs_complete_upload_Info[]>(m_partInfo.partNum);
    if (uploadInfoPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "Malloc upload_Info failed!" << HCPENDLOG;
        return false;
    }

    int partIndex = 0;
    for (const auto &part : m_uploadPartStatus) {
        uploadInfoPtr[partIndex].part_number = part.partNum;
        uploadInfoPtr[partIndex].etag = const_cast<char *>(part.etag.c_str());
        partIndex++;
        HCP_Log(DEBUG, MODULE) << "Part num: " << part.partNum << HCPENDLOG;
        HCP_Log(DEBUG, MODULE) << "Part etag: " << string(part.etag) << HCPENDLOG;
    }
    int retryTimes = S3_RETRY_TIMES;
    do {
        complete_multi_part_upload(&bucketContext,
            const_cast<char *>(m_partInfo.key.c_str()),
            uploadId.c_str(),
            m_partInfo.partNum,
            uploadInfoPtr.get(),
            &putProperties,
            &completeMultiHandler,
            &status);
    } while (obs_status_is_retryable(status) && ShouldRetry(retryTimes));
    if (status != OBS_STATUS_OK) {
        HCP_Log(ERR, MODULE) << "Complete upload part failed: " << obs_get_status_name(status) << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, MODULE) << "Complete upload successfully." << HCPENDLOG;
    return true;
}

bool UploadFile::MultipartUploadCancel(const string &remoteFile, const string &uploadId)
{
    HCP_Log(DEBUG, MODULE) << "Enter MultipartUploadCancel." << DBG(remoteFile) << HCPENDLOG;
    S3BucketContextProxy bucketContext(m_s3IOInfo);
    obs_status status = OBS_STATUS_BUTT;

    obs_response_handler responseHandler = {&ResponsePropertiesCallback, &ResponseCompleteCallback};

    int retryTimes = S3_RETRY_TIMES;
    do {
        abort_multi_part_upload(&bucketContext,
            const_cast<char *>(remoteFile.c_str()),
            const_cast<char *>(uploadId.c_str()),
            &responseHandler,
            &status);
    } while (obs_status_is_retryable(status) && ShouldRetry(retryTimes));
    if (status != OBS_STATUS_OK && status != OBS_STATUS_NoSuchUpload) {
        HCP_Log(ERR, MODULE) << "AbortMultipartUpload failed!" << HCPENDLOG;
        return false;
    }

    HCP_Log(DEBUG, MODULE) << "AbortMultipartUpload success!!" << DBG(remoteFile) << HCPENDLOG;
    return true;
}

void UploadFile::SetUpLoadRateLimit(uint64_t qos)
{
    HCP_Log(DEBUG, MODULE) << "UploadFile::SetUpLoadRateLimit: " << qos << HCPENDLOG;
    m_s3IOInfo.uploadRateLimit = qos;
}

void UploadFile::SetDownLoadRateLimit(uint64_t qos)
{
    m_s3IOInfo.downloadRateLimit = qos;
}