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
#ifndef OBSCTX_CLOUDE_SERVICE_INTERFACE
#define OBSCTX_CLOUDE_SERVICE_INTERFACE
#include <string>
#include <vector>
#include <memory>
#include "interface/CloudServiceResponse.h"
#include "interface/CloudServiceRequest.h"
#include "common/CloudServiceErrorCode.h"

namespace Module {

    struct OBSResult {
        ResultType result {ResultType::SUCCESS};
        std::string errorCode;
        std::string errorDesc;
        StorageType storageType {StorageType::OTHER};

        int64_t GetCommonErrorCode() const
        {
            return CloudServiceErrorCode::Transform2OMRP(storageType, errorCode);
        }

        int64_t GetLinuxErrorCode() const
        {
            return CloudServiceErrorCode::Transform2Linux(storageType, errorCode);
        }

        bool IsSucc() const { return result == ResultType::SUCCESS; }

        bool operator== (const OBSResult& other) {
            return result == other.result && errorCode == other.errorCode && errorDesc == other.errorDesc;
        }
    };

    struct StorageVerifyInfo {
        std::string endPoint;
        std::string accessKey;
        std::string secretKey;
        bool useHttps {false};
        std::string certHttps;
        std::string caPath;
        std::string caFile;
        bool useProxy {false};
        std::string proxyHostName;
        std::string proxyUserName;
        std::string proxyUserPwd;
    };

    class CloudServiceInterface {
    public:
        CloudServiceInterface(const StorageVerifyInfo& verifyInfo) : m_verifyInfo(verifyInfo) {};
        virtual ~CloudServiceInterface() = default;

        /**
        * 检测连通性
        *
        * @param CheckConnectRequest 重试参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 连通成功,其他连通失败.
        */
        virtual OBSResult CheckConnect(const std::unique_ptr<CheckConnectRequest>& request) = 0;

        /**
        * 列举对象存储中所有的桶列表，最大支持一次性列举200个桶
        *
        * @param ListBucketsRequest 列举桶请求参数
        * @param ListBucketsResponse 列举桶结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult ListBuckets(
            const std::unique_ptr<ListBucketsRequest> &request, std::unique_ptr<ListBucketsResponse> &resp) = 0;

        /**
        * 获取桶的ACL
        *
        * @param GetBucketACLRequest 获取桶的ACL请求参数
        * @param GetBucketACLResponse 获取桶的ACL结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult GetBucketACL(
            const std::unique_ptr<GetBucketACLRequest>& request, std::unique_ptr<GetBucketACLResponse>& resp) = 0;

        /**
        * 列举桶内的所有对象，每次最多列举1000个
        *
        * @param ListObjectsRequest 列举对象请求参数
        * @param ListObjectsResponse 列举对象结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult ListObjects(
            const std::unique_ptr<ListObjectsRequest>& request, std::unique_ptr<ListObjectsResponse>& resp) = 0;

        /**
        * 获取对象元数据
        *
        * @param GetObjectMetaDataRequest 获取对象元数据请求参数
        * @param GetObjectMetaDataResponse 获取对象元数据结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult GetObjectMetaData(
            const std::unique_ptr<GetObjectMetaDataRequest>& request,
            std::unique_ptr<GetObjectMetaDataResponse>& resp) = 0;

        /**
        * 获取对象ACL
        *
        * @param GetObjectACLRequest 获取对象ACL请求参数
        * @param GetObjectACLResponse 获取对象ACL结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult GetObjectACL(
            const std::unique_ptr<GetObjectACLRequest>& request, std::unique_ptr<GetObjectACLResponse>& resp) = 0;

        /**
        * 指定分片下载数据
        *
        * @param GetObjectRequest 指定分片下载数据请求参数
        * @param GetObjectResponse 指定分片下载数据结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult GetObject(
            const std::unique_ptr<GetObjectRequest>& request, std::unique_ptr<GetObjectResponse>& resp) = 0;

        /**
        * 分段下载数据
        *
        * @param MultiPartDownloadObjectRequest 分段下载数据请求参数
        * @param MultiPartDownloadObjectResponse 分段下载数据结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult MultiPartDownloadObject(
            const std::unique_ptr<MultiPartDownloadObjectRequest>& request,
            std::unique_ptr<MultiPartDownloadObjectResponse>& resp) = 0;

        /**
        * 获取桶日志配置
        *
        * @param GetBucketLogConfigRequest 获取桶日志配置参数
        * @param GetBucketLogConfigResponse 获取桶日志配置结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他获取失败.
        */
        virtual OBSResult GetBucketLogConfig(
            const std::unique_ptr<GetBucketLogConfigRequest>& request,
            std::unique_ptr<GetBucketLogConfigResponse>& resp) = 0;

