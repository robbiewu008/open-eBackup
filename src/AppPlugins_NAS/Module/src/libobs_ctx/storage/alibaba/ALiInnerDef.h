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
#ifndef OBSCTX_ALI_INNER_DEF_H
#define OBSCTX_ALI_INNER_DEF_H

#include <string>
#include "alibabacloud/oss/OssClient.h"

namespace Module {
    class ListServiceData : public ListBucketsResponse {
    public:
        int bucketListSize {0};
    };

    class GetObjectMetaDataResult : public GetObjectMetaDataResponse {
    public:
        bool AddSysDefMetaData(const std::string& name, const std::string& value)
        {
            if (name == "Date" || name == "x-oss-request-id" || name == "x-oss-server-time") {
                return true;
            }
            sysDefMetaData[name] = value;
            return true;
        }
        bool AddUserDefMetaData(const std::string& name, const std::string& value)
        {
            userDefMetaData[name] = value;
            return true;
        }
    };

    class MultiPartUploadObjectCallBackData : public MultiPartUploadObjectResponse {
    public:
        MultiPartUploadObjectCallbackFun *callBack = nullptr;
        void* callBackData = nullptr;
        int partCount = 0;
        uint64_t partSize = 0;
    };

}  // namespace Module

#endif  // OBSCTX_ALI_INNER_DEF_H
