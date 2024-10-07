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
#ifndef OBSCTX_CLOUD_SERVICE_REQUEST_H
#define OBSCTX_CLOUD_SERVICE_REQUEST_H

#include <vector>
#include <string>
#include <unordered_map>
#include "interface/CloudServiceResponse.h"

namespace Module {
    const int MAX_LIST_OBJECT_NUM = 1000;
    const int DEFAULT_RETRY_NUM = 5;
    const int DEFAULT_RETRY_INTERVAL = 1; // second
    const uint64_t MULTI_PART_DOWNLOAD_OBJECT_PART_SIZE = 5L * 1024 * 1024 * 1024; // 5G
    const uint64_t MULTI_PART_DOWNLOAD_OBJECT_TASK_NUM = 8;

    class RetryConfig {
        public:
        bool isRetryable = true;                   // 是否需要失败重试（只针对特定可重试恢复的错误码）
        int retryNum = DEFAULT_RETRY_NUM;           // 重试次数
        int retryInterval = DEFAULT_RETRY_INTERVAL; // 重试间隔时间（单位：秒）
    };

    class BasicRequest {
        public:
        std::string bucketName;  // 桶名
        RetryConfig retryConfig; // 重试设置
    };

    class CheckConnectRequest : public BasicRequest {
    };

    class ListBucketsRequest : public BasicRequest {
    };

    class GetBucketACLRequest : public BasicRequest {
    public:
        bool isNewGet {false};    // 判断是否为backup新的获取
    };

    class ListObjectsRequest : public BasicRequest {
        public:
        std::string prefix;     // 限定返回的对象名必须带有prefix前缀
        std::string marker;     // 列举对象的起始位置，返回的对象列表将是对象名按照字典序排序后该参数以后的所有对象
        std::string delimiter;  // 用于对对象名进行分组的字符。对于对象名中包含delimiter的对象，其对象名
                                // （如果请求中指定了prefix，则此处的对象名需要去掉prefix）中从首字符至第一个delimiter之间
                                // 的字符串将作为一个分组并作为commonPrefix返回
        int maxkeys = MAX_LIST_OBJECT_NUM; // 列举对象的最大数目，取值范围为1~1000，当超出范围时，按照默认的1000进行处理
    };

    class GetObjectMetaDataRequest : public BasicRequest {
        public:
        std::string key;         // 对象名
        std::string versionId;   // 版本号
    };

    class GetObjectACLRequest : public BasicRequest {
        public:
        std::string key;         // 对象名
        std::string versionId;   // 版本号
        bool isNewGet {false};   // 判断是否为backup新的获取
    };

    class GetObjectRequest : public BasicRequest {
        public:
        std::string key;         // 对象名
        std::string versionId;   // 版本号
        uint64_t startByte = 0;  // 分片起始位置（字节）
        uint64_t byteCount = 0;  // 分片大小（0表示读到对象内容末尾，如果byteCount超过对象剩余实际大小，则按对象实际大小取值）

        uint8_t* buffer = nullptr; // 适配FS_Backup，用于存放读取到的数据。（需要外部进行内存管理）
        int bufferSize = 0;
    };

    class MultiPartDownloadObjectRequest : public BasicRequest {
        public:
        std::string key;         // 对象名
        std::string versionId;   // 版本号
        std::string downLoadTargetPath;                           // 下载目标路径
        uint64_t partSize = MULTI_PART_DOWNLOAD_OBJECT_PART_SIZE; // 每个分块大小（单位：字节）
        uint64_t taskNum = MULTI_PART_DOWNLOAD_OBJECT_TASK_NUM;   // 下载最大并发线程数
        bool enableCheckPoint = false;                            // 是否支持断点续传
        std::string checkPointFilePath;                           // 断点续传记录文件路径
    };

    class GetBucketLogConfigRequest : public BasicRequest {
    };

    class HeadBucketRequest : public BasicRequest {

    };

    class CreateBucketRequest : public BasicRequest {
        public:
        int cannedAcl = 0;  // 桶的ACL
    };

    typedef void(MultiPartUploadObjectCallbackFun)(uint64_t partSize, int partCount, void* callBackData);

    class MultiPartUploadObjectRequest : public BasicRequest {
        public:
        std::string key;         // 对象名
        std::string upLoadTargetPath;                             // 上传目标路径
        uint64_t partSize = MULTI_PART_DOWNLOAD_OBJECT_PART_SIZE; // 每个分块大小（最大5GB，单位：字节）
        uint64_t taskNum = MULTI_PART_DOWNLOAD_OBJECT_TASK_NUM;   // 上传最大并发线程数
        bool enableCheckPoint = false;                            // 是否支持断点续传(开启后才会生成checkpoint文件)
        std::string checkPointFilePath;                           // 断点续传记录文件路径
        MultiPartUploadObjectCallbackFun *callBack = nullptr;
        void* callBackData;
    };

    class SetObjectACLRequest : public BasicRequest {
        public:
        std::string key;                    // 对象名
        std::string versionId;              // 版本号，非多版本对象version设置为0
        std::string ownerId;                // 用户的domianID
        std::string ownerDisplayName;       // 用户显示名称
        std::vector<ACLGrant> aclGrants;    // acl规则
    };

    class SetBucketACLRequest : public BasicRequest {
        public:
        std::string ownerId;                // 用户的domianID
        std::string ownerDisplayName;       // 用户显示名称
        std::vector<ACLGrant> aclGrants;    // acl规则
    };

    class SetObjectMetaDataRequest : public BasicRequest {
        public:
        std::string key;         // 对象名
        std::string versionId;   // 版本号
        std::unordered_map<std::string, std::string> sysDefMetaData; // 系统metadata
        std::unordered_map<std::string, std::string> userDefMetaData; // 用户默认metadate
    };

    class GetUploadIdRequest : public SetObjectMetaDataRequest {
        public:
        std::string key;            // 对象名
    };

    class PutObjectPartRequest : public SetObjectMetaDataRequest {
        public:
        std::string key;            // 对象名
        uint32_t partNumber = 0;    // 分片编号1,2,3... (只有分段上传接口使用)
        std::string uploadId;       // upload id唯一,用于标识本次分段上传任务 (只有分段上传接口使用)
        uint64_t startByte = 0;     // 开始位置
        uint64_t partSize = 0;      // 本次上传内存大小
        char* bufPtr = nullptr;     // 内存地址
    };

    class UploadInfo {
        public:
        uint32_t partNumber = 0;
        std::string etag;
    };

    class CompletePutObjectPartRequest : public BasicRequest {
        public:
        std::string key;            // 对象名
        std::string uploadId;       // upload id唯一,用于标识本次分段上传任务
        std::vector<UploadInfo> uploadInfo; // 上传分片信息
        bool isFailed {false};    // 上传是否失败
    };

    class DeleteBucketRequest : public BasicRequest {
    };

    class DeleteObjectRequest : public BasicRequest {
    public:
        std::string key;            // 对象名
        std::string versionId;   // 版本号，非多版本填0
    };
}

#endif  // OBSCTX_CLOUD_SERVICE_REQUEST_H
