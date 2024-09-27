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
#include "ALiCloudService.h"
#include <string>
#include <regex>
#include "securec.h"
#include "Log.h"
#include "ALiInnerDef.h"
#include "common/CloudServiceUtils.h"

using namespace Module;
using namespace AlibabaCloud::OSS;
using namespace std;

namespace {
const std::string MODULE_NAME = "ALiCloudService";
const std::string SCHEME_HTTP = "http://";
const std::string SCHEME_HTTPS = "https://";
const std::string REGEX_URL = "(http|https)://([^/:]+)(:\\d+)?";
const unsigned int HTTP_DEFAULT_PORT = 80;
const unsigned int HTTPS_DEFAULT_PORT = 443;
const long DEFAULT_CONNECTTIMEOUT_MS = 120000;
const long CHECK_CONNECTION_CONNECTTIMEOUT_MS = 10000;
const int SCHEMA_IN_URL_INDEX = 1;
const int IP_IN_URL_INDEX = 2;
const int PORT_IN_URL_INDEX = 3;
}  // namespace

std::unique_ptr<OssClient> ALiCloudService::InitBasicOptions(ClientConfiguration& option)
{
    option.scheme = Http::Scheme::HTTP;
    if (option.requestTimeoutMs != CHECK_CONNECTION_CONNECTTIMEOUT_MS) {
        option.requestTimeoutMs = DEFAULT_CONNECTTIMEOUT_MS;
    }
    if (option.connectTimeoutMs != CHECK_CONNECTION_CONNECTTIMEOUT_MS) {
        option.connectTimeoutMs = DEFAULT_CONNECTTIMEOUT_MS;
    }
    option.isCname = false;    // Cname域名用于將域名解析到另一個域名，默认false
    std::string endPoint = SCHEME_HTTP + m_verifyInfo.endPoint;
    if (m_verifyInfo.useHttps) {
        option.scheme = Http::Scheme::HTTPS;
        endPoint = SCHEME_HTTPS + m_verifyInfo.endPoint;
        if (!m_verifyInfo.caPath.empty() || !m_verifyInfo.caFile.empty()) {
            option.verifySSL = true;
            option.caPath = m_verifyInfo.caPath;
            option.caFile = m_verifyInfo.caFile;
        } else {
            option.verifySSL = false;
        }
    }
    if (m_verifyInfo.useProxy) {
        option.proxyScheme = Http::Scheme::HTTP;
        std::string proxyScheme;
        std::string proxyHost;
        unsigned int proxyPort = HTTP_DEFAULT_PORT;
        if (!SplitUrl(m_verifyInfo.proxyHostName, proxyScheme, proxyHost, proxyPort)) {
            HCP_Log(ERR, MODULE_NAME) << "error url(" << m_verifyInfo.proxyHostName << "), check please." << HCPENDLOG;
            return nullptr;
        }
        option.proxyHost = proxyHost;
        option.proxyPort = proxyPort;
        option.proxyUserName = m_verifyInfo.proxyUserName;
        option.proxyPassword = m_verifyInfo.proxyUserPwd;
    }
    PrintInitOptions(option);
    return std::make_unique<OssClient>(endPoint, m_verifyInfo.accessKey, m_verifyInfo.secretKey, option);
}

void ALiCloudService::PrintInitOptions(ClientConfiguration &option)
{
    auto null2EmptyStr = [] (const std::string str, bool anonymize = false) {
        if (str.empty()) {
            return "empty";
        } else {
            return anonymize ? "not empty" : str.c_str();
        }
    };
    HCP_Log(DEBUG, MODULE_NAME) << "host_name - " << m_verifyInfo.endPoint << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "access_key - " << m_verifyInfo.accessKey << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "secret_access_key - " << null2EmptyStr(m_verifyInfo.secretKey, true) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "protocol - " << std::to_string(option.scheme) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxyScheme - " << std::to_string(option.proxyScheme) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxy_host - " << option.proxyHost << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxy_port - " << std::to_string(option.proxyPort) << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxy_UserName - " << option.proxyUserName << HCPENDLOG;
    HCP_Log(DEBUG, MODULE_NAME) << "proxy_Password - " << null2EmptyStr(option.proxyPassword, true) << HCPENDLOG;
}

