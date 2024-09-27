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
#ifndef S3_BASE_H
#define S3_BASE_H

#include <boost/logic/tribool.hpp>
#include "S3IOParams.h"
#include "IODeviceInterface.h"
#include "securec.h"

namespace Module {
typedef struct upload_callback_data {
    upload_callback_data()
        : fp(nullptr), originalContentLength(0), contentLength(0), currentReadLength(0), uploadpartNum(0)
    {}
    FILE *fp;
    uint64_t originalContentLength;
    uint64_t contentLength;
    uint64_t currentReadLength;
    int uploadpartNum;
} upload_callback_data;

typedef struct singlePart {
    unsigned int partNumber;
    std::string eTag;
    int64_t lastModified;
    uint64_t partSzie;
} singlePart;

typedef struct list_parts_callback_data {
    int isTruncated;
    char initiatorId[1024];
    char initiatorDisplayName[1024];
    char ownerId[1024];
    char ownerDisplayName[1024];
    unsigned int nextPartNumberMarker;
    char storageClass[64];
    int keyCount;
    int allDetails;

    int partsCount;
    std::map<int, singlePart> partMap;
} list_parts_callback_data;

typedef struct singleUploadTask {
    std::string objName;
    std::string uploadTaskID;
    int64_t initiated;

    singleUploadTask() : initiated(0)
    {}
    singleUploadTask(std::string _objName, std::string _uploadTaskID, int64_t _initiated)
        : objName(_objName), uploadTaskID(_uploadTaskID), initiated(_initiated)
    {}
} singleUploadTask;

typedef struct list_multipart_uploads_callback_data {
    int isTruncated;
    char nextMarker[1024];
    char nextUploadIdMarker[1024];
    int uploadsCount;
    int keyCount;
    int allDetails;

    int uploadTasksCount;
    std::map<int, singleUploadTask> uploadTaskMap;

    list_multipart_uploads_callback_data()
        : isTruncated(0), uploadsCount(0), keyCount(0), allDetails(0), uploadTasksCount(0)
    {
        memset_s(nextMarker, sizeof(char) * 1024, 0, sizeof(char) * 1024);
        memset_s(nextUploadIdMarker, sizeof(char) * 1024, 0, sizeof(char) * 1024);
    }
} list_multipart_uploads_callback_data;

typedef struct list_bucket_callback_data {
    list_bucket_callback_data()
    {
        isTruncated = 0;
        keyCount = 0;
        memset_s(nextMarker, sizeof(char) * 1024, 0, sizeof(char) * 1024);
    }

    int isTruncated;
    char nextMarker[1024];
    int keyCount;

} list_bucket_callback_data;

typedef struct S3ObjectContent : public obs_list_objects_content {
    S3ObjectContent(const obs_list_objects_content &content)
    {
        copy(content);
    }

    S3ObjectContent &operator=(const obs_list_objects_content &content)
    {
        copy(content);
        return *this;
    }

    std::string m_ObjectName;

private:
    void copy(const obs_list_objects_content &content)
    {
        this->key = content.key;
        this->last_modified = content.last_modified;
        this->etag = content.etag;
        this->owner_id = content.owner_id;
        this->owner_display_name = content.owner_display_name;
        this->storage_class = content.storage_class;
        this->size = content.size;

        this->m_ObjectName = content.key;
    }
} S3ObjectContent;

typedef struct obj_metadata
{
    std::string name;
    std::string value;
} obj_metadata;

class ICache;

class S3Base {
public:
    S3Base();
    virtual ~S3Base();
    virtual void PrintStatusErr(std::string funName);
    virtual bool ShouldRetry(int &retryTimes);
    virtual bool S3StatusIsRetryable();
    virtual bool S3HeadStatusIsRetryable();
    virtual int GetS3RetryTimeAndSetConnectTImeOut();
    virtual std::string Base64Encode();
    virtual std::string DekToMd5();

    // these functions are for callback
    virtual void CallbackComplete(obs_status status);
    virtual void CallbackResponse(const obs_response_properties *properties);

    virtual ssize_t CallbackWrite(const char *buf, size_t bufSize)
    {
        return 0;
    };
    virtual size_t CallbackRead(char *buf, size_t bufSize)
    {
        return 0;
    };
    virtual size_t PartCallbackRead(char *buf, size_t bufSize)
    {
        return 0;
    };
    virtual int CallbackUploadData(int bufferSize, char *buffer);
    virtual obs_status CallbackListParts(int isTruncated, unsigned int nextPartNumberMarker, int partsCount, const obs_list_parts *parts);
    virtual obs_status ListBucketCallbackInternal(int isTruncated, const char *nextMarker, int contentsCount,
                                                  const obs_list_objects_content *contents, int commonPrefixesCount, const char **commonPrefixes);
    virtual boost::tribool FileExists(S3BucketContextProxy &bucketContext, const std::string &fileName);

