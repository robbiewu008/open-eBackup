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
#include "Log.h"
#include "securec.h"
#include "common/CloudServiceUtils.h"
#include "HWInnerDef.h"
#include "HWSDKCallback.h"
#include "HWCloudService.h"

using namespace Module;

namespace {
const std::string MODULE_NAME = "HWCloudService";
const int DEFAULT_CONNECTTIMEOUT_MS = 120000;
const int DEFAULT_CONNECTTIMEOUT_S = 120;
const int DEFAULT_LIST_TIMEOUT_MS = 10000;
const int DEFAULT_LIST_TIMEOUT_S = 10;
const int MAX_OBJECT_ACL_ATTRIBUTE_LENGTH = 100;
const int NUM10 = 10;
const int NUM256 = 256;
}  // namespace

void HWCloudService::InitBasicOptions(obs_options& option)
{
    init_obs_options(&option);
    option.bucket_options.host_name = String2CharPtr(m_verifyInfo.endPoint);
    option.bucket_options.access_key = String2CharPtr(m_verifyInfo.accessKey);
    option.bucket_options.secret_access_key = String2CharPtr(m_verifyInfo.secretKey);
    option.bucket_options.bucket_list_type = OBS_BUCKET_LIST_ALL;
    option.bucket_options.certificate_info = nullptr;
    option.bucket_options.uri_style = OBS_URI_STYLE_PATH;
    option.request_options.http2_switch = OBS_HTTP2_CLOSE;
    option.bucket_options.protocol = m_verifyInfo.useHttps ? OBS_PROTOCOL_HTTPS : OBS_PROTOCOL_HTTP;
    option.bucket_options.bucket_type = OBS_BUCKET_OBJECT;
    option.request_options.connect_time = DEFAULT_CONNECTTIMEOUT_MS;
    option.request_options.max_connected_time = DEFAULT_CONNECTTIMEOUT_S;
    option.request_options.bbr_switch = OBS_BBR_CLOSE;
    option.request_options.auth_switch = m_authSwitch;

    if (m_verifyInfo.useProxy) {
        option.request_options.proxy_host = String2CharPtr(m_verifyInfo.proxyHostName);
        std::string proxyAuth = m_verifyInfo.proxyUserName + ':' + m_verifyInfo.proxyUserPwd;
        option.request_options.proxy_auth = String2CharPtr(proxyAuth);
    }

    if (m_verifyInfo.useHttps) {
        option.bucket_options.certificate_info = String2CharPtr(m_verifyInfo.certHttps);
    }

    PrintInitOptions(option);
}

void HWCloudService::PrintInitOptions(obs_options& option)
{
    auto null2EmptyStr = [] (const char* cPtr, bool anonymize = false) {
        if (cPtr == nullptr) {
            return "empty";
        } else {
            return anonymize ? "not empty" : cPtr;
        }
    };
    HCP_Log(DEBUG, MODULE_NAME) << "host_name - " << null2EmptyStr(option.bucket_options.host_name) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "access_key - " << null2EmptyStr(option.bucket_options.access_key) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "secret_access_key - "
        << null2EmptyStr(option.bucket_options.secret_access_key, true) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "protocol - " << option.bucket_options.protocol << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "certificate_info - "
        << null2EmptyStr(option.bucket_options.certificate_info, true) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "auth_switch - " << option.request_options.auth_switch << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxy_host - " << null2EmptyStr(option.request_options.proxy_host) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxy_auth - "
        << null2EmptyStr(option.request_options.proxy_auth, true) << HCPENDLOG;
}

bool HWCloudService::CheckRetryAndWait(
    const RetryConfig& retryConfig, int retryCount, obs_status retStatus, OBSResult& result)
{
    result.storageType = StorageType::HUAWEI;

    if (OBS_STATUS_OK == retStatus) {
        result.result = ResultType::SUCCESS;
        return false;
    }
    result.result = ResultType::FAILED;
    result.errorCode = std::to_string(retStatus);
    result.errorDesc = obs_get_status_name(retStatus);

    if (IsErrorRetryable(retStatus) && retryConfig.isRetryable && retryCount <= retryConfig.retryNum) {
        HCP_Log(WARN, MODULE_NAME) << "request failed, need retry, retry time " << retryCount << HCPENDLOG;
        sleep(retryConfig.retryInterval);
        return true;
    } else {
        return false;
    }
}

