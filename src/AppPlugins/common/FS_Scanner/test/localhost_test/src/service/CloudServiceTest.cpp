/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: CloudServiceInterface stub for object storage.
* Author: w00444223
* Create: 2024-02-01
*/

#include "log/Log.h"
#include "securec.h"
#include "common/CloudServiceUtils.h"
#include "CloudServiceTest.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <thread>
#include <fstream>
#include <boost/filesystem.hpp>

using namespace Module;

namespace {
    const std::string MODULE_NAME = "CloudServiceTest";
    const int NUM256 = 256;
    const uint64_t SIZE_MB = 1024 * 1024;
    std::map<std::string, std::atomic<uint32_t>> g_failCnt {};
}

static ListObjectsContent TestFillObjectCont(std::string key, std::string etag, uint64_t size, uint64_t modify)
{
    ListObjectsContent object {};
    object.etag = etag;
    object.key = key;
    object.size = size;
    object.lastModified = modify;
    return object;
}

OBSResult CloudServiceTest::CheckConnect(const std::unique_ptr<CheckConnectRequest>& request)
{
    return OBSResult();
}

OBSResult CloudServiceTest::ListBuckets(
    const std::unique_ptr<ListBucketsRequest> &request, std::unique_ptr<ListBucketsResponse> &resp)
{
    OBSResult result;
    resp = std::make_unique<ListBucketsResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for ListBucketsResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    return result;
}

OBSResult CloudServiceTest::GetBucketACL(
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
    resp = std::make_unique<GetBucketACLResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for GetBucketACLResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    ACLGrant aclText {};
    aclText.grantType = 1;
    aclText.userId = "abc";
    aclText.userType = "123";
    aclText.permission = 0;
    resp->aclGrants.emplace_back(aclText);

    return result;
}

OBSResult CloudServiceTest::ListObjects(
    const std::unique_ptr<ListObjectsRequest>& request, std::unique_ptr<ListObjectsResponse>& resp)
{
    OBSResult result {};
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

    resp = std::make_unique<ListObjectsResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for ListObjectsResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    INFOLOG("check ListObjects: %s", request->prefix.c_str());
    if (request->prefix.empty()) {
        resp->contents.emplace_back(TestFillObjectCont("file11", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("file12", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("subdir1/file21", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("subdir1/file22", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("subdir2/file31", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("subdir2/file32", "md5_value", SIZE_MB, 98765432101));
    } else if (request->prefix == "subdir1/") {
        resp->contents.emplace_back(TestFillObjectCont("subdir1/file21", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("subdir1/file22", "md5_value", SIZE_MB, 98765432101));
    } else if (request->prefix == "subdir2/") {
        resp->contents.emplace_back(TestFillObjectCont("subdir2/file31", "md5_value", SIZE_MB, 98765432101));
        resp->contents.emplace_back(TestFillObjectCont("subdir2/file32", "md5_value", SIZE_MB, 98765432101));
    }

    return result;
}

OBSResult CloudServiceTest::GetObjectMetaData(
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

    resp = std::make_unique<GetObjectMetaDataResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectMetaDataResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    resp->sysDefMetaData = {{"Sysdef1", "firstmetadata"}};
    resp->userDefMetaData = {{"Userdef1", "sencondmetadata"}};

    return result;
}

OBSResult CloudServiceTest::GetObjectACL(
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

    resp = std::make_unique<GetObjectACLResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectACLResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    ACLGrant aclText {};
    aclText.grantType = 1;
    aclText.userId = "def";
    aclText.userType = "456";
    aclText.permission = 1;
    resp->aclGrants.emplace_back(aclText);

    return result;
}

OBSResult CloudServiceTest::GetObject(
    const std::unique_ptr<GetObjectRequest>& request, std::unique_ptr<GetObjectResponse>& resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::MultiPartDownloadObject(
    const std::unique_ptr<MultiPartDownloadObjectRequest>& request,
    std::unique_ptr<MultiPartDownloadObjectResponse>& resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::GetBucketLogConfig(
    const std::unique_ptr<GetBucketLogConfigRequest>& request,
    std::unique_ptr<GetBucketLogConfigResponse>& resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest> &request,
    std::unique_ptr<MultiPartUploadObjectResponse> &resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::IsBucketExist(
    const std::unique_ptr<HeadBucketRequest> &request, std::unique_ptr<HeadBucketResponse> &resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::CreateBucket(const std::unique_ptr<CreateBucketRequest> &request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::GetUploadId(
    const std::unique_ptr<GetUploadIdRequest>& request, std::unique_ptr<GetUploadIdResponse>& resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::PutObjectPart(
    const std::unique_ptr<PutObjectPartRequest> &request, std::unique_ptr<PutObjectPartResponse> &resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::CompletePutObjectPart(
    const std::unique_ptr<CompletePutObjectPartRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::AbortPutObjectPart(
    const std::unique_ptr<CompletePutObjectPartRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::DeleteBucket(const std::unique_ptr<DeleteBucketRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::DeleteObject(const std::unique_ptr<DeleteObjectRequest>& request)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::PutObject(const std::unique_ptr<PutObjectPartRequest>& request,
            std::unique_ptr<PutObjectPartResponse>& resp)
{
    OBSResult result;
    return result;
}

bool CloudServiceTest::IsSimulateFail(std::string& key, uint32_t flagBit)
{
    if (!m_testPara.IsFlagSet(flagBit) || (m_testPara.maxFailNum == 0)) {
        return false;
    }

    auto item = g_failCnt.find(key);
    if (item == g_failCnt.end()) {
        g_failCnt.emplace(key, 1);
        return true;
    }

    if (item->second < m_testPara.maxFailNum) {
        item->second++;
        return true;
    }

    return false;
}