    virtual bool RemoveFile(S3BucketContextProxy& bucketContext, const std::string& fileName);
    virtual bool MultipartUploadCancel(S3BucketContextProxy& bucketCtx,const std::string& remote_objName,const std::string& uploadTaskID);
    static int Md5Base64Encode(const unsigned char *in, int inLen, char *out);

#ifndef EBKSDK_LIBS3_COMPILE
    virtual boost::tribool FileExists(S3BucketContextProxy &bucketContext, const std::string &fileName,
      uint64_t *contentLength, int64_t *lastModified, std::string &etag);
#endif

protected:
    virtual void AppendObjectList(std::vector<obs_list_objects_content> &vecContents);
    virtual void AppendObjName(std::string objName);
    virtual void AppendDirName(std::string dirName);
    virtual bool CheckStatus()
    {
        return (m_completedStatus == OBS_STATUS_OK);
    }
    virtual void RWBegin() = 0;
    virtual void RWRetry(bool resetFlag) = 0;
    virtual bool BeginUpload()
    {
        return true;
    }
    virtual bool RetryUpload(uint64_t segmentNum, uint64_t segmentSize) = 0;
    virtual size_t GetUploadFileData(char *buffer, int dataLen) = 0;
    virtual bool ListBucket(const S3IOParams &params, const char *prefix = nullptr, const char *marker = nullptr,
                            const char *delimiter = nullptr, int maxkeys = 0);
    virtual bool ListBucket(S3BucketContextProxy &bucketContext, const char *prefix = nullptr, const char *marker = nullptr,
                            const char *delimiter = nullptr, int maxkeys = 0);
    virtual bool BigFileUploadInit(S3BucketContextProxy &bucketCtx, const std::string &objName,
                                   std::string &uploadTaskID);

    virtual void ReadDataFromS3(S3BucketContextProxy &bucketContext, size_t offset, size_t dataLen);
    virtual bool FillS3Variables(obs_object_info &object_info, obs_get_conditions &get_conditions, size_t offset, size_t dataLen);
    virtual size_t WriteDataToS3(S3BucketContextProxy &bucketContext, const std::string objKey, size_t dataLen);
    virtual size_t WriteDataToS3(S3BucketContextProxy &bucketContext, obs_put_object_handler &handler,
                                 const std::string objKey, size_t dataLen);
    void SetObjectKey(const std::string &objKey);
    void SetObjectName(const std::string &objName);
    virtual bool IsS3Accelerator();

    void CalculationFileMd5();
    void InitPutProperties(obs_put_properties *put_properties, bool isMd5Upload = false);  // set the acl of put object
    void GetUploadPutProperties(obs_put_properties &put_properties, int &isMd5Upload, const std::string &objKey);
    void InitS3AccessParameter(server_side_encryption_params &serverSideEncryptionParams,
        obs_put_properties &put_properties, const std::string &objKey);
    static obs_status ResponseCallback(const obs_response_properties *properties, void *callbackData);
    static void CompleteCallback(obs_status status, const obs_error_details *error, void *callbackData);
    static int PutObjectDataCallback(int bufferSize, char *buffer, void *callbackData);
    static obs_status GetObjectDataCallback(int bufferSize, const char *buffer, void *callbackData);

private:
    void InitEncryptionParams(server_side_encryption_params &ServerSideEncryptionParams);
    void PrintGetObjectStatusInfo();
public: 
    int m_objType;  // 0 cache data; 1 file data
    std::shared_ptr<int> m_localFile;
    uint64_t m_downloadedSize {0};
    ICache *m_cache;     // data buffer
    char m_uploadMd5[64];
    CallBackHandle m_callbackHandle;
    ReadFileCallback m_readFileFun;
protected:
    obs_status m_completedStatus;
    uint64_t m_contentLength;
    char m_requestID[64];
    // s3 maximum object length supported, the last is '/0'.
    char m_objectName[1024 + 1];
    char m_objKey[1024 + 1];
    std::string m_dek;
    // int m_bbrSwitch;
    S3IOParams m_S3IOInfo;
    upload_callback_data m_UploadPartInfo;
    list_parts_callback_data m_ListPartsInfo;
    list_multipart_uploads_callback_data m_ListPartUploadsInfo;
    list_bucket_callback_data m_ListBucketInfo;
    std::vector<S3ObjectContent> m_objectInfoList;  // object struct list on the bucket
    std::vector<std::string> m_matchObjList;        // object list on the bucket
    std::vector<std::string> m_matchDirList;        // dir list on the bucket
    uint64_t m_ObjectsTotalSize;
    std::string m_accelerator_proxy;
    bool m_useVpp;
    int64_t m_lastModified;
    std::string m_etag;
    std::vector<obj_metadata> m_obj_metadata;
    std::vector<singlePart> m_PartsInfoVector;
};
} // namespace Module
#endif