template <class OutcomeStruct>
bool ALiCloudService::CheckRetryAndWait(
    const RetryConfig &retryConfig, int retryCount, const OutcomeStruct &outcome, OBSResult &result)
{
    result.storageType = StorageType::ALI;

    if (outcome.isSuccess()) {
        result.result = ResultType::SUCCESS;
        return false;
    }
    result.result = ResultType::FAILED;
    result.errorCode = outcome.error().Code();
    result.errorDesc = outcome.error().Message();
    HCP_Log(ERR, MODULE_NAME) << "error code:" << outcome.error().Code()
                              << " , error message:" << outcome.error().Message() << HCPENDLOG;

    if (retryConfig.isRetryable && retryCount < retryConfig.retryNum) {
        HCP_Log(WARN, MODULE_NAME) << "request failed, need retry, retry time " << retryCount << HCPENDLOG;
        sleep(retryConfig.retryInterval);
        return true;
    } else {
        return false;
    }
}

bool ALiCloudService::SplitUrl(const std::string &url, std::string &protocol, std::string &ip, unsigned int &port)
{
    std::regex pattern(REGEX_URL);
    std::smatch result;
    if (std::regex_match(url, result, pattern)) {
        protocol = result[SCHEMA_IN_URL_INDEX];
        ip = result[IP_IN_URL_INDEX];
        std::string tempPort = result[PORT_IN_URL_INDEX];
        if (tempPort.length() > 0) {
            port = std::stoi(tempPort.substr(1));
        }
        return true;
    }
    return false;
}

OBSResult ALiCloudService::CheckConnect(const std::unique_ptr<CheckConnectRequest>& request)
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

