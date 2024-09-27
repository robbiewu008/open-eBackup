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
#ifndef OBSCTX_ALI_CLOUD_SERVICE_H
#define OBSCTX_ALI_CLOUD_SERVICE_H

#include "alibabacloud/oss/OssClient.h"
#include "interface/CloudServiceInterface.h"

namespace Module {
    class ALiCloudService : public CloudServiceInterface {
    public:
        explicit ALiCloudService(const StorageVerifyInfo &verifyInfo) : CloudServiceInterface(verifyInfo){};
        ~ALiCloudService() {};
        OBSResult CheckConnect(const std::unique_ptr<CheckConnectRequest>& request) override;
        OBSResult ListBuckets(
            const std::unique_ptr<ListBucketsRequest>& request, std::unique_ptr<ListBucketsResponse>& resp) override;
        OBSResult GetBucketACL(
            const std::unique_ptr<GetBucketACLRequest>& request, std::unique_ptr<GetBucketACLResponse>& resp) override;
        OBSResult ListObjects(
            const std::unique_ptr<ListObjectsRequest>& request, std::unique_ptr<ListObjectsResponse>& resp) override;
        OBSResult GetObjectMetaData(
            const std::unique_ptr<GetObjectMetaDataRequest>& request,
            std::unique_ptr<GetObjectMetaDataResponse>& resp) override;
        OBSResult GetObjectACL(
            const std::unique_ptr<GetObjectACLRequest>& request, std::unique_ptr<GetObjectACLResponse>& resp) override;
        OBSResult GetObject(
            const std::unique_ptr<GetObjectRequest>& request, std::unique_ptr<GetObjectResponse>& resp) override;
        OBSResult MultiPartDownloadObject(
            const std::unique_ptr<MultiPartDownloadObjectRequest>& request,
            std::unique_ptr<MultiPartDownloadObjectResponse>& resp) override;
        OBSResult IsBucketExist(
            const std::unique_ptr<HeadBucketRequest> &request, std::unique_ptr<HeadBucketResponse> &resp) override;
        OBSResult CreateBucket(const std::unique_ptr<CreateBucketRequest> &request) override;
        OBSResult MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest> &request,
            std::unique_ptr<MultiPartUploadObjectResponse> &resp) override;
        OBSResult GetBucketLogConfig(
            const std::unique_ptr<GetBucketLogConfigRequest>& request,
            std::unique_ptr<GetBucketLogConfigResponse>& resp) override;
        OBSResult SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request) override;
        OBSResult SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request) override;
        OBSResult SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request) override;
        OBSResult GetUploadId(
            const std::unique_ptr<GetUploadIdRequest>& request, std::unique_ptr<GetUploadIdResponse>& resp) override;
        OBSResult PutObjectPart(const std::unique_ptr<PutObjectPartRequest> &request,
            std::unique_ptr<PutObjectPartResponse> &resp) override;
        OBSResult CompletePutObjectPart(
            const std::unique_ptr<CompletePutObjectPartRequest>& request) override;
        OBSResult DeleteBucket(const std::unique_ptr<DeleteBucketRequest>& request) override;
        OBSResult DeleteObject(const std::unique_ptr<DeleteObjectRequest>& request) override;
        OBSResult PutObject(
            const std::unique_ptr<PutObjectPartRequest>& request,
            std::unique_ptr<PutObjectPartResponse>& resp) override;
    
    private:
        std::unique_ptr<AlibabaCloud::OSS::OssClient> InitBasicOptions(AlibabaCloud::OSS::ClientConfiguration &option);
        void PrintInitOptions(AlibabaCloud::OSS::ClientConfiguration &option);
        template <class OutcomeStruct>
        bool CheckRetryAndWait(
            const RetryConfig &retryConfig, int retryCount, const OutcomeStruct &outcome, OBSResult &result);
        bool SplitUrl(const std::string &url, std::string &protocol, std::string &ip, unsigned int &port);
        OBSResult AbortPutObjectPart(const std::unique_ptr<CompletePutObjectPartRequest>& request);
        AlibabaCloud::OSS::ObjectMetaData CreateObjectMetaData(
            const std::unordered_map<std::string, std::string> &sysData,
            const std::unordered_map<std::string, std::string> &userData);
        void SaveObjectContent(const std::string& delimiter,
            const AlibabaCloud::OSS::ObjectSummaryList& objectList, std::vector<ListObjectsContent>& contents);
        void SaveObjectMetaData(
            const AlibabaCloud::OSS::ObjectMetaData &metaData, std::unique_ptr<GetObjectResponse>& resp);
    };
}

#endif  // OBSCTX_ALI_CLOUD_SERVICE_H
