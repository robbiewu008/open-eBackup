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
#include "common/CloudServiceErrorCode.h"
#include "eSDKOBS.h"
#include "alibabacloud/oss/OssClient.h"

using namespace Module;

std::map<std::string, int64_t> CloudServiceErrorCode::m_errCodeALI = {
    {"InvalidArgument", ERROR_OBS_RESOURCE_AK_OR_SK_ERROR},
    {"InvalidAccessKeyId", ERROR_OBS_RESOURCE_AK_OR_SK_ERROR},
    {"SignatureDoesNotMatch", ERROR_OBS_RESOURCE_AK_OR_SK_ERROR},
    {"ClientError:200007", ERROR_OBS_RESOURCE_PROXY_SERVER_CONNECT_FAILED},
    {"RequestTimeTooSkewed", ERROR_OBS_RESOURCE_TIME_VERIFY_FAILED}
};
std::map<std::string, int64_t> CloudServiceErrorCode::m_errCodeALI2Linux = {
    {"RequestTimeTooSkewed", ENETUNREACH},
    {"InvalidArgument", EACCES},
    {"InvalidAccessKeyId", EACCES},
    {"SignatureDoesNotMatch", EACCES},
    {"AccessDenied", EACCES},
    {"UserDisable", EACCES},
    {"TooManyBuckets", EACCES},
    {"ClientError:200007", EAGAIN}
};

int64_t CloudServiceErrorCode::Transform2OMRP(const StorageType &storageType, const std::string &errorCode)
{
    if (storageType == StorageType::PACIFIC || storageType == StorageType::HUAWEI) {
        return GetFromHCS(errorCode);
    } else if (storageType == StorageType::ALI) {
        return GetFromALI(errorCode);
    }
    return OBS_INTERNAL_ERROR_CODE;
}

int64_t CloudServiceErrorCode::Transform2Linux(const StorageType &storageType, const std::string &errorCode)
{
    if (storageType == StorageType::PACIFIC || storageType == StorageType::HUAWEI) {
        return GetFromHCS2Linux(errorCode);
    } else if (storageType == StorageType::ALI) {
        return GetFromALI2Linux(errorCode);
    }
    return 0;
}

int64_t CloudServiceErrorCode::GetFromHCS(const std::string &errorCode)
{
    int64_t hcsErrorCode = std::stoi(errorCode);
    switch (hcsErrorCode) {
        case OBS_STATUS_ConnectionFailed:
            return ERROR_OBS_RESOURCE_ENDPOINT_CONNECT_FAILED;
        case OBS_STATUS_InvalidArgument:
        case OBS_STATUS_InvalidAccessKeyId:
        case OBS_STATUS_SignatureDoesNotMatch:
            return ERROR_OBS_RESOURCE_AK_OR_SK_ERROR;
        case OBS_STATUS_FailedToConnect:
        case OBS_STATUS_NameLookupError:
            return ERROR_OBS_RESOURCE_PROXY_SERVER_CONNECT_FAILED;
        case OBS_STATUS_ServerFailedVerification:
            return ERROR_OBS_RESOURCE_CERT_VERIFY_FAILED;
        case OBS_STATUS_RequestTimeTooSkewed:
            return ERROR_OBS_RESOURCE_TIME_VERIFY_FAILED;
        default:
            return OBS_INTERNAL_ERROR_CODE;
    }
}

int64_t CloudServiceErrorCode::GetFromHCS2Linux(const std::string &errorCode)
{
    int64_t hcsErrorCode = std::stoi(errorCode);
    switch (hcsErrorCode) {
        case OBS_STATUS_RequestTimeTooSkewed:
            return ENETUNREACH;
        case OBS_STATUS_InvalidAccessKeyId:
        case OBS_STATUS_SignatureDoesNotMatch:
        case OBS_STATUS_ServerFailedVerification:
        case OBS_STATUS_AccessDenied:
        case OBS_STATUS_InArrearOrInsufficientBalance:
        case OBS_STATUS_NotSignedUp:
            return EACCES;
        case OBS_STATUS_InsufficientStorageSpace:
            return ENOSPC;
        case OBS_STATUS_HttpErrorNotFound:
            return ENOENT;
        case OBS_STATUS_ErrorUnknown:
        case OBS_STATUS_EntityTooSmall:
            return EISDIR;
        case OBS_STATUS_ConnectionFailed:
        case OBS_STATUS_FailedToConnect:
        case OBS_STATUS_ServiceUnavailable:
            return EAGAIN;
        default:
            return -1; // Return to backup without stopping the task
    }
}

int64_t CloudServiceErrorCode::GetFromALI(const std::string &errorCode)
{
    auto it = CloudServiceErrorCode::m_errCodeALI.find(errorCode);
    if (it != CloudServiceErrorCode::m_errCodeALI.end()) {
        return it->second;
    }

    return OBS_INTERNAL_ERROR_CODE;
}

int64_t CloudServiceErrorCode::GetFromALI2Linux(const std::string &errorCode)
{
    auto it = CloudServiceErrorCode::m_errCodeALI2Linux.find(errorCode);
    if (it != CloudServiceErrorCode::m_errCodeALI2Linux.end()) {
        return it->second;
    }

    return 0;
}
