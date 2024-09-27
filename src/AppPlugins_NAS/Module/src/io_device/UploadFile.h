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
#ifndef UPLOAD_FILE_H
#define UPLOAD_FILE_H

#include <boost/thread/thread.hpp>
#include <set>
#include <condition_variable>
#include "IODeviceManager.h"
#include "S3IOParams.h"
#include "LayoutCommon.h"
#include "IODeviceInterface.h"

namespace Module {
class UploadFileInterface {
public:
    virtual ~UploadFileInterface() = default;
    virtual bool ConcurrentUploadPart(
        const std::string &remoteFile, const std::string &localFile, int threadNum, CallBackHandle &handle) = 0;
};

struct UploadPartInfo {
    uint32_t partNum{0};
    uint64_t partSize{0};
    uint64_t contentLength{0};
    std::string key;
    std::string uploadId;
};

using FilePtrT = std::shared_ptr<FILE>;

struct UploadPartData {
    bool isPill{false};
    FilePtrT handle{nullptr};
    obs_status retStatus{OBS_STATUS_BUTT};
    uint32_t partNum{0};
    std::string etag;
    uint64_t partSize{0};
    uint64_t partStartPos{0};
    uint64_t offsetInPart{0};
    CallBackHandle callbackHandle;
};

struct UploadPartCompleteStatus {
    obs_status retStatus{OBS_STATUS_BUTT};
    uint32_t partNum{0};
    std::string etag;
    bool operator<(const UploadPartCompleteStatus &u) const
    {
        return (partNum < u.partNum);
    }
};

class UploadQueue {
public:
    UploadQueue()
    {}
    ~UploadQueue()
    {}
    UploadPartData Pop();
    void Push(const UploadPartData &data);
    uint64_t Size()
    {
        std::lock_guard<std::mutex> lck(m_mutex);
        return m_queue.size();
    }

private:
    std::queue<UploadPartData> m_queue{};
    std::mutex m_mutex;
    std::condition_variable m_cond;
    const static unsigned int QUEUE_SIZE = 100;
};

class UploadFile : public UploadFileInterface {
public:
    UploadFile(const IODeviceInfo &deviceInfo);
    ~UploadFile();

    bool ConcurrentUploadPart(const std::string &remoteFile, const std::string &localFile, int threadNum,
        CallBackHandle &handle) override;
    void SetUpLoadRateLimit(uint64_t qos);
    void SetDownLoadRateLimit(uint64_t qos);

protected:
    void CreateDeviceInfo(const IODeviceInfo &deviceInfo);
    bool SplitToPart(const std::string &localFile, const std::string &remoteFile, const std::string &uploadId);
    void InitPutProperties(obs_put_properties &putProperties);
    bool PrepareUploadPartData(const std::string &localFile, UploadPartData &partData);
    void StartUploadThreads(const std::string &localFile, int threadNum);
    void DistributeThreadPoc();
    void UploadThreadProc(const std::string &localFile);
    void JoinAllThread();
    bool CheckUploadPartIsComplete();
    bool InitiateMultiPartUpload(const std::string &remoteFile, std::string &uploadId);
    bool CompleteMultiPartUpload(const std::string &uploadId);
    bool MultipartUploadCancel(const std::string &remoteFile, const std::string &uploadId);

    static obs_status ResponsePropertiesCallback(const obs_response_properties *properties, void *callbackData);
    static void ResponseCompleteCallback(obs_status status, const obs_error_details *error, void *callbackData);
    static obs_status ConcurrentResponsePropertiesCallback(
        const obs_response_properties *properties, void *callbackData);
    static obs_status CompleteMultipartUploadCallback(const char *location,
                                         const char *bucket,
                                         const char *key,
                                         const char* eTag,
                                         void *callbackData);
    static void ConcurrentUploadFileCompleteCallback(
        obs_status status, const obs_error_details *error, void *callbackData);
    static int ConcurrentUploadPartDataCallback(int bufferSize, char *buffer, void *callbackData);
    void RegisterCallbackHandle(CallBackHandle &handle)
    {
        m_callbackHandle = handle;
    }
    bool ShouldRetry(int &retryTimes);
    static void FileDeleter(FILE *fp)
    {
        if (fp != nullptr) {
            fclose(fp);
        }
    }
    void InsertUploadPartStatus(const UploadPartData &uploadData)
    {
        std::lock_guard<std::mutex> lck(m_partStatusMutex);
        UploadPartCompleteStatus uploadPart;
        uploadPart.etag = uploadData.etag;
        uploadPart.partNum = uploadData.partNum;
        uploadPart.retStatus = uploadData.retStatus;
        m_uploadPartStatus.insert(uploadPart);
    }

private:
    S3IOParams m_s3IOInfo;
    UploadPartInfo m_partInfo;
    UploadQueue m_uploadQueue;
    std::mutex m_partStatusMutex;
    std::set<UploadPartCompleteStatus> m_uploadPartStatus;
    boost::thread_group m_threadg {};
    CallBackHandle m_callbackHandle;
};
}
#endif // UPLOAD_FILE_H