OBSResult HWCloudService::CheckConnect(const std::unique_ptr<CheckConnectRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    auto listBucketsRequest = std::make_unique<ListBucketsRequest>();
    if (listBucketsRequest == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "listBucketsRequest is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    listBucketsRequest->retryConfig = request->retryConfig;

    std::unique_ptr<ListBucketsResponse> listBucketsResponse;

    return ListBuckets(listBucketsRequest, listBucketsResponse);
}

OBSResult HWCloudService::ListBuckets(
    const std::unique_ptr<ListBucketsRequest>& request, std::unique_ptr<ListBucketsResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.request_options.connect_time = DEFAULT_LIST_TIMEOUT_MS;
    option.request_options.max_connected_time = DEFAULT_LIST_TIMEOUT_S;

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        listServiceData data;

        if (option.request_options.auth_switch == obs_auth_switch::OBS_S3_TYPE) {
            obs_list_service_handler listHandler = {{NULL, &HWCloudServiceCompleteCallback}, &HWCloudServiceS3Callback};
            list_bucket(&option, &listHandler, &data);
        } else {
            obs_list_service_obs_handler listHandler = {
                {NULL, &HWCloudServiceCompleteCallback}, &HWCloudServiceCallback};
            list_bucket_obs(&option, &listHandler, &data);
        }

        retStatus = data.ret_status;
        if (data.ret_status == OBS_STATUS_OK) {
            resp = std::make_unique<ListBucketsResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for ListBucketsResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            HCP_Log(DEBUG, MODULE_NAME) << "list bucket successfully." << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "list bucket failed."
                << " error - " << obs_get_status_name(data.ret_status)
                << " bucket - " << request->bucketName
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));
    
    return result;
}

OBSResult HWCloudService::GetBucketACL(
    const std::unique_ptr<GetBucketACLRequest>& request, std::unique_ptr<GetBucketACLResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "list object with empty bucket name" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    HCP_Log(DEBUG, MODULE_NAME) << "GetBucketACL request: " << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "bucketName - " << request->bucketName << HCPENDLOG;

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        manager_acl_info* aclinfo = MallocAclInfo();

        get_bucket_acl(&option, aclinfo, &responseHandler, &retStatus);

        if (OBS_STATUS_OK == retStatus) {
            resp = std::make_unique<GetBucketACLResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetBucketACLResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                FreeAclInfo(&aclinfo);
                break;
            }
            ExtractAclInfo(aclinfo, resp->aclGrants, request->isNewGet);
            resp->ownerId = aclinfo->owner_id;
            resp->ownerDisplayName = aclinfo->owner_display_name;
            HCP_Log(DEBUG, MODULE_NAME) << "GetBucketACL success - "
                << " size of aclGrants - " << resp->aclGrants.size()
                << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get bucket acl failed"
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << HCPENDLOG;
        }
        FreeAclInfo(&aclinfo);
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

bool HWCloudService::CheckAndPrintListObjectsParams(
    const std::unique_ptr<ListObjectsRequest>& request, OBSResult& result)
{
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return false;
    }
    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "list object with empty bucket name" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return false;
    }

    HCP_Log(DEBUG, MODULE_NAME) << "ListObjects request: " << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "bucketName - " << request->bucketName << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "prefix - " << request->prefix << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "marker - " << request->marker << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "delimiter - " << request->delimiter << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "maxkeys - " << request->maxkeys << HCPENDLOG;

    return true;
}

