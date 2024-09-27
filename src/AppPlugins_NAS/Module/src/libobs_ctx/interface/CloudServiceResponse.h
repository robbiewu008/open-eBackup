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
#ifndef OBSCTX_CLOUD_SERVICE_RESPONSE_H
#define OBSCTX_CLOUD_SERVICE_RESPONSE_H

#include <vector>
#include <string>
#include <unordered_map>

namespace Module {
    class ACLGrant {
        public:
        int grantType;
        std::string userType;
        std::string userId;
        int permission;
        int bucketDelivered;
    };

    class GetBucketACLResponse {
        public:
        std::string ownerId;
        std::string ownerDisplayName;
        std::vector<ACLGrant> aclGrants;
    };

    class ListBucketsResponse {
        public:
        std::vector<std::string> bucketList;
    };

    class ListObjectsContent {
        public:
        std::string key;
        uint64_t lastModified;
        std::string etag;
        uint64_t size;
        std::string ownerId;
        std::string ownerDisplayName;
        std::string storageClass;
        std::string type;
    };

    class ListObjectsResponse {
        public:
        bool isTruncated = false;
        std::string nextMarker;
        std::vector<ListObjectsContent> contents;
        std::vector<std::string> commonPrefixes;
    };

    class GetObjectMetaDataResponse {
        public:
        uint64_t lastModified;
        std::string etag;
        uint64_t size;
        std::unordered_map<std::string, std::string> sysDefMetaData;
        std::unordered_map<std::string, std::string> userDefMetaData;
    };

    class GetObjectACLResponse {
        public:
        std::string ownerId;
        std::string ownerDisplayName;
        std::vector<ACLGrant> aclGrants;
    };

    class DownloadObjectPartInfo {
        public:
        uint64_t partId;
        uint64_t startByte;
        uint64_t partSize;
        uint64_t status;
    };

    class GetObjectResponse : public GetObjectMetaDataResponse {
    };

    class MultiPartDownloadObjectResponse {
        public:
        std::vector<DownloadObjectPartInfo> downloadObjectPartInfo;
    };

    class GetBucketLogConfigResponse {
    public:
        std::string targetBucket;
        std::string targetPrefix;
    };

    class HeadBucketResponse {
        public:
        bool isExist;
    };

    class UploadObjectPartInfo {
        public:
        uint64_t partId;
        uint64_t startByte;
        uint64_t partSize;
        uint64_t status;
    };

    class MultiPartUploadObjectResponse {
        public:
        std::vector<UploadObjectPartInfo> uploadObjectPartInfo;
    };

    class GetUploadIdResponse {
        public:
        std::string uploadId;
    };

    class PutObjectPartResponse {
        public:
        uint64_t startByte;
        std::string etag;
    };
}

#endif  // OBSCTX_CLOUD_SERVICE_RESPONSE_H