        /**
        * 判断桶是否存在
        *
        * @param HeadBucketRequest 判断桶是否存在请求参数
        * @param HeadBucketResponse 判断桶是否存在结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult IsBucketExist(
            const std::unique_ptr<HeadBucketRequest> &request, std::unique_ptr<HeadBucketResponse> &resp) = 0;

        /**
        * 创建桶
        *
        * @param CreateBucketRequest 判断桶是否存在请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult CreateBucket(const std::unique_ptr<CreateBucketRequest> &request) = 0;

        /**
        * 分段上传数据
        *
        * @param MultiPartUploadObjectRequest 分段上传数据请求参数
        * @param MultiPartUploadObjectResponse 分段上传数据结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest> &request,
            std::unique_ptr<MultiPartUploadObjectResponse> &resp) = 0;

        /**
        * 设置对象ACL
        *
        * @param SetObjectACLRequest 设置对象ACL请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request) = 0;

        /**
        * 设置桶ACL
        *
        * @param SetBucketACLRequest 设置桶ACL请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request) = 0;

        /**
        * 设置对象meta
        *
        * @param SetObjectMetaDataRequest 设置对象meta请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request) = 0;

        /**
        * 分片流式上传文件获取upload id
        *
        * @param GetObjectRequest 分片流式上传文件请求参数
        * @param GetObjectResponse 分片流式上传文件结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult GetUploadId(
            const std::unique_ptr<GetUploadIdRequest> &request, std::unique_ptr<GetUploadIdResponse> &resp) = 0;

        /**
        * 分片流式上传文件
        *
        * @param GetObjectRequest 分片流式上传文件请求参数
        * @param GetObjectResponse 分片流式上传文件结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult PutObjectPart(
            const std::unique_ptr<PutObjectPartRequest>& request, std::unique_ptr<PutObjectPartResponse>& resp) = 0;

        /**
        * 分片流式上传文件
        *
        * @param CompletePutObjectPartRequest 分片流式上传文件请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult CompletePutObjectPart(
            const std::unique_ptr<CompletePutObjectPartRequest>& request) = 0;

        /**
        * 删除桶
        * @param DeleteBucketRequest 删除桶请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 删除成功,其他删除失败.
        */
        virtual OBSResult DeleteBucket(const std::unique_ptr<DeleteBucketRequest>& request) = 0;

        /**
        * 删除对象
        * @param DeleteObjectRequest 删除对象请求参数
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 删除成功,其他删除失败.
        */
        virtual OBSResult DeleteObject(const std::unique_ptr<DeleteObjectRequest>& request) = 0;

        /**
        * 流式上传缓冲区数据
        *
        * @param PutObjectPartRequest  流式上传缓冲区数据请求参数（复用分段上传参数）
        * @param PutObjectPartResponse 流式上传缓冲区数据结果
        * @return OBSResult 列举返回值 = OBSResult::SUCCESS 列举成功,其他列举失败.
        */
        virtual OBSResult PutObject(
            const std::unique_ptr<PutObjectPartRequest>& request, std::unique_ptr<PutObjectPartResponse>& resp) = 0;

    protected:
        StorageVerifyInfo m_verifyInfo;
    };
}

#endif  // OBSCTX_CLOUDE_SERVICE_INTERFACE