OBSResult ALiCloudService::ListBuckets(
    const std::unique_ptr<ListBucketsRequest>& request, std::unique_ptr<ListBucketsResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    ListBucketsOutcome outcome;
    ClientConfiguration option;
    option.requestTimeoutMs = CHECK_CONNECTION_CONNECTTIMEOUT_MS;
    option.connectTimeoutMs = CHECK_CONNECTION_CONNECTTIMEOUT_MS;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    int retryCount = 0;
    do {
        AlibabaCloud::OSS::ListBucketsRequest aLiRequest;
        outcome = client->ListBuckets(aLiRequest);
        if (outcome.isSuccess()) {
            std::vector<Bucket> buckets = outcome.result().Buckets();
            ListServiceData data;
            data.bucketListSize = buckets.size();
            for (int i = 0; i < data.bucketListSize; i++) {
                data.bucketList.push_back(buckets[i].Name());
            }
            resp = std::make_unique<ListBucketsResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for ListBucketsResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "list bucket failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));
    
    return result;
}

OBSResult ALiCloudService::GetBucketACL(
    const std::unique_ptr<GetBucketACLRequest>& request, std::unique_ptr<GetBucketACLResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::GetBucketAclRequest aliRequest(request->bucketName);
    GetBucketAclOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->GetBucketAcl(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "get bucket acl success" << HCPENDLOG;
            resp = std::make_unique<GetBucketACLResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetBucketACLResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            if (!request->isNewGet) {
                ACLGrant aclGrant;
                aclGrant.grantType = 0;
                aclGrant.bucketDelivered = 0;
                aclGrant.permission = static_cast<int>(outcome.result().Acl());
                resp->aclGrants.emplace_back(aclGrant);
            }
            resp->ownerId = outcome.result().Owner().Id();
            resp->ownerDisplayName = outcome.result().Owner().DisplayName();
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get bucket acl failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::ListObjects(
    const std::unique_ptr<ListObjectsRequest>& request, std::unique_ptr<ListObjectsResponse>& resp)
{
    OBSResult result;
    if (request == nullptr || request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr or bucketName is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::ListObjectsRequest aliRequest(request->bucketName);
    aliRequest.setPrefix(request->prefix);
    aliRequest.setMarker(request->marker);
    aliRequest.setDelimiter(request->delimiter);
    aliRequest.setMaxKeys(request->maxkeys);
    ListObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->ListObjects(aliRequest);
        if (outcome.isSuccess()) {
            resp = std::make_unique<ListObjectsResponse>();
            if (resp == nullptr) {
                result.result = ResultType::FAILED;
                break;
            }
            resp->isTruncated = outcome.result().IsTruncated();
            resp->nextMarker = outcome.result().NextMarker();
            resp->commonPrefixes = outcome.result().CommonPrefixes();
            SaveObjectContent(request->delimiter, outcome.result().ObjectSummarys(), resp->contents);
            HCP_Log(DEBUG, MODULE_NAME) << "ListObjects success - " << " isTruncated - " << resp->isTruncated
                    << " nextMarker - " << resp->nextMarker << " size of content - " << resp->contents.size()
                    << " size of commonPrefixes - " << resp->commonPrefixes.size() << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "list object failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::GetObjectMetaData(const std::unique_ptr<GetObjectMetaDataRequest>& request,
    std::unique_ptr<GetObjectMetaDataResponse>& resp)
{
    OBSResult result;
    if (request == nullptr || request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr or request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::HeadObjectRequest aliRequest(request->bucketName, request->key);
    aliRequest.setVersionId(request->versionId);
    GetObjectMetaDataResult data;
    ObjectMetaDataOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->HeadObject(aliRequest);
        if (outcome.isSuccess()) {
            auto metadata = outcome.result();
            HCP_Log(DEBUG, MODULE_NAME) << "key:" << request->key
                << "LastModified:" << (uint64_t)GmtToTimestamp(metadata.LastModified())
                << "etag:" << metadata.ETag() << "size:" << metadata.ContentLength() << HCPENDLOG;
            data.lastModified = (uint64_t)GmtToTimestamp(metadata.LastModified());
            data.etag = metadata.ETag();
            data.size = metadata.ContentLength();
            for (auto& meta : metadata.HttpMetaData()) {
                data.AddSysDefMetaData(meta.first, meta.second);
            }
            for (auto& meta : metadata.UserMetaData()) {
                data.AddUserDefMetaData(meta.first, meta.second);
            }
            resp = std::make_unique<GetObjectMetaDataResponse>(data);
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectMetaDataResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get object meta failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::GetObjectACL(
    const std::unique_ptr<GetObjectACLRequest>& request, std::unique_ptr<GetObjectACLResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::GetObjectAclRequest aliRequest(request->bucketName, request->key);
    aliRequest.setVersionId(request->versionId);
    GetObjectAclOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->GetObjectAcl(aliRequest);
        if (outcome.isSuccess()) {
            resp = std::make_unique<GetObjectACLResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectACLResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            if (!request->isNewGet) {
                ACLGrant aclGrant;
                aclGrant.grantType = 0;
                aclGrant.bucketDelivered = 0;
                aclGrant.permission = static_cast<int>(outcome.result().Acl());
                resp->aclGrants.emplace_back(aclGrant);
            }
            resp->ownerId = outcome.result().Owner().Id();
            resp->ownerDisplayName = outcome.result().Owner().DisplayName();
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get object acl failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::GetObject(
    const std::unique_ptr<GetObjectRequest>& request, std::unique_ptr<GetObjectResponse>& resp)
{
    OBSResult result;
    if (request == nullptr || request->buffer == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request or buffer is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::GetObjectRequest aliRequest(request->bucketName, request->key);
    aliRequest.setVersionId(request->versionId);
    int64_t start = static_cast<int64_t>(request->startByte);
    int64_t end = static_cast<int64_t>(request->startByte + request->byteCount);
    aliRequest.setRange(start, end, true);
    GetObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->GetObject(aliRequest);
        if (outcome.isSuccess()) {
            resp = std::make_unique<GetObjectResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            if (outcome.result().Content() == nullptr) {
                break;
            }
            while (outcome.result().Content()->good()) {
                outcome.result().Content()->read(reinterpret_cast<char*>(request->buffer), request->bufferSize);
            }
            SaveObjectMetaData(outcome.result().Metadata(), resp);
            HCP_Log(DEBUG, MODULE_NAME) << "key:" << request->key << "LastModified:" << resp->lastModified
                << "etag:" << resp->etag << "size:" << resp->size << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get object acl failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::MultiPartDownloadObject(
    const std::unique_ptr<MultiPartDownloadObjectRequest>& request,
    std::unique_ptr<MultiPartDownloadObjectResponse>& resp)
{
    OBSResult result;
    result.result = ResultType::FAILED;
    return result;
}

OBSResult ALiCloudService::GetBucketLogConfig(
    const std::unique_ptr<GetBucketLogConfigRequest>& request,
    std::unique_ptr<GetBucketLogConfigResponse>& resp)
{
    OBSResult result;
    result.result = ResultType::FAILED;
    return result;
}

OBSResult ALiCloudService::IsBucketExist(
    const std::unique_ptr<HeadBucketRequest> &request, std::unique_ptr<HeadBucketResponse> &resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    StringOutcome outcome("success");
    int retryCount = 0;
    do {
        resp = std::make_unique<HeadBucketResponse>();
        if (resp == nullptr) {
            HCP_Log(ERR, MODULE_NAME) << "make unique for HeadBucketResponse failed" << HCPENDLOG;
            result.result = ResultType::FAILED;
            break;
        }
        resp->isExist = client->DoesBucketExist(request->bucketName);
        HCP_Log(DEBUG, MODULE_NAME) << "IsBucketExist: " << resp->isExist << HCPENDLOG;
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::CreateBucket(const std::unique_ptr<CreateBucketRequest> &request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::CreateBucketRequest aliRequest(request->bucketName);
    CreateBucketOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->CreateBucket(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "create bucket success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "create bucket failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

static void ProgressCallback(size_t increment, int64_t transfered, int64_t total, void* userData)
{
    if (userData) {
        MultiPartUploadObjectCallBackData* data = (MultiPartUploadObjectCallBackData*) userData;
        if (data->callBack) {
            if (data->partSize > 0) {
                data->partCount = transfered / data->partSize;
            }
            data->callBack(increment, data->partCount, data->callBackData);
        }
    }
}

OBSResult ALiCloudService::MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest> &request,
    std::unique_ptr<MultiPartUploadObjectResponse> &resp)
{
    OBSResult result;
    if (request == nullptr || request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr or bucketName is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    MultiPartUploadObjectCallBackData data;
    data.callBack = request->callBack;
    data.partSize = request->partSize;
    data.callBackData = request->callBackData;

    AlibabaCloud::OSS::UploadObjectRequest aliRequest(request->bucketName, request->key,
    request->upLoadTargetPath);
    if (request->enableCheckPoint) {
        size_t pos = request->checkPointFilePath.find_last_of("/");
        if (pos != std::string::npos) {
            std::string tmpDir = request->checkPointFilePath.substr(0, pos);
            aliRequest.setCheckpointDir(tmpDir);
        }
    }
    aliRequest.setThreadNum(request->taskNum);
    AlibabaCloud::OSS::TransferProgress progressCallback = {ProgressCallback, &data};
    aliRequest.setTransferProgress(progressCallback);
    PutObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->ResumableUploadObject(aliRequest);
        if (outcome.isSuccess()) {
            resp = std::make_unique<MultiPartUploadObjectResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for MultiPartUploadObjectResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
        } else {
            HCP_Log(ERR, MODULE_NAME) << "multi part upload object failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty() || request->aclGrants.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or aclGrants is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::SetObjectAclRequest aliRequest(request->bucketName, request->key);
    aliRequest.setAcl(static_cast<CannedAccessControlList>(request->aclGrants[0].permission));
    aliRequest.setVersionId(request->versionId);
    SetObjectAclOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->SetObjectAcl(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "set object acl success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "set object acl failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty() || request->aclGrants.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or aclGrants is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::SetBucketAclRequest aliRequest(
        request->bucketName, static_cast<CannedAccessControlList>(request->aclGrants[0].permission));
    VoidOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->SetBucketAcl(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "set bucket acl success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "set bucket acl failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    auto meta = CreateObjectMetaData(request->sysDefMetaData, request->userDefMetaData);

    CopyObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->ModifyObjectMeta(request->bucketName, request->key, meta);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "set object meta success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "set object meta failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::GetUploadId(
    const std::unique_ptr<GetUploadIdRequest>& request, std::unique_ptr<GetUploadIdResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::InitiateMultipartUploadRequest aliRequest(request->bucketName, request->key);

    aliRequest.MetaData() = CreateObjectMetaData(request->sysDefMetaData, request->userDefMetaData);

    InitiateMultipartUploadOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->InitiateMultipartUpload(aliRequest);
        if (outcome.isSuccess()) {
            resp = std::make_unique<GetUploadIdResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for GetUploadIdResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            resp->uploadId = outcome.result().UploadId();
        } else {
            HCP_Log(ERR, MODULE_NAME) << "get upload id failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::PutObjectPart(const std::unique_ptr<PutObjectPartRequest> &request,
    std::unique_ptr<PutObjectPartResponse> &resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::shared_ptr<std::iostream> content = std::make_shared<std::stringstream>();
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr || content == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client or content is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    content->write(request->bufPtr, request->partSize);
    AlibabaCloud::OSS::UploadPartRequest aliRequest(request->bucketName, request->key, content);
    aliRequest.setContentLength(request->partSize);
    aliRequest.setUploadId(request->uploadId);
    aliRequest.setPartNumber(request->partNumber);
    PutObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->UploadPart(aliRequest);
        if (outcome.isSuccess()) {
            resp = std::make_unique<PutObjectPartResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for PutObjectPartResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            resp->startByte = request->startByte + request->partSize;
            resp->etag = outcome.result().ETag();
        } else {
            HCP_Log(ERR, MODULE_NAME) << "put object part failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::CompletePutObjectPart(
    const std::unique_ptr<CompletePutObjectPartRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->isFailed) {
        HCP_Log(ERR, MODULE_NAME) << "abort" << HCPENDLOG;
        return AbortPutObjectPart(request);
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    PartList partETagList;
    std::sort(request->uploadInfo.begin(), request->uploadInfo.end(), AscendingSort);
    for (const auto &iter : request->uploadInfo) {
        Part part(iter.partNumber, iter.etag);
        partETagList.push_back(part);
        HCP_Log(DEBUG, MODULE_NAME) << "Part num: " << iter.partNumber << HCPENDLOG;
        HCP_Log(DEBUG, MODULE_NAME) << "Part etag: " << iter.etag << HCPENDLOG;
    }

    AlibabaCloud::OSS::CompleteMultipartUploadRequest aliRequest(request->bucketName, request->key);
    aliRequest.setUploadId(request->uploadId);
    aliRequest.setPartList(partETagList);
    CompleteMultipartUploadOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->CompleteMultipartUpload(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "complete put object part success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "complete put object part failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::AbortPutObjectPart(const std::unique_ptr<CompletePutObjectPartRequest>& request)
{
    OBSResult result;
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::AbortMultipartUploadRequest aliRequest(request->bucketName, request->key, request->uploadId);
    VoidOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->AbortMultipartUpload(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "abort put object part success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "abort put object part failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::DeleteBucket(const std::unique_ptr<DeleteBucketRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::DeleteBucketRequest aliRequest(request->bucketName);
    VoidOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->DeleteBucket(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "delete bucket success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "delete bucket failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::DeleteObject(const std::unique_ptr<DeleteObjectRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    AlibabaCloud::OSS::DeleteObjectRequest aliRequest(request->bucketName, request->key);
    DeleteObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->DeleteObject(aliRequest);
        if (outcome.isSuccess()) {
            HCP_Log(DEBUG, MODULE_NAME) << "delete object success" << HCPENDLOG;
        } else {
            HCP_Log(ERR, MODULE_NAME) << "delete object failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

OBSResult ALiCloudService::PutObject(
    const std::unique_ptr<PutObjectPartRequest>& request,
    std::unique_ptr<PutObjectPartResponse>& resp)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "request bucketName or key is empty" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    AlibabaCloud::OSS::ClientConfiguration option;
    std::shared_ptr<std::iostream> content = std::make_shared<std::stringstream>();
    std::unique_ptr<OssClient> client = InitBasicOptions(option);
    if (client == nullptr || content == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "client or content is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    content->write(request->bufPtr, request->partSize);
    auto meta = CreateObjectMetaData(request->sysDefMetaData, request->userDefMetaData);

    PutObjectOutcome outcome;
    int retryCount = 0;
    do {
        outcome = client->PutObject(request->bucketName, request->key, content, meta);
        if (outcome.isSuccess()) {
            resp = std::make_unique<PutObjectPartResponse>();
            if (resp == nullptr) {
                HCP_Log(ERR, MODULE_NAME) << "make unique for PutObjectPartResponse failed" << HCPENDLOG;
                result.result = ResultType::FAILED;
                break;
            }
            resp->startByte = request->startByte + request->partSize;
            resp->etag = outcome.result().ETag();
        } else {
            HCP_Log(ERR, MODULE_NAME) << "put object failed." << outcome.error().Code() << HCPENDLOG;
        }
    } while (CheckRetryAndWait(request->retryConfig, ++retryCount, outcome, result));

    return result;
}

AlibabaCloud::OSS::ObjectMetaData ALiCloudService::CreateObjectMetaData(
    const std::unordered_map<std::string, std::string> &sysData,
    const std::unordered_map<std::string, std::string> &userData)
{
    HeaderCollection headers;
    for (auto const&header : sysData) {
        if (header.first == "Date") {
            continue;
        }
        headers[header.first] = header.second;
    }
    for (auto const&header : userData) {
        std::string key("x-oss-meta-");
        key.append(header.first);
        headers[key] = header.second;
    }

    return ObjectMetaData(headers);
}

void ALiCloudService::SaveObjectContent(
    const std::string& delimiter, const ObjectSummaryList& objectList, std::vector<ListObjectsContent>& contents)
{
    for (auto const &obj : objectList) {
        if (IsEndsWith(obj.Key(), delimiter)) {
            // 以 delimiter 结尾的是目录
            HCP_Log(DEBUG, MODULE_NAME) << "Skip " << obj.Key() << HCPENDLOG;
            continue;
        }
        ListObjectsContent tmpContent;
        tmpContent.key = obj.Key();
        tmpContent.lastModified = UtcToTimestamp(obj.LastModified());
        tmpContent.etag = obj.ETag();
        tmpContent.size = obj.Size();
        tmpContent.ownerId = obj.Owner().Id();
        tmpContent.ownerDisplayName = obj.Owner().DisplayName();
        tmpContent.storageClass = obj.StorageClass();
        tmpContent.type = obj.Type();
        contents.emplace_back(tmpContent);
    }
}

void ALiCloudService::SaveObjectMetaData(const ObjectMetaData &metadata, std::unique_ptr<GetObjectResponse>& resp)
{
    GetObjectMetaDataResult data;
    data.lastModified = (uint64_t)GmtToTimestamp(metadata.LastModified());
    data.etag = metadata.ETag();
    data.size = metadata.ContentLength();
    for (auto& meta : metadata.HttpMetaData()) {
        data.AddSysDefMetaData(meta.first, meta.second);
    }
    for (auto& meta : metadata.UserMetaData()) {
        data.AddUserDefMetaData(meta.first, meta.second);
    }
    resp->lastModified = data.lastModified;
    resp->etag = data.etag;
    resp->size = data.size;
    resp->sysDefMetaData = data.sysDefMetaData;
    resp->userDefMetaData = data.userDefMetaData;
}