OBSResult HWCloudService::ListObjects(
    const std::unique_ptr<ListObjectsRequest>& request, std::unique_ptr<ListObjectsResponse>& resp)
{
    OBSResult result;
    if (!CheckAndPrintListObjectsParams(request, result)) {
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    const char* prefix = String2CharPtr(request->prefix);
    const char* marker = String2CharPtr(request->marker);
    const char* delimiter = String2CharPtr(request->delimiter);
    const int maxkeys = request->maxkeys;

    obs_list_objects_handler listBucketObjectsHandler = {{NULL, &ListObjectCompleteCallback}, &ListObjectsCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        ListObjectCallBackData data;

        list_bucket_objects(&option, prefix, marker, delimiter, maxkeys, &listBucketObjectsHandler, &data);

        retStatus = data.retStatus;
        if (OBS_STATUS_OK == data.retStatus) {
            resp = std::make_unique<ListObjectsResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for ListObjectsResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            CheckAndAddDelimiter(request->delimiter, resp->commonPrefixes);
            HCP_Log(DEBUG, MODULE_NAME) << "ListObjects success - "
                << " isTruncated - " << resp->isTruncated
                << " nextMarker - " << resp->nextMarker
                << " size of content - " << resp->contents.size()
                << " size of commonPrefixes - " << resp->commonPrefixes.size()
                << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "list object failed"
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::GetObjectMetaData(
    const std::unique_ptr<GetObjectMetaDataRequest>& request, std::unique_ptr<GetObjectMetaDataResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get object meta with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    HCP_Log(DEBUG, MODULE_NAME) << "GetObjectMetaData request: " << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "bucketName - " << request->bucketName << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "key - " << request->key << HCPENDLOG;

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_object_info objectinfo;
    memset_s(&objectinfo, sizeof(obs_object_info), 0, sizeof(obs_object_info));
    objectinfo.key = String2CharPtr(request->key);
    objectinfo.version_id = String2CharPtr(request->versionId);

    obs_response_handler responseHandler = {&GetObjectMetaDataPropertiesCallback, &GetObjectMetaDataCompleteCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        GetObjectMetaDataCallBackData data;

        get_object_metadata(&option, &objectinfo, nullptr, &responseHandler, &data);

        retStatus = data.retStatus;
        if (OBS_STATUS_OK == data.retStatus) {
            resp = std::make_unique<GetObjectMetaDataResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectMetaDataResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            HCP_Log(DEBUG, MODULE_NAME) << "GetObjectMetaData success - "
                << " size of sysDefMetaData - " << resp->sysDefMetaData.size()
                << " size of userDefMetaData - " << resp->userDefMetaData.size()
                << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get object meta data failed"
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::GetObjectACL(
    const std::unique_ptr<GetObjectACLRequest>& request, std::unique_ptr<GetObjectACLResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get object acl with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        manager_acl_info* aclinfo = MallocAclInfo();
        aclinfo->object_info.key = String2CharPtr(request->key);
        aclinfo->object_info.version_id = String2CharPtr(request->versionId);

        get_object_acl(&option, aclinfo, &responseHandler, &retStatus);

        if (OBS_STATUS_OK == retStatus) {
            resp = std::make_unique<GetObjectACLResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectACLResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            ExtractAclInfo(aclinfo, resp->aclGrants, request->isNewGet);
            resp->ownerId = aclinfo->owner_id;
            resp->ownerDisplayName = aclinfo->owner_display_name;
            HCP_Log(DEBUG, MODULE_NAME) << "GetObjectACL success - "
                << " size of aclGrants - " << resp->aclGrants.size()
                << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get object acl failed"
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
        FreeAclInfo(&aclinfo);
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

bool HWCloudService::CheckAndPrintGetObjectParams(const std::unique_ptr<GetObjectRequest>& request, OBSResult& result)
{
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return false;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return false;
    }
    if (request->buffer == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "input buffer is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return false;
    }

    HCP_Log(DEBUG, MODULE_NAME) << "GetObject request: " << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "bucketName - " << request->bucketName << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "key - " << request->key << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "startByte - " << request->startByte << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "byteCount - " << request->byteCount << HCPENDLOG;

    return true;
}

OBSResult HWCloudService::GetObject(
    const std::unique_ptr<GetObjectRequest>& request, std::unique_ptr<GetObjectResponse>& resp)
{
    OBSResult result;
    if (!CheckAndPrintGetObjectParams(request, result)) {
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_object_info objectinfo;
    memset_s(&objectinfo, sizeof(obs_object_info), 0, sizeof(obs_object_info));
    objectinfo.key = String2CharPtr(request->key);
    objectinfo.version_id = String2CharPtr(request->versionId);

    obs_get_conditions getConditions;
    memset_s(&getConditions, sizeof(obs_get_conditions), 0, sizeof(obs_get_conditions));
    init_get_properties(&getConditions);
    getConditions.start_byte = request->startByte;
    getConditions.byte_count = request->byteCount;

    obs_get_object_handler getObjectHandler = {
        {&GetObjectMetaDataPropertiesCallback, &GetObjectCompleteCallback}, &GetObjectDataCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        GetObjectCallBackData* data = new GetObjectCallBackData();
        if (data == nullptr) {
            HCP_Log(ERR, MODULE_NAME) << "new GetObjectCallBackData failed" << HCPENDLOG;
            result.result = ResultType::FAILED;
            break;
        }
        data->buffer = request->buffer;
        data->bufferSize = request->bufferSize;

        get_object(&option, &objectinfo, &getConditions, nullptr, &getObjectHandler, data);

        retStatus = data->retStatus;
        if (OBS_STATUS_OK == data->retStatus) {
            resp = std::make_unique<GetObjectResponse>(*data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                delete data;
                break;
            }
            HCP_Log(DEBUG, MODULE_NAME) << "GetObject success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get object failed" << " error - " << obs_get_status_name(data->retStatus)
                << " bucket - " << request->bucketName << " key - " << request->key << HCPENDLOG;
        }

        delete data;
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::MultiPartDownloadObject(
    const std::unique_ptr<MultiPartDownloadObjectRequest>& request,
    std::unique_ptr<MultiPartDownloadObjectResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "multipart download object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    char* key = String2CharPtr(request->key);
    char* versionId = String2CharPtr(request->versionId);

    obs_get_conditions getConditions;
    memset_s(&getConditions, sizeof(obs_get_conditions), 0, sizeof(obs_get_conditions));
    init_get_properties(&getConditions);

    obs_download_file_configuration downloadFileConfig;
    memset_s(&downloadFileConfig, sizeof(obs_download_file_configuration), 0, sizeof(obs_download_file_configuration));
    downloadFileConfig.downLoad_file= String2CharPtr(request->downLoadTargetPath);
    downloadFileConfig.part_size = request->partSize;
    downloadFileConfig.task_num = request->taskNum;
    downloadFileConfig.check_point_file = String2CharPtr(request->checkPointFilePath);
    downloadFileConfig.enable_check_point = (int)request->enableCheckPoint;

    obs_download_file_response_handler handler = {
        {&CommonPropertiesCallback, &MultiPartDownloadObjectCompleteCallback}, &MultiPartDownloadObjectCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        MultiPartDownloadObjectCallBackData data;

        download_file(&option, key, versionId, &getConditions, nullptr, &downloadFileConfig, &handler, &data);

        retStatus = data.retStatus;
        if (OBS_STATUS_OK == data.retStatus) {
            resp = std::make_unique<MultiPartDownloadObjectResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for MultiPartDownloadObjectResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "multipart download object failed"
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName << " key - " << request->key << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::GetBucketLogConfig(
    const std::unique_ptr<GetBucketLogConfigRequest>& request, std::unique_ptr<GetBucketLogConfigResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get bucket log with empty bucket name" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = { nullptr, &ResponseCompleteCallback };

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        bucket_logging_message logging_message;
        InitBucketGetLoggingMessage(&logging_message);

        get_bucket_logging_configuration(&option, &responseHandler, &logging_message, &retStatus);

        if (OBS_STATUS_OK == retStatus) {
            resp = std::make_unique<GetBucketLogConfigResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetBucketLogConfigResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                DestroyLoggingMessage(&logging_message);
                break;
            }
            resp->targetBucket = ConvertCStr2Str(logging_message.target_bucket);
            resp->targetPrefix = ConvertCStr2Str(logging_message.target_prefix);
            HCP_Log(DEBUG, MODULE_NAME) << "GetBucketLogConfig success,"
                << " targetBucket - " << resp->targetBucket << " targetPrefix - " << resp->targetPrefix
                << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get bucket log failed"
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << HCPENDLOG;
        }
        DestroyLoggingMessage(&logging_message);
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

manager_acl_info* HWCloudService::MallocAclInfo()
{
    manager_acl_info* aclinfo = (manager_acl_info*)malloc(sizeof(manager_acl_info));
    memset_s(aclinfo, sizeof(manager_acl_info), 0, sizeof(manager_acl_info));

    aclinfo->acl_grants = (obs_acl_grant*)malloc(sizeof(obs_acl_grant) * MAX_OBJECT_ACL_ATTRIBUTE_LENGTH);
    memset_s(aclinfo->acl_grants,
        sizeof(obs_acl_grant) * MAX_OBJECT_ACL_ATTRIBUTE_LENGTH,
        0,
        sizeof(obs_acl_grant) * MAX_OBJECT_ACL_ATTRIBUTE_LENGTH);
    aclinfo->acl_grant_count_return = (int*)malloc(sizeof(int));
    *(aclinfo->acl_grant_count_return) = MAX_OBJECT_ACL_ATTRIBUTE_LENGTH;

    aclinfo->owner_id = (char*)malloc(MAX_OBJECT_ACL_ATTRIBUTE_LENGTH);
    memset_s(aclinfo->owner_id, MAX_OBJECT_ACL_ATTRIBUTE_LENGTH, 0, MAX_OBJECT_ACL_ATTRIBUTE_LENGTH);
    aclinfo->owner_display_name = (char*)malloc(MAX_OBJECT_ACL_ATTRIBUTE_LENGTH);
    memset_s(aclinfo->owner_display_name, MAX_OBJECT_ACL_ATTRIBUTE_LENGTH, 0, MAX_OBJECT_ACL_ATTRIBUTE_LENGTH);

    return aclinfo;
}

void HWCloudService::FreeAclInfo(manager_acl_info** acl)
{
    manager_acl_info* aclinfo = *acl;
    free(aclinfo->acl_grants);
    free(aclinfo->owner_display_name);
    free(aclinfo->owner_id);
    free(aclinfo->acl_grant_count_return);
    free(aclinfo);
}

void HWCloudService::ExtractAclInfo(const manager_acl_info* aclinfo,
    std::vector<ACLGrant>& aclGrants, bool isNewGet)
{
    int aclGrantCount = *aclinfo->acl_grant_count_return;
    std::string ownerId = aclinfo->owner_id;

    for (int i = 0; i < aclGrantCount; ++i) {
        ACLGrant aclGrant;

        obs_acl_grant *grant = aclinfo->acl_grants + i;
        char composedId[OBS_MAX_GRANTEE_USER_ID_SIZE +
                        OBS_MAX_GRANTEE_DISPLAY_NAME_SIZE + 16] = {0}; // 参照sdk样例添加了16这个余量，含义暂时不明

        aclGrant.grantType = grant->grantee_type;
        aclGrant.permission = grant->permission;
        aclGrant.bucketDelivered = grant->bucket_delivered;
        std::string userId = "";
        switch (grant->grantee_type) {
            case OBS_GRANTEE_TYPE_HUAWEI_CUSTOMER_BYEMAIL:
                aclGrant.userType = "Email";
                aclGrant.userId = grant->grantee.huawei_customer_by_email.email_address;
                userId = aclGrant.userId;
                break;
            case OBS_GRANTEE_TYPE_CANONICAL_USER:
                aclGrant.userType = "UserID";
                snprintf_s(composedId, sizeof(composedId), sizeof(composedId) - 1,
                    "%s:%s", grant->grantee.canonical_user.id,
                    grant->grantee.canonical_user.display_name);
                aclGrant.userId = composedId;
                userId = grant->grantee.canonical_user.id;
                break;
            case OBS_GRANTEE_TYPE_ALL_OBS_USERS:
                aclGrant.userType = "Group";
                aclGrant.userId = "Authenticated AWS Users"; // 使用aws通用接口标准
                break;
            default:
                aclGrant.userType = "Group";
                aclGrant.userId = "All Users";
                break;
        }
        if (!isNewGet) {
            aclGrants.emplace_back(aclGrant);
            continue;
        }
        if (userId == ownerId) {
            aclGrants.emplace_back(aclGrant);
        }
    }
}

void HWCloudService::InitBucketGetLoggingMessage(bucket_logging_message *loggingMessage)
{
    loggingMessage->target_bucket = (char *)malloc(sizeof(char)*OBS_MAX_HOSTNAME_SIZE);
    memset_s(loggingMessage->target_bucket, OBS_MAX_HOSTNAME_SIZE, 0, OBS_MAX_HOSTNAME_SIZE);
    loggingMessage->target_bucket_size = OBS_MAX_HOSTNAME_SIZE;

    loggingMessage->target_prefix = (char *)malloc(sizeof(char)*OBS_MAX_KEY_SIZE);
    memset_s(loggingMessage->target_prefix, OBS_MAX_KEY_SIZE, 0, OBS_MAX_KEY_SIZE);
    loggingMessage->target_prefix_size = OBS_MAX_KEY_SIZE;

    loggingMessage->acl_grants = (obs_acl_grant*)malloc(sizeof(obs_acl_grant)*OBS_MAX_ACL_GRANT_COUNT);
    memset_s(loggingMessage->acl_grants,
        sizeof(obs_acl_grant) * OBS_MAX_ACL_GRANT_COUNT,
        0,
        sizeof(obs_acl_grant) * OBS_MAX_ACL_GRANT_COUNT);
    loggingMessage->acl_grant_count = (int *)malloc(sizeof(int));
    *(loggingMessage->acl_grant_count) = 0;
}

void HWCloudService::DestroyLoggingMessage(bucket_logging_message *loggingMessage)
{
    free(loggingMessage->target_bucket);
    free(loggingMessage->target_prefix);
    free(loggingMessage->acl_grants);
    free(loggingMessage->acl_grant_count);
}

OBSResult HWCloudService::IsBucketExist(const std::unique_ptr<HeadBucketRequest> &request,
    std::unique_ptr<HeadBucketResponse> &resp)
{
    obs_options option;
    OBSResult result;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler headHandler = {nullptr, &HeadBucketCompleteCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        HeadBucketCallBackData data;
        obs_head_bucket(&option, &headHandler, &data);

        retStatus = data.retStatus;
        if (data.retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "head bucket successfully." << HCPENDLOG;
            data.isExist = true;
            resp = std::make_unique<HeadBucketResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for HeadBucketResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
            }
        } else if (data.retStatus == OBS_STATUS_HttpErrorForbidden ||
            data.retStatus == OBS_STATUS_HttpErrorNotFound ||
            data.retStatus == OBS_STATUS_NoSuchBucket) {
            HCP_Log(DEBUG, MODULE_NAME) << "head bucket successfully, bucket not exist." << HCPENDLOG;
            data.isExist = false;
            resp = std::make_unique<HeadBucketResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for HeadBucketResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
            }
            retStatus = OBS_STATUS_OK;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "head bucket failed." << obs_get_status_name(data.retStatus) << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::CreateBucket(const std::unique_ptr<CreateBucketRequest> &request)
{
    obs_options option;
    OBSResult result;
    obs_status retStatus = OBS_STATUS_BUTT;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = {NULL, &ResponseCompleteCallback};

    int retryCount = 0;
    do {
        create_bucket(&option, (obs_canned_acl)request->cannedAcl, NULL, &responseHandler, &retStatus);

        if (retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "create bucket successfully." << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "create bucket failed." << obs_get_status_name(retStatus) << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest>& request,
    std::unique_ptr<MultiPartUploadObjectResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "multipart upload object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    char* key = String2CharPtr(request->key);

    obs_upload_file_configuration uploadFileConfig;
    memset_s(&uploadFileConfig, sizeof(obs_upload_file_configuration), 0, sizeof(obs_upload_file_configuration));
    uploadFileConfig.upload_file= String2CharPtr(request->upLoadTargetPath);
    uploadFileConfig.part_size = request->partSize;
    uploadFileConfig.task_num = request->taskNum;
    uploadFileConfig.check_point_file = String2CharPtr(request->checkPointFilePath);
    uploadFileConfig.enable_check_point = (int)request->enableCheckPoint;

    obs_upload_file_response_handler handler = {
        {&MultiPartUploadObjectPropertiesCallback, &MultiPartUploadObjectCompleteCallback},
        &MultiPartUploadObjectCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        MultiPartUploadObjectCallBackData data;
        data.callBack = request->callBack;
        data.partSize = request->partSize;
        data.callBackData = request->callBackData;

        upload_file(&option, key, nullptr, &uploadFileConfig, &handler, &data);

        retStatus = data.retStatus;
        if (OBS_STATUS_OK == data.retStatus) {
            HCP_Log(DEBUG, MODULE_NAME) << "MultiPartUploadObject success" << HCPENDLOG;
            resp = std::make_unique<MultiPartUploadObjectResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for MultiPartUploadObjectResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "multipart upload object failed."
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName << " key - " << request->key << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty() || request->ownerId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "set object acl with empty bucket name or object key or ownerId" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    obs_status retStatus;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};
    manager_acl_info* aclinfo = MallocAclInfo();
    aclinfo->object_info.key = String2CharPtr(request->key);
    aclinfo->object_info.version_id = String2CharPtr(request->versionId);
    if (strcpy_s(aclinfo->owner_id,
        MAX_OBJECT_ACL_ATTRIBUTE_LENGTH, String2CharPtr(request->ownerId)) != 0) {
        HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
    }
    if (strcpy_s(aclinfo->owner_display_name,
        MAX_OBJECT_ACL_ATTRIBUTE_LENGTH, String2CharPtr(request->ownerDisplayName)) != 0) {
        HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
    }
    SetAclGrantInfoInner(aclinfo, request->aclGrants);
    int retryCount = 0;
    do {
        retStatus = OBS_STATUS_BUTT;
        set_object_acl(&option, aclinfo, &responseHandler, &retStatus);
        if (OBS_STATUS_OK == retStatus) {
            HCP_Log(DEBUG, MODULE_NAME) << "set object acl successfully." << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "set object acl failed."
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    FreeAclInfo(&aclinfo);
    return result;
}

OBSResult HWCloudService::SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->ownerId.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "set bucket acl with empty bucket name" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    obs_status retStatus;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};
    manager_acl_info* aclinfo = MallocAclInfo();
    if (strcpy_s(aclinfo->owner_id,
        MAX_OBJECT_ACL_ATTRIBUTE_LENGTH, String2CharPtr(request->ownerId)) != 0) {
        HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
    }
    if (strcpy_s(aclinfo->owner_display_name,
        MAX_OBJECT_ACL_ATTRIBUTE_LENGTH, String2CharPtr(request->ownerDisplayName)) != 0) {
        HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
    }
    SetAclGrantInfoInner(aclinfo, request->aclGrants);
    int retryCount = 0;
    do {
        retStatus = OBS_STATUS_BUTT;
        set_bucket_acl(&option, aclinfo, &responseHandler, &retStatus);
        if (OBS_STATUS_OK == retStatus) {
            HCP_Log(DEBUG, MODULE_NAME) << "set bucket acl successfully." << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "set bucket acl failed."
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    FreeAclInfo(&aclinfo);
    return result;
}

void HWCloudService::SetAclGrantInfoInner(manager_acl_info* aclinfo, const std::vector<ACLGrant>& aclGrants)
{
    *(aclinfo->acl_grant_count_return) = aclGrants.size();
    for (size_t i = 0; i < aclGrants.size(); ++i) {
        aclinfo->acl_grants[i].grantee_type = static_cast<obs_grantee_type>(aclGrants[i].grantType);
        aclinfo->acl_grants[i].permission = static_cast<obs_permission>(aclGrants[i].permission);
        aclinfo->acl_grants[i].bucket_delivered = static_cast<obs_bucket_delivered>(aclGrants[i].bucketDelivered);

        if (aclGrants[i].userType == "Email") {
            if (strcpy_s(aclinfo->acl_grants[i].grantee.huawei_customer_by_email.email_address,
                OBS_MAX_GRANTEE_EMAIL_ADDRESS_SIZE, aclGrants[i].userId.c_str()) != 0) {
                HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
            }
        } else if (aclGrants[i].userType == "UserID") {
            size_t pos = aclGrants[i].userId.find(":");
            if (pos != std::string::npos) {
                std::string id = aclGrants[i].userId.substr(0, pos);
                std::string name = aclGrants[i].userId.substr(pos + 1, aclGrants[i].userId.size());
                if (strcpy_s(aclinfo->acl_grants[i].grantee.canonical_user.id,
                    OBS_MAX_GRANTEE_USER_ID_SIZE, id.c_str()) != 0) {
                    HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
                }
                if (strcpy_s(aclinfo->acl_grants[i].grantee.canonical_user.display_name,
                    OBS_MAX_GRANTEE_DISPLAY_NAME_SIZE, name.c_str()) != 0) {
                    HCP_Log(WARN, MODULE_NAME) << "strcpy_s failed" << HCPENDLOG;
                }
            }
        }
    }
}

OBSResult HWCloudService::SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get object meta with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    obs_status retStatus;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_object_info objectinfo;
    memset_s(&objectinfo, sizeof(obs_object_info), 0, sizeof(obs_object_info));
    objectinfo.key = String2CharPtr(request->key);
    objectinfo.version_id = String2CharPtr(request->versionId);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};
    obs_put_properties put_properties;
    init_put_properties(&put_properties);
    SetObjectMetaInfoInner(put_properties, request->sysDefMetaData, request->userDefMetaData);

    int retryCount = 0;
    do {
        retStatus = OBS_STATUS_BUTT;
        set_object_metadata(&option, &objectinfo, &put_properties, nullptr, &responseHandler, &retStatus);

        if (OBS_STATUS_OK == retStatus) {
            HCP_Log(DEBUG, MODULE_NAME) << "set object meta data successfully." << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "set object meta data failed."
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));
    if (put_properties.meta_data != NULL) {
        free(put_properties.meta_data);
        put_properties.meta_data = NULL;
    }

    return result;
}

void HWCloudService::SetObjectMetaInfoInner(obs_put_properties &property,
    const std::unordered_map<std::string, std::string> &sysData,
    const std::unordered_map<std::string, std::string> &userData)
{
    property.metadata_action = OBS_REPLACE;
    for (auto iter = sysData.begin(); iter != sysData.end(); iter++) {
        if (iter->first == "ContentType") {
            property.content_type = String2CharPtr(iter->second);
        } else if (iter->first == "WebsiteRedirectLocation") {
            property.website_redirect_location = String2CharPtr(iter->second);
        } else if (iter->first == "Expires") {
            char* trmpPtr;
            property.expires = std::strtoll(iter->second.c_str(), &trmpPtr, NUM10); // 10进制
        }
    }

    property.meta_data_count = userData.size();
    if (property.meta_data_count > 0) {
        int i = 0;
        property.meta_data = (obs_name_value*)malloc(sizeof(obs_name_value) * property.meta_data_count);
        for (auto iter = userData.begin(); iter != userData.end(); iter++) {
            property.meta_data[i].name = String2CharPtr(iter->first);
            property.meta_data[i].value = String2CharPtr(iter->second);
            i++;
        }
    }
}

OBSResult HWCloudService::GetUploadId(
    const std::unique_ptr<GetUploadIdRequest>& request, std::unique_ptr<GetUploadIdResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get upload id with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    char* key = String2CharPtr(request->key);
    char uploadId[NUM256] = {0};
    GetUploadIdCallData data;

    obs_put_properties putProperties;
    putProperties.meta_data = NULL;
    init_put_properties(&putProperties);
    SetObjectMetaInfoInner(putProperties, request->sysDefMetaData, request->userDefMetaData);

    obs_response_handler handler = {nullptr, &GetUploadIdCompleteCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        initiate_multi_part_upload(&option, key, NUM256, uploadId, &putProperties, 0, &handler, &data);

        retStatus = data.retStatus;
        if (data.retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "get upload id success" << HCPENDLOG;
            data.uploadId = ConvertCStr2Str(uploadId);
            resp = std::make_unique<GetUploadIdResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetUploadIdResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get upload id failed."
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));
    if (putProperties.meta_data != NULL) {
        free(putProperties.meta_data);
    }

    return result;
}

OBSResult HWCloudService::PutObjectPart(
    const std::unique_ptr<PutObjectPartRequest> &request, std::unique_ptr<PutObjectPartResponse> &resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "put object part with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    char* key = String2CharPtr(request->key);

    obs_upload_part_info uploadPartInfo;
    memset_s(&uploadPartInfo, sizeof(obs_upload_part_info), 0, sizeof(obs_upload_part_info));

    uploadPartInfo.upload_id = String2CharPtr(request->uploadId);

    obs_put_properties putProperties;
    init_put_properties(&putProperties);

    obs_upload_handler handler = {
        {&PutPartObjectPropertiesCallback, &PutPartObjectCompleteCallback}, &PutPartObjectCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        uploadPartInfo.part_number = request->partNumber;

        PutObjectPartCallData data;
        data.startByte = request->startByte;
        data.bufPtr = request->bufPtr;
        data.partSize = request->partSize;
        upload_part(&option, key, &uploadPartInfo, request->partSize, &putProperties, 0, &handler, &data);

        retStatus = data.retStatus;
        if (data.retStatus == OBS_STATUS_OK) {
            resp = std::make_unique<PutObjectPartResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for PutObjectPartResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "put object part failed."
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::CompletePutObjectPart(
    const std::unique_ptr<CompletePutObjectPartRequest>& request)
{
    OBSResult result;

    if (request == nullptr || request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "paraments err request bucket name or object key is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->isFailed) {
        HCP_Log(ERR, MODULE_NAME) << "abort" << HCPENDLOG;
        return AbortPutObjectPart(request);
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    char* key = String2CharPtr(request->key);
    char* uploadId = String2CharPtr(request->uploadId);
    int tmpNum = request->uploadInfo.size();

    std::unique_ptr<obs_complete_upload_Info[]> uploadInfoPtr = std::make_unique<obs_complete_upload_Info[]>(tmpNum);
    int partIndex = 0;
    std::sort(request->uploadInfo.begin(), request->uploadInfo.end(), AscendingSort);
    for (const auto &part : request->uploadInfo) {
        uploadInfoPtr[partIndex].part_number = part.partNumber;
        uploadInfoPtr[partIndex].etag = String2CharPtr(part.etag);
        partIndex++;
        HCP_Log(DEBUG, MODULE_NAME) << "Part num: " << part.partNumber << ", etag: " << std::string(part.etag)
                                    << HCPENDLOG;
    }

    obs_put_properties putProperties;
    init_put_properties(&putProperties);

    obs_complete_multi_part_upload_handler handler = {
        {NULL, &ResponseCompleteCallback}, &CompletePutPartObjectCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        complete_multi_part_upload(
            &option, key, uploadId, tmpNum, uploadInfoPtr.get(), &putProperties, &handler, &retStatus);

        if (retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "complete put object successfully. " << request->bucketName << " "
                                        << request->key << HCPENDLOG;
        } else if (retStatus == OBS_STATUS_NoSuchUpload && retryCount >= 1) {
            // 因为连接超时等原因，在第二次重试时第一次的complete请求已经ok，适配该情景
            HCP_Log(WARN, MODULE_NAME) << "put object successfully, but NoSuchUpload" << HCPENDLOG;
            result.result = ResultType::SUCCESS;
            return result;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "complete put object failed." << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName << " key - " << request->key << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::AbortPutObjectPart(
    const std::unique_ptr<CompletePutObjectPartRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "abort put object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        abort_multi_part_upload(
            &option, String2CharPtr(request->key), String2CharPtr(request->uploadId), &responseHandler, &retStatus);

        if (retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "abort put object successfully." << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "abort put object failed."
                << " error - " << obs_get_status_name(retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::DeleteBucket(const std::unique_ptr<DeleteBucketRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "Delete bucket with empty bucket name" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);
    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};
    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        delete_bucket(&option, &responseHandler, &retStatus);
        if (retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "Delete bucket success: " << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "Delete bucket failed, error - "
                << obs_get_status_name(retStatus) << ", bucketname:" << request->bucketName <<HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::DeleteObject(const std::unique_ptr<DeleteObjectRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "Delete object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    obs_object_info objectinfo;
    memset_s(&objectinfo, sizeof(obs_object_info), 0, sizeof(obs_object_info));
    objectinfo.key = String2CharPtr(request->key);
    objectinfo.version_id = String2CharPtr(request->versionId);

    obs_response_handler responseHandler = {nullptr, &ResponseCompleteCallback};
    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        delete_object(&option, &objectinfo, &responseHandler, &retStatus);
        if (retStatus == OBS_STATUS_OK) {
            HCP_Log(DEBUG, MODULE_NAME) << "Delete object success: " << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "Delete object failed, error - "
                << obs_get_status_name(retStatus) << ", object:" << request->key <<HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

OBSResult HWCloudService::PutObject(
    const std::unique_ptr<PutObjectPartRequest>& request, std::unique_ptr<PutObjectPartResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "put object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    obs_options option;
    InitBasicOptions(option);
    option.bucket_options.bucket_name = String2CharPtr(request->bucketName);

    char* key = String2CharPtr(request->key);

    obs_put_properties putProperties;
    init_put_properties(&putProperties);
    SetObjectMetaInfoInner(putProperties, request->sysDefMetaData, request->userDefMetaData);

    obs_put_object_handler handler = {
        {&PutPartObjectPropertiesCallback, &PutPartObjectCompleteCallback}, PutPartObjectCallback};

    int retryCount = 0;
    obs_status retStatus = OBS_STATUS_BUTT;
    do {
        PutObjectPartCallData data;
        data.startByte = request->startByte;
        data.bufPtr = request->bufPtr;
        data.partSize = request->partSize;

        put_object(&option, key, request->partSize, &putProperties, 0, &handler, &data);

        retStatus = data.retStatus;
        if (data.retStatus == OBS_STATUS_OK) {
            resp = std::make_unique<PutObjectPartResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for PutObjectPartResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "put object failed."
                << " error - " << obs_get_status_name(data.retStatus)
                << " bucket - " << request->bucketName
                << " key - " << request->key
                << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, retStatus, result));

    return result;
}

bool HWCloudService::IsErrorRetryable(obs_status retStatus)
{
    // eSDK retryable errorcode defalut
    if (obs_status_is_retryable(retStatus)) {
        return true;
    }
    // timeout no response 10060 need to retry
    if (OBS_STATUS_XmlParseFailure == retStatus) {
        return true;
    }
    return false;
}

void HWCloudService::CheckAndAddDelimiter(const std::string &delimiter, std::vector<std::string> &commonPrefixes)
{
    for (std::string &tempCommonPrefix : commonPrefixes) {
        if (tempCommonPrefix.length() < delimiter.length() ||
            tempCommonPrefix.substr(tempCommonPrefix.length() - delimiter.length()) != delimiter) {
            tempCommonPrefix += delimiter;
        }
    }
}