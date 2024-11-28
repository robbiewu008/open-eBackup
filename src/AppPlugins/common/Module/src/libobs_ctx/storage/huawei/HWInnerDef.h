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
#ifndef OBSCTX_HW_INNER_DEF_H
#define OBSCTX_HW_INNER_DEF_H

#include <unordered_set>
#include "eSDKOBS.h"
#include "common/CloudServiceUtils.h"

namespace Module {
    const int MAX_BUCKET_NUM = 200;
    const int MAX_BUCKET_NAME_LEN = 256;
    const int MAX_OWNER_ID_LEN = 128;
    const std::unordered_set<std::string> SUPPORT_RECOVER_SYS_META_DATA_NAME_LIST {
        "ContentType", "WebsiteRedirectLocation", "Expires"};

    class listServiceData : public ListBucketsResponse {
    public:
        int headerPrinted {0};
        int allDetails {0};
        obs_status ret_status {OBS_STATUS_BUTT};
        int bucketListSize {0};
        std::string ownerId;
    };

    class ListObjectCallBackData : public ListObjectsResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
    };

    class GetObjectMetaDataCallBackData : public GetObjectMetaDataResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;

        bool AddSysDefMetaData(const char* name, const char* value)
        {
            if (name == nullptr || value == nullptr) {
                return false;
            }
            if (SUPPORT_RECOVER_SYS_META_DATA_NAME_LIST.count(name) == 0) {
                return false;
            }
            sysDefMetaData[name] = value;
            return true;
        }
        bool AddUserDefMetaData(const char* name, const char* value)
        {
            if (name == nullptr || value == nullptr) {
                return false;
            }
            userDefMetaData[name] = value;
            return true;
        }
    };

    class GetObjectCallBackData : public GetObjectResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
        uint8_t* buffer = nullptr;
        int bufferSize = 0;
        int doneSize = 0;
    };

    class MultiPartDownloadObjectCallBackData : public MultiPartDownloadObjectResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
    };

    class HeadBucketCallBackData : public HeadBucketResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
    };

    class MultiPartUploadObjectCallBackData : public MultiPartUploadObjectResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
        MultiPartUploadObjectCallbackFun *callBack = nullptr;
        void* callBackData = nullptr;
        int partCount = 0;
        uint64_t partSize = 0;
    };

    class GetUploadIdCallData : public GetUploadIdResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
    };

    class PutObjectPartCallData : public PutObjectPartResponse {
    public:
        obs_status retStatus = OBS_STATUS_BUTT;
        char* bufPtr = nullptr;
        uint64_t partSize;
    };
}

#endif  // OBSCTX_HW_INNER_DEF_H
