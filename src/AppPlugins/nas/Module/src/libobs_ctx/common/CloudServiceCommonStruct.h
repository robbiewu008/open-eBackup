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
#ifndef CLOUD_SERVICE_COMMON_STRUCT_H
#define CLOUD_SERVICE_COMMON_STRUCT_H
#include <string>

namespace Module {
    enum class StorageType {
        OTHER,
        PACIFIC,
        HUAWEI, // 华为云
        ALI,    // 阿里云
    };

    enum class ResultType {
        SUCCESS,
        FAILED
    };

    enum class ObjectOperation {
        UNKNOWN,
        ADD,
        MODIFY,
        DELETE,
        ADD_OR_MODIFY
    };

    enum class ObjectTarget {
        UNKNOWN,
        OBJECT,
        METADATA,
        ACL
    };

    struct BucketLogInfo {
        ObjectOperation objectOperation = ObjectOperation::UNKNOWN;
        ObjectTarget objectTarget = ObjectTarget::UNKNOWN; // 写入objectlist时忽略
        std::string bucketName; // 写入objectlist时忽略
        std::string objectName;
        std::string operateTime;
        std::string statusCode; // 写入objectlist时忽略
    };
}

#endif // CLOUD_SERVICE_COMMON_STRUCT_H
