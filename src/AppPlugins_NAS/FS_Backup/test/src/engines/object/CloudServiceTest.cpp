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
    uint64_t g_uploadIdVal = 0;
    std::map<std::string, uint64_t> uploadIdMap {};
    std::map<std::string, std::atomic<uint32_t>> g_failCnt {};
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

    const std::string dataDirPath = m_testPara.dataPath;
    std::vector<std::string> fileList {};
    try {
        for (const auto &entry : boost::filesystem::directory_iterator(dataDirPath)) {
            std::string fileName = entry.path().generic_string();
            fileList.emplace_back(fileName);
        }
    } catch (const boost::filesystem::filesystem_error &e) {
        HCP_Log(ERR, MODULE_NAME) << "directory_iterator() exeption: " << e.code().message() << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    if (fileList.empty()) {
        ERRLOG("NO file");
        result.result = ResultType::FAILED;
        return result;
    }

    for (auto fileName : fileList) {
        ListObjectsContent object {};
        struct stat st {};
        if (stat(fileName.c_str(), &st) == 0) {
            size_t pos = fileName.find_last_of("/");
            object.key = fileName.substr(pos + 1);
            object.lastModified = st.st_mtime;
            object.size = st.st_size;
            object.etag = std::to_string(st.st_ino);
            resp->contents.emplace_back(object);
        }
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

    std::string filePath = m_testPara.dstPath + "/" + request->bucketName + "/" + request->key;
    struct stat st {};
    if (::stat(filePath.c_str(), &st) == 0) {
        resp->lastModified = st.st_mtime;
        resp->size = st.st_size;
    }

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
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "get object with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    resp = std::make_unique<GetObjectResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for GetObjectResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    INFOLOG("Read file %s, start pos: %llu, size: %llu", request->key.c_str(), request->startByte, request->byteCount);
    if (IsSimulateFail(request->key, BackupTest::GET_OBJECT_ERR)) {
        ERRLOG("Simulate fail for %s.", request->key.c_str());
        result.result = ResultType::FAILED;
        result.storageType = StorageType::HUAWEI;
        result.errorCode = std::to_string(m_testPara.errorCode);
        return result;
    }

    const std::string dataDirPath = m_testPara.dataPath;
    std::string filePath = dataDirPath + "/" + request->key;

    int fd = open(filePath.c_str(), O_RDONLY);
    if (fd == -1) {
        char errMsg[NUM256] = "\0";
        ERRLOG("Open file %s failed. errmsg: %s", request->key.c_str(), strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }
    ssize_t cnt = pread(fd, (void *)request->buffer, request->byteCount, request->startByte);
    if (cnt != request->byteCount) {
        close(fd);
        char errMsg[NUM256] = "\0";
        ERRLOG("Read file %s failed. size:%zu, errmsg: %s",
            request->key.c_str(), cnt, strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }

    close(fd);
    return result;
}

OBSResult CloudServiceTest::MultiPartDownloadObject(
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

    INFOLOG("request backet name is %s, key is %s, download file path is %s",
            request->bucketName.c_str(), request->key.c_str(), request->downLoadTargetPath.c_str());

    FILE *fp = fopen(request->downLoadTargetPath.c_str(), "w+");
    if (fp == nullptr) {
        char errMsg[NUM256] = "\0";
        ERRLOG("Open file %s failed. errmsg: %s",
            request->downLoadTargetPath.c_str(), strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }

    std::string content = "This is a key file content. Key is " + request->key + "\n";
    size_t size = fwrite(content.c_str(), 1, content.length(), fp);
    if (size <= 0) {
        char errMsg[NUM256] = "\0";
        ERRLOG("Write file %s failed. errmsg: %s",
            request->downLoadTargetPath.c_str(), strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
    }

    fclose(fp);
    return result;
}

OBSResult CloudServiceTest::MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest> &request,
    std::unique_ptr<MultiPartUploadObjectResponse> &resp)
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

    if (request->callBack) {
        request->callBack(request->partSize, 0, request->callBackData);
    }

    std::ofstream ofile(m_testPara.dstPath + "/" + request->bucketName + "/" + request->key, std::ios::out);
    std::ifstream ifile(request->upLoadTargetPath, std::ios::in);
    ofile << ifile.rdbuf();
    ifile.close();
    ofile.close();

    return result;
}

OBSResult CloudServiceTest::GetBucketLogConfig(
    const std::unique_ptr<GetBucketLogConfigRequest>& request, std::unique_ptr<GetBucketLogConfigResponse>& resp)
{
    OBSResult result;
    return result;
}

OBSResult CloudServiceTest::IsBucketExist(
    const std::unique_ptr<HeadBucketRequest> &request, std::unique_ptr<HeadBucketResponse> &resp)
{
    OBSResult result;

    resp = std::make_unique<HeadBucketResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for HeadBucketResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    std::string dirPath = m_testPara.dstPath + "/" + request->bucketName;
    struct stat st {};
    if (::stat(dirPath.c_str(), &st) != 0) {
        char errMsg[NUM256] = "\0";
        INFOLOG("%s, %s", dirPath.c_str(), strerror_r(errno, errMsg, NUM256));
        resp->isExist = false;
    } else {
        resp->isExist = S_ISDIR(st.st_mode);
    }

    DBGLOG("Bucket exist flag %d, path: %s", resp->isExist, dirPath.c_str());
    return result;
}

OBSResult CloudServiceTest::CreateBucket(const std::unique_ptr<CreateBucketRequest> &request)
{
    OBSResult result;
    std::this_thread::sleep_for(std::chrono::seconds(5));

    std::string dirPath = m_testPara.dstPath + "/" + request->bucketName;
    struct stat st {};
    if (::stat(dirPath.c_str(), &st) == 0) {
        INFOLOG("Directory %s exist.", dirPath.c_str());
        return result;
    }

    int res = mkdir(dirPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
    if (res != Module::SUCCESS) {
        ERRLOG("CreateDirectory fail for: %s", dirPath.c_str());
        result.result = ResultType::FAILED;
        return result;
    }
    return result;
}

OBSResult CloudServiceTest::SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty() || request->key.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "set object acl with empty bucket name or object key" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    INFOLOG("Set object ACL.");
    for (auto &item : request->aclGrants) {
        INFOLOG("userId : %s, userType : %s, grantType : %d, permission : %d, bucketDelivered : %d",
            item.userId.c_str(), item.userType.c_str(), item.grantType, item.permission, item.bucketDelivered);
    }

    return result;
}

OBSResult CloudServiceTest::SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request)
{
    OBSResult result;
    if (request == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "request is nullptr" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }
    if (request->bucketName.empty()) {
        HCP_Log(ERR, MODULE_NAME) << "set bucket acl with empty bucket name" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    INFOLOG("Set bucket ACL.");
    for (auto &item : request->aclGrants) {
        INFOLOG("userId : %s, userType : %s, grantType : %d, permission : %d, bucketDelivered : %d",
            item.userId.c_str(), item.userType.c_str(), item.grantType, item.permission, item.bucketDelivered);
    }

    return result;
}

OBSResult CloudServiceTest::SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request)
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

    INFOLOG("sysDefMetaData:");
    for (auto &item : request->sysDefMetaData) {
        INFOLOG("%s,%s", item.first.c_str(), item.second.c_str());
    }

    INFOLOG("userDefMetaData:");
    for (auto &item : request->userDefMetaData) {
        INFOLOG("%s,%s", item.first.c_str(), item.second.c_str());
    }

    return result;
}

OBSResult CloudServiceTest::GetUploadId(
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
    resp = std::make_unique<GetUploadIdResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for GetUploadIdResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    auto item = uploadIdMap.find(request->key);
    if (item == uploadIdMap.end()) {
        g_uploadIdVal++;
        uploadIdMap.emplace(request->key, g_uploadIdVal);
        resp->uploadId = std::to_string(g_uploadIdVal);
    } else {
        resp->uploadId = std::to_string(item->second);
    }

    return result;
}

OBSResult CloudServiceTest::PutObjectPart(
    const std::unique_ptr<PutObjectPartRequest> &request, std::unique_ptr<PutObjectPartResponse> &resp)
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

    resp = std::make_unique<PutObjectPartResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for PutObjectPartResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    INFOLOG("Write file %s, start pos: %llu, size: %llu", request->key.c_str(), request->startByte, request->partSize);
    if (IsSimulateFail(request->key, BackupTest::PUT_OBJECT_PART_ERR)) {
        ERRLOG("Simulate fail for %s.", request->key.c_str());
        result.result = ResultType::FAILED;
        result.storageType = StorageType::HUAWEI;
        result.errorCode = std::to_string(m_testPara.errorCode);
        return result;
    }

    std::string fileName = request->key + "_" + std::to_string(request->partNumber);
    std::string filePath = m_testPara.dstPath + "/" + request->bucketName + "/" + fileName;

    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT);
    if (fd == -1) {
        char errMsg[NUM256] = "\0";
        ERRLOG("Open file %s failed. errmsg: %s", request->key.c_str(), strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }

    off_t pos = lseek(fd, 0, SEEK_END);
    ssize_t cnt = pwrite(fd, request->bufPtr + request->startByte, request->partSize, pos);
    if (cnt != request->partSize) {
        close(fd);
        char errMsg[NUM256] = "\0";
        ERRLOG("Write file %s failed. size:%zu, errmsg: %s",
            request->key.c_str(), cnt, strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }

    close(fd);
    resp->startByte = request->partSize;
    resp->etag = std::to_string(request->startByte);
    return result;
}

OBSResult CloudServiceTest::CompletePutObjectPart(
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

    std::string finalFile = m_testPara.dstPath + "/" + request->bucketName + "/" + request->key;
    std::ofstream ofile(finalFile, std::ios::out);

    auto cmp = [](const UploadInfo& a, const UploadInfo& b) -> bool { return (a.partNumber < b.partNumber); };
    std::sort(request->uploadInfo.begin(), request->uploadInfo.end(), cmp);
    for (auto &it : request->uploadInfo) {
        std::string fileName = request->key + "_" + std::to_string(it.partNumber);
        std::string filePath = m_testPara.dstPath + "/" + request->bucketName + "/" + fileName;
        std::ifstream ifile(filePath, std::ios::in);
        ofile << ifile.rdbuf();
        ifile.close();
        ::remove(filePath.c_str());
    }

    ofile.close();
    auto item = uploadIdMap.find(request->key);
    if (item != uploadIdMap.end()) {
        uploadIdMap.erase(item);
    }

    return result;
}

OBSResult CloudServiceTest::AbortPutObjectPart(
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

    INFOLOG("Abort %s success.", request->key.c_str());
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
    std::string fileName = m_testPara.dstPath + "/" + request->bucketName + "/" + request->key;
    ::remove(fileName.c_str());
    return result;
}

OBSResult CloudServiceTest::PutObject(
    const std::unique_ptr<PutObjectPartRequest> &request, std::unique_ptr<PutObjectPartResponse> &resp)
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

    resp = std::make_unique<PutObjectPartResponse>();
    if (resp == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "make unique for PutObjectPartResponse failed" << HCPENDLOG;
        result.result = ResultType::FAILED;
        return result;
    }

    INFOLOG("Write file %s, start pos: %llu, size: %llu", request->key.c_str(), request->startByte, request->partSize);
    if (IsSimulateFail(request->key, BackupTest::PUT_OBJECT_PART_ERR)) {
        ERRLOG("Simulate fail for %s.", request->key.c_str());
        result.result = ResultType::FAILED;
        result.storageType = StorageType::HUAWEI;
        result.errorCode = std::to_string(m_testPara.errorCode);
        return result;
    }

    std::string filePath = m_testPara.dstPath + "/" + request->bucketName + "/" + request->key;
    int fd = open(filePath.c_str(), O_WRONLY | O_CREAT);
    if (fd == -1) {
        char errMsg[NUM256] = "\0";
        ERRLOG("Open file %s failed. errmsg: %s", request->key.c_str(), strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }

    off_t pos = lseek(fd, 0, SEEK_END);
    ssize_t cnt = pwrite(fd, request->bufPtr + request->startByte, request->partSize, pos);
    if (cnt != request->partSize) {
        close(fd);
        char errMsg[NUM256] = "\0";
        ERRLOG("Write file %s failed. size:%zu, errmsg: %s",
            request->key.c_str(), cnt, strerror_r(errno, errMsg, NUM256));
        result.result = ResultType::FAILED;
        return result;
    }

    close(fd);
    resp->startByte = request->partSize;
    resp->etag = std::to_string(request->startByte);

    INFOLOG("sysDefMetaData:");
    for (auto &item : request->sysDefMetaData) {
        INFOLOG("%s,%s", item.first.c_str(), item.second.c_str());
    }

    INFOLOG("userDefMetaData:");
    for (auto &item : request->userDefMetaData) {
        INFOLOG("%s,%s", item.first.c_str(), item.second.c_str());
    }

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

void CloudServiceTestReSet()
{
    uploadIdMap.clear();
    g_failCnt.clear();
}
