/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */
#ifndef CLOUD_SERVICE_TEST_H
#define CLOUD_SERVICE_TEST_H

#include <vector>
#include <string>
#include <atomic>
#include "eSDKOBS.h"
#include "interface/CloudServiceInterface.h"

namespace BackupTest {
    constexpr uint32_t GET_BUCKET_ACL_ERR = 0x00000001;
    constexpr uint32_t GET_OBJECT_ACL_ERR = 0x00000002;
    constexpr uint32_t LIST_OBJECTS_ERR = 0x00000004;
    constexpr uint32_t GET_OBJECT_ERR = 0x00000008;
    constexpr uint32_t GET_OBJECT_META_ERR = 0x00000010;
    constexpr uint32_t CREATE_BUCKET_ERR = 0x00000020;
    constexpr uint32_t SET_BUCKET_ACL_ERR = 0x00000040;
    constexpr uint32_t SET_OBJECT_ACL_ERR = 0x00000080;
    constexpr uint32_t SET_OBJECT_META_ERR = 0x00000100;
    constexpr uint32_t PUT_OBJECT_PART_ERR = 0x00000200;
}

struct BackupTestPara {
    std::string dataPath;
    std::string dstPath;
    uint32_t maxFailNum {0};
    uint32_t flag {0};
    int64_t errorCode {0};
    void SetFlag(uint32_t bit) { flag |= bit; }
    void ClearFlag(uint32_t bit) { flag &= ~(bit); }
    bool IsFlagSet(uint32_t bit) { return (flag & bit); }
};

namespace Module {
    class CloudServiceTest : public CloudServiceInterface {
    public:
        CloudServiceTest(const StorageVerifyInfo &verifyInfo, BackupTestPara &testPara)
            : CloudServiceInterface(verifyInfo), m_testPara(testPara) {};
        ~CloudServiceTest() {};

        OBSResult CheckConnect(const std::unique_ptr<CheckConnectRequest>& request) override;
        OBSResult ListBuckets(
            const std::unique_ptr<ListBucketsRequest> &request, std::unique_ptr<ListBucketsResponse> &resp) override;
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
        OBSResult GetBucketLogConfig(const std::unique_ptr<GetBucketLogConfigRequest>& request,
            std::unique_ptr<GetBucketLogConfigResponse>& resp) override;
        OBSResult IsBucketExist(
            const std::unique_ptr<HeadBucketRequest> &request, std::unique_ptr<HeadBucketResponse> &resp) override;
        OBSResult CreateBucket(const std::unique_ptr<CreateBucketRequest> &request) override;
        OBSResult MultiPartUploadObject(const std::unique_ptr<MultiPartUploadObjectRequest> &request,
            std::unique_ptr<MultiPartUploadObjectResponse> &resp) override;
        OBSResult SetObjectACL(const std::unique_ptr<SetObjectACLRequest>& request) override;
        OBSResult SetBucketACL(const std::unique_ptr<SetBucketACLRequest>& request) override;
        OBSResult SetObjectMetaData(const std::unique_ptr<SetObjectMetaDataRequest>& request) override;
        OBSResult GetUploadId(
            const std::unique_ptr<GetUploadIdRequest>& request, std::unique_ptr<GetUploadIdResponse>& resp) override;
        OBSResult PutObjectPart(const std::unique_ptr<PutObjectPartRequest> &request,
            std::unique_ptr<PutObjectPartResponse> &resp) override;
        OBSResult CompletePutObjectPart(const std::unique_ptr<CompletePutObjectPartRequest>& request) override;
        OBSResult DeleteBucket(const std::unique_ptr<DeleteBucketRequest>& request) override;
        OBSResult DeleteObject(const std::unique_ptr<DeleteObjectRequest>& request) override;
        OBSResult PutObject(const std::unique_ptr<PutObjectPartRequest>& request,
            std::unique_ptr<PutObjectPartResponse>& resp) override;

    private:
        OBSResult AbortPutObjectPart(const std::unique_ptr<CompletePutObjectPartRequest>& request);

    private:
        bool IsSimulateFail(std::string& key, uint32_t flagBit);
        BackupTestPara m_testPara;
    };
}

#endif  // CLOUD_SERVICE_TEST_H
