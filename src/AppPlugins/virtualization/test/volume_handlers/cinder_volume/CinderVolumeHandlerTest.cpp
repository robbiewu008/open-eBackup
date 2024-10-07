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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <iostream>
#include <memory>
#include "common/CommonMock.h"
#include "common/Structs.h"
#include "thrift_interface/ApplicationProtectBaseDataType_types.h"
#include "thrift_interface/ApplicationProtectPlugin_types.h"
#include "thrift_interface/ApplicationProtectFramework_types.h"
#include "volume_handlers/cloud_volume/cinder_volume/CinderVolumeHandler.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;
using ::testing::A;

using AppProtect::ApplicationEnvironment;
using AppProtect::ResourceResultByPage;
using AppProtect::QueryByPage;
using AppProtect::Application;
using AppProtect::ApplicationResource;
using AppProtect::ActionResult;
using AppProtect::BackupJob;

using namespace VirtPlugin;
using namespace OpenStackPlugin;
using namespace HDT_TEST;


namespace HDT_TEST_CINDER {
constexpr uint32_t VOLUME_DATABLOCK_SIZE = 4194304;  // Byte

static std::string Stub_ConfigReaderGetString(const std::string & sectionName, const std::string &keyName)
{
    return "Default";
}

static std::string Stub_ConfigReaderGetStringEmpty(const std::string & sectionName, const std::string &keyName)
{
    return "";
}

const std::string volData = "1234242424242141";

static int fsOpenRet = SUCCESS;
static int fsCloseRet = SUCCESS;
static int fsfileSizeRet = volData.length();
static bool fsRemoveRet = true;
static bool fsCreateDirRet = true;
static bool fileExistRet = true;
std::function<int32_t(std::string &, size_t)> stubReadFunc = nullptr;

std::string getVolumeResponse = "{\
    \"volume\": {\
		\"attachments\": [{\
			\"server_id\": \"85c80531-a24c-4732-9f45-228c8301478f\",\
			\"attachment_id\": \"6b7a281e-dcde-47d5-8d4a-6a4e9748111f\",\
			\"attached_at\": \"2022-11-29T02:26:39.847005\",\
			\"host_name\": null,\
			\"volume_id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
			\"device\": \"/dev/vda\",\
			\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\"\
		}],\
		\"availability_zone\": \"az236.dc236\",\
		\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
		\"encrypted\": false,\
		\"updated_at\": \"2022-11-29T02:26:39.874180\",\
		\"os-volume-replication:extended_status\": null,\
		\"replication_status\": \"disabled\",\
		\"snapshot_id\": null,\
		\"wwn\": \"658f987100b749bc4811cd7700001b24\",\
		\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
		\"size\": 40,\
		\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
		\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
		\"os-vol-mig-status-attr:migstat\": null,\
		\"metadata\": {\
			\"StorageType\": \"OceanStorV5\",\
			\"take_over_lun_wwn\": \"--\",\
			\"attached_mode\": \"rw\",\
			\"readonly\": \"False\",\
			\"lun_wwn\": \"658f987100b749bc4811cd7700001b24\"\
		},\
		\"status\": \"in-use\",\
		\"volume_image_metadata\": {\
			\"architecture\": \"x86_64\",\
			\"container_format\": \"bare\",\
			\"hw_disk_bus\": \"virtio\"\
		},\
		\"backup_id\": null,\
		\"description\": \"\",\
		\"multiattach\": false,\
		\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6948\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
		\"source_volid\": null,\
		\"consistencygroup_id\": null,\
		\"os-vol-pro-location-attr:provider_location\": \"6948\",\
		\"os-vol-mig-status-attr:name_id\": null,\
		\"name\": \"\",\
		\"bootable\": \"true\",\
		\"created_at\": \"2022-11-29T02:23:25.771826\",\
		\"volume_type\": null,\
		\"shareable\": false,\
		\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
	}\
}";

const std::string g_tokenStr =
    "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdC"
    "I6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoi"
    "dmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0Yj"
    "Q0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0s"
    "eyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOi"
    "JhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkw"
    "NzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YW"
    "FkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9u"
    "ZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZW"
    "I2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQg"
    "UHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-"
    "bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-"
    "WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-"
    "BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";
const std::string g_endpoint = "https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b";
const std::string g_volumeSnapDetailStr = "{\"snapshot\":{\"status\":\"available\",\"size\":10,\"metadata\":{},\"name\":\"VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3\",\"volume_id\":\"c1f1465b-6a63-4343-9761-7ad3e413f4ef\", \"provider_auth\": \"{\\\"lun_id\\\": \\\"6604\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\", \"created_at\":\"2022-07-21T07:00:21.862040\",\"description\":\"VirtPlugin backup. Job 3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967.\",\"updated_at\":null}}";

std::string getVolumeResponseDetaching = "{\
    \"volume\": {\
		\"attachments\": [{\
			\"server_id\": \"85c80531-a24c-4732-9f45-228c8301478f\",\
			\"attachment_id\": \"6b7a281e-dcde-47d5-8d4a-6a4e9748111f\",\
			\"attached_at\": \"2022-11-29T02:26:39.847005\",\
			\"host_name\": null,\
			\"volume_id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
			\"device\": \"/dev/vda\",\
			\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\"\
		}],\
		\"availability_zone\": \"az236.dc236\",\
		\"os-vol-host-attr:host\": \"cinder@FusionSphereOpenstack#FusionSphereOpenstack-ma\",\
		\"encrypted\": false,\
		\"updated_at\": \"2022-11-29T02:26:39.874180\",\
		\"os-volume-replication:extended_status\": null,\
		\"replication_status\": \"disabled\",\
		\"snapshot_id\": null,\
		\"wwn\": \"658f987100b749bc4811cd7700001b24\",\
		\"id\": \"787c2a44-d8f2-41de-ba04-a70fce8201bd\",\
		\"size\": 40,\
		\"user_id\": \"278c058da02b44d484dc8b99a77063de\",\
		\"os-vol-tenant-attr:tenant_id\": \"28a23331b60747bfb2bd8606ba930396\",\
		\"os-vol-mig-status-attr:migstat\": null,\
		\"metadata\": {\
			\"StorageType\": \"OceanStorV5\",\
			\"take_over_lun_wwn\": \"--\",\
			\"attached_mode\": \"rw\",\
			\"readonly\": \"False\",\
			\"lun_wwn\": \"658f987100b749bc4811cd7700001b24\"\
		},\
		\"status\": \"detaching\",\
		\"volume_image_metadata\": {\
			\"architecture\": \"x86_64\",\
			\"container_format\": \"bare\",\
			\"hw_disk_bus\": \"virtio\"\
		},\
		\"backup_id\": null,\
		\"description\": \"\",\
		\"multiattach\": false,\
		\"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"6948\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\",\
		\"source_volid\": null,\
		\"consistencygroup_id\": null,\
		\"os-vol-pro-location-attr:provider_location\": \"6948\",\
		\"os-vol-mig-status-attr:name_id\": null,\
		\"name\": \"\",\
		\"bootable\": \"true\",\
		\"created_at\": \"2022-11-29T02:23:25.771826\",\
		\"volume_type\": null,\
		\"shareable\": false,\
		\"os-vendor-ext-attr:vendor_driver_metadata\": {}\
	}\
}";

static bool Stub_Http_GetToken_Success(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = g_tokenStr;
    endPoint = g_endpoint;
    return true;
}

static int32_t Stub_SendRequestToGetVolumeInDetachingStatus(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    response->SetSuccess(true);
    response->SetGetBody(getVolumeResponseDetaching);
    response->SetStatusCode(200);
    return SUCCESS;
}

static int32_t Stub_OpenStackConfigReaderTen(std::string section, std::string keyName)
{
    return 10;
}

static std::shared_ptr<VolumeResponse> Stub_GetVolumeInDetachingStatus(VolumeRequest &request)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    response->SetSuccess(true);
    response->SetGetBody(getVolumeResponseDetaching);
    response->SetStatusCode(200);
    return response;
}

// class mockDiskDeviceFileSharedPtr : public std::shared_ptr<VirtPlugin::DiskDeviceFile>
// {
// public:
//     mockDiskDeviceFileSharedPtr(){};
//     VirtPlugin::DiskDeviceFile* get()
//     {
//         return nullptr;
//     };
// };

// static std::shared_ptr<VirtPlugin::DiskDeviceFile> Stub_DiskDeviceFileNull()
// {
//     std::shared_ptr<VirtPlugin::DiskDeviceFile> mockPtr;
//     mockPtr.reset();
//     return mockPtr;
// }

static VirtPlugin::DiskDeviceFile* Stub_DiskDeviceFileNull()
{
    return nullptr;
}

static std::shared_ptr<VolumeResponse> Stub_SendRequestToDetachServerVolumeSuccess(VolumeRequest &request,
    const std::string& volumeId)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    response->SetSuccess(true);
    response->SetStatusCode(Module::SC_ACCEPTED);
    return response;
}

static std::shared_ptr<VolumeResponse> Stub_SendRequestToAttachServerVolumeSuccess(VolumeRequest &request,
    const std::string& volumeId)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    response->SetSuccess(true);
    response->SetStatusCode(Module::SC_OK);
    return response;
}

static int g_getStatusCount = 0;

static std::shared_ptr<VolumeResponse> Stub_SendRequestToGetVolumeStatusSuccess(VolumeRequest &request)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    if (g_getStatusCount == 0) { // Get volume detail
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    }  else if (g_getStatusCount == 1) { // detach volume
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 2) { // get if volume detached
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 3) { // attach volume
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 4) { // get if volume detached
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 5) { // delete volume
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 6) { // confirm if volume deleted
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
    }
    response->Serial();
    INFOLOG("Get volume detail called %d times, status is %s", g_getStatusCount, response->GetVolume().m_status.c_str());
    return response;
}

static std::shared_ptr<VolumeResponse> Stub_SendRequestToGetVolumeStatusFail(VolumeRequest &request)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    if (g_getStatusCount == 0) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 1) { // detach volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        g_getStatusCount ++;
    } else if (g_getStatusCount == 2) { // get if volume detached
        std::string volResp = getVolumeResponse;
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 3) { // attach volume
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        response->SetGetBody(volResp);
        g_getStatusCount ++;
    } else if (g_getStatusCount == 4) { // get if volume detached
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        g_getStatusCount = 0;
    }
    response->Serial();
    INFOLOG("Get volume detail called %d times, status is %s", g_getStatusCount, response->GetVolume().m_status.c_str());
    return response;
}

static std::shared_ptr<VolumeResponse> Stub_SendRequestToCreateVolume(VolumeRequest &request)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    response->SetSuccess(true);
    response->SetGetBody(getVolumeResponse);
    response->SetStatusCode(202);
    response->Serial();
    return response;
}

static DISK_DEVICE_RETURN_CODE Stub_FlushSuccess()
{
    return DISK_DEVICE_OK;
}

static DISK_DEVICE_RETURN_CODE Stub_FlushFail()
{
    return DISK_DEVICE_ERROR;
}

static int32_t FileSystemReadMetaDataSuccess_Invoke(std::string &buf, size_t size)
{
    buf = volData;
    return volData.length();
}

static std::shared_ptr<RepositoryHandler> Stub_CinderCreateFSHandler(
    const AppProtect::StorageRepository &storageRepo)
{
    if (stubReadFunc == nullptr) {
        stubReadFunc = FileSystemReadMetaDataSuccess_Invoke;
    }
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(fsOpenRet));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(fsCloseRet));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(fsfileSizeRet));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(stubReadFunc));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(Return(SHA256_DIGEST_LENGTH));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(fileExistRet));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(fsRemoveRet));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(fsCreateDirRet));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

static DISK_DEVICE_RETURN_CODE Stub_ReadVolume(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer)
{
    return DISK_DEVICE_RETURN_CODE::DISK_DEVICE_OK;
}

static DISK_DEVICE_RETURN_CODE Stub_WriteVolume(uint64_t offsetInBytes, uint64_t &bufferSizeInBytes,
    std::shared_ptr<uint8_t[]> &buffer)
{
    return DISK_DEVICE_RETURN_CODE::DISK_DEVICE_OK;
}

static DISK_DEVICE_RETURN_CODE Stub_OpenVolume(const std::string &fileName, int owMode, const uint64_t &volumeSize)
{
    return DISK_DEVICE_RETURN_CODE::DISK_DEVICE_OK;
}

std::shared_ptr<JobHandle> FormJobHandler(BackupJob job, StorageRepository &repo)
{
    repo.__set_repositoryType(RepositoryDataType::META_REPOSITORY);
    repo.path.push_back("/tmp/");
    job.repositories.push_back(repo);
    std::shared_ptr<JobCommonInfo> jobCommonInfoPtr = std::make_shared<JobCommonInfo>(std::make_shared<BackupJob>(job));
    std::shared_ptr<JobHandle> jobHandler = std::make_shared<JobHandle>(JobType::BACKUP, jobCommonInfoPtr);
    return jobHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateRepositoryHandlerFail(const AppProtect::StorageRepository &storageRepo)
{
    static int cntCreateRepositoryHandlerFail = 0;
    if (cntCreateRepositoryHandlerFail == 0) {
        ++ cntCreateRepositoryHandlerFail;
        return nullptr;
    } else if (cntCreateRepositoryHandlerFail == 1) {
        ++ cntCreateRepositoryHandlerFail;
        return std::make_shared<FileSystemHandler>();
    } else {
        return nullptr;
    }
}
int32_t Stub_OpenSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count = 0;
    if (count == 0) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    }  else if (count == 1) { // detach volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        count ++;
    } else if (count == 2) { // get if volume detached
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count ++;
    } else if (count == 3) { // attach volume
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetGetBody(volResp);
        count ++;
    } else if (count >= 4) { // get if volume attached
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    }
    return SUCCESS;
}

int32_t Stub_OpenFailed(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int numProcess = 0;
    if (numProcess == 0) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        numProcess ++;
    } else if (numProcess == 1) { // detach volume
        response->SetSuccess(true);
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        numProcess ++;
    } else if (numProcess == 2) { // get if volume detached
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        numProcess ++;
    } else if (numProcess == 3) { // attach volume
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetGetBody(volResp);
        numProcess ++;
    } else if (numProcess == 4) { // get if volume detached
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        numProcess = 0;
    }
    return SUCCESS;
}

int32_t Stub_OpenfromSnapSucc(void *obj, const Module::HttpRequest &request, std::shared_ptr<ResponseModel> response)
{
    static int count_snap = 0;
    if (count_snap == 0) {
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        response->SetGetBody(volResp);
        count_snap ++;
    } else if (count_snap == 1) {
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count_snap ++;
    } else if (count_snap == 2) {
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count_snap ++;
    } else if (count_snap == 3) {
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        response->SetGetBody(volResp);
        count_snap ++;
    } else if (count_snap < 7) {
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count_snap ++;
    } else if (count_snap == 7) {
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count_snap ++;
    }else if (count_snap == 8) {
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetGetBody(volResp);
        count_snap ++;
    } else if (count_snap > 8) {
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetGetBody(volResp);
        count_snap ++;
    } 

    return SUCCESS;
}

static int Stub_RunShell_Scan_OK(const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
{
    static int count_shell = 0;
    if (count_shell <= 1)
    {
        count_shell ++;
        return FAILED;
    }
    else {
        cmdoutput.push_back("lrwxrwxrwx. 1 root root   9 Jul 26 00:00 virtio-10 -> ../../sda");
        return SUCCESS;
    }

}

static int Stub_RunCommand_Scan_Success(const std::string& cmdName, const std::vector<Module::CmdParam>& cmdVec, std::vector<std::string>& result,
    const Module::CmdRunUser& cmdUser)
{
    static int count_shell = 0;
    if (count_shell <= 1)
    {
        count_shell ++;
        return FAILED;
    }
    else {
        result.push_back("lrwxrwxrwx. 1 root root   9 Jul 26 00:00 virtio-10 -> ../../sda");
        return SUCCESS;
    }
}

static int Stub_RunCommand_Change_Success(const std::string& cmdName, const std::vector<Module::CmdParam>& cmdVec, std::vector<std::string>& result,
    const std::unordered_set<std::string>& whiteList, const Module::CmdRunUser& cmdUser)
{
    return SUCCESS;
}


static int Stub_RunShell_Snap_OK(const std::string &moduleName, const std::size_t &requestID,
    const std::string &cmd, const std::vector<std::string> params, std::vector<std::string> &cmdoutput,
    std::vector<std::string> &stderroutput, std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput.push_back("lrwxrwxrwx. 1 root root   9 Jul 26 00:00 virtio-1 -> ../../sda");
    return SUCCESS;
}

static int Stub_RunCommand_Snap_Success(const std::string& cmdName, const std::vector<Module::CmdParam>& cmdVec, std::vector<std::string>& cmdoutput,
    const Module::CmdRunUser& cmdUser)
{
    cmdoutput.push_back("lrwxrwxrwx. 1 root root   9 Jul 26 00:00 virtio-1 -> ../../sda");
    return SUCCESS;
}

void PreInitHashFile(const StorageRepository &repo, const VolInfo &volObj)
{
    std::shared_ptr<FileSystemHandler> fileHandler = std::make_shared<FileSystemHandler>();
    std::string hashDir = repo.path[0] + VIRT_PLUGIN_VOLUMES_HASH_DIR;
    if (!fileHandler->Exists(hashDir)) {
        fileHandler->CreateDirectory(hashDir);
    }
    std::string hashFile = hashDir + volObj.m_uuid + "_hash.record";
    fileHandler->Open(hashFile, "w");
    fileHandler->Write("2524242243324242424242424224242a");
    fileHandler->Close();
}


class CinderVolumeHandlerTest : public testing::Test {
public:
    void InitLogger()
    {
        std::string logFileName = "volume_handler_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }
    void SetUp()
    {
        InitLogger();
        if (m_buffer == nullptr) {
            m_buffer = std::make_unique<uint8_t[]>(size_t(VOLUME_DATABLOCK_SIZE));
            m_calcHashBuffer = std::make_unique<uint8_t[]>(size_t(SHA256_DIGEST_LENGTH));
            m_readHashBuffer = std::make_unique<uint8_t[]>(size_t(SHA256_DIGEST_LENGTH));
        }
        m_stub.set(ADDR(OpenStackPlugin::OpenStackTokenMgr, GetToken), StubGetToken);
        memset_s(m_buffer.get(), VOLUME_DATABLOCK_SIZE, 0, VOLUME_DATABLOCK_SIZE);
        memset_s(m_calcHashBuffer.get(), SHA256_DIGEST_LENGTH, 0, SHA256_DIGEST_LENGTH);
        memset_s(m_readHashBuffer.get(), SHA256_DIGEST_LENGTH, 0, SHA256_DIGEST_LENGTH);
        m_stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderGetString);
        m_job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
        auto jobHandler = FormJobHandler(m_job, m_repo);
        m_volObj.m_volSizeInBytes = VOLUME_DATABLOCK_SIZE;
        m_volObj.m_uuid = "10-10-10-10";
        m_cinderHandler = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
        PreInitHashFile(m_repo, m_volObj);
        m_cinderHandler->InitializeVolumeInfo("OpenStackConfig");
        m_stub.set(sleep, Stub_Sleep);
    };
    void TearDown()
    {
        m_stub.reset(ADDR(Module::ConfigReader, getString));
        m_stub.reset(ADDR(OpenStackPlugin::OpenStackTokenMgr, GetToken));
        m_stub.reset(sleep);
    };
    Stub m_stub;
    std::shared_ptr<uint8_t[]> m_buffer = nullptr;
    std::shared_ptr<uint8_t[]> m_calcHashBuffer = nullptr;
    std::shared_ptr<uint8_t[]> m_readHashBuffer = nullptr;
    std::shared_ptr<CinderVolumeHandler> m_cinderHandler = nullptr;
    VolInfo m_volObj;
    BackupJob m_job;
    StorageRepository m_repo;
};

/*
 * 测试用例： 初始化卷信息失败
 * 前置条件:  m_jobHandle为空
 * CHECK点:  InitializeVolumeInfo返回失败
 */
TEST_F(CinderVolumeHandlerTest, InitializeVolumeInfoFailNUllHandle)
{
    std::shared_ptr<JobHandle> jobHandleNull = nullptr;
    std::shared_ptr<CinderVolumeHandler> cinderHandlerFail = nullptr;
    cinderHandlerFail = std::make_shared<CinderVolumeHandler>(jobHandleNull, m_volObj, "123", "123");
    int ret = cinderHandlerFail->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, FAILED);
}

/*
 * 测试用例： 初始化卷信息失败
 * 前置条件:  m_jobHandle非空
 * CHECK点:  InitializeVolumeInfo返回失败
 */
TEST_F(CinderVolumeHandlerTest, InitializeVolumeInfoCreateRepositoryHandleFail)
{
    auto jobHandler = FormJobHandler(m_job, m_repo);
    std::shared_ptr<CinderVolumeHandler> cinderHandlerFail = nullptr;
    cinderHandlerFail = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");

    m_stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateRepositoryHandlerFail);
    int ret = cinderHandlerFail->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, FAILED);
    ret = cinderHandlerFail->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, FAILED);
    m_stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    // EXPECT_EQ(ret, FAILED);
}

TEST_F(CinderVolumeHandlerTest, InitializeVolumeInfoFailNoServerID)
{
    auto jobHandler = FormJobHandler(m_job, m_repo);
    std::shared_ptr<CinderVolumeHandler> cinderHandlerFail = nullptr;
    cinderHandlerFail = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    m_stub.reset(ADDR(Module::ConfigReader, getString));
    m_stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderGetStringEmpty);
    int ret = cinderHandlerFail->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, FAILED);
    m_stub.reset(ADDR(Module::ConfigReader, getString));
    m_stub.set(ADDR(Module::ConfigReader, getString), Stub_ConfigReaderGetString);

}


/*
 * 测试用例： 1. 全量备份cindervolumehander读数据成功 2. 增量备份读数据sha256一致
 * 前置条件:  全量备份
 * CHECK点:  1. ReadBlocks返回SUCCESS 2. ReadBlocks返回DATA_SAME_IGNORE_WRITE
 */
TEST_F(CinderVolumeHandlerTest, IncreMentBackupReadBlocksSame)
{
    m_stub.set(ADDR(DiskDeviceFile, Read), Stub_ReadVolume);
    const uint64_t offset = 0;
    uint64_t size = 10;
    memcpy_s(m_buffer.get(), size_t(VOLUME_DATABLOCK_SIZE), &volData[0], size_t(volData.length()));
    int ret = m_cinderHandler->ReadBlocks(offset, size, m_buffer, m_calcHashBuffer, m_readHashBuffer);
    EXPECT_EQ(ret, 0);
    m_cinderHandler->m_metaRepoHandler->Close();
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
    auto jobHandler = FormJobHandler(m_job, m_repo);
    m_cinderHandler = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    ret = m_cinderHandler->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, 0);
    ret = m_cinderHandler->ReadBlocks(offset, size, m_buffer, m_calcHashBuffer, m_readHashBuffer);
    EXPECT_EQ(ret, DATA_SAME_IGNORE_WRITE);
    m_cinderHandler->m_metaRepoHandler->Close();
    m_stub.reset(ADDR(DiskDeviceFile, Read));
}

/*
 * 测试用例： 1. 全量备份cindervolumehander读数据成功 2. 增量备份读数据sha256一致
 * 前置条件:  全量备份
 * CHECK点:  1. ReadBlocks返回SUCCESS 2. ReadBlocks返回DATA_SAME_IGNORE_WRITE
 */
TEST_F(CinderVolumeHandlerTest, FullBackupWriteBlocksSucc)
{
    m_stub.set(ADDR(DiskDeviceFile, Write), Stub_WriteVolume);
    const uint64_t offset = 0;
    uint64_t size = 10;
    memcpy_s(m_buffer.get(), size_t(VOLUME_DATABLOCK_SIZE), &volData[0], size_t(volData.length()));
    int ret = m_cinderHandler->WriteBlocks(offset, size, m_buffer);
    EXPECT_EQ(ret, 0);
    m_stub.reset(ADDR(DiskDeviceFile, Write));
}

/*
 * 测试用例： cindervolumehander全量备份Readblocks成功
 * 前置条件:  全量备份
 * CHECK点:
 */
TEST_F(CinderVolumeHandlerTest, FullBackupReadBlocksSUCCESS)
{
    m_stub.set(ADDR(DiskDeviceFile, Read), Stub_ReadVolume);
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    auto jobHandler = FormJobHandler(m_job, m_repo);
    m_cinderHandler = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    const uint64_t offset = 0;
    uint64_t size = 10;
    int ret = m_cinderHandler->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, 0);
    memcpy_s(m_buffer.get(), size_t(VOLUME_DATABLOCK_SIZE), &volData[0], size_t(volData.length()));
    ret = m_cinderHandler->ReadBlocks(offset, size, m_buffer, m_calcHashBuffer, m_readHashBuffer);
    m_cinderHandler->m_metaRepoHandler->Close();
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(DiskDeviceFile, Read));
}

/*
 * 测试用例： cindervolumehander全量备份Readblocks数据全为0
 * 前置条件:  全量备份
 * CHECK点:
 */
TEST_F(CinderVolumeHandlerTest, FullBackupReadBlocksAllZero)
{
    m_stub.set(ADDR(DiskDeviceFile, Read), Stub_ReadVolume);
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    auto jobHandler = FormJobHandler(m_job, m_repo);
    m_cinderHandler = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    const uint64_t offset = 0;
    uint64_t size = 10;
    int ret = m_cinderHandler->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, 0);
    ret = m_cinderHandler->ReadBlocks(offset, size, m_buffer, m_calcHashBuffer, m_readHashBuffer);
    m_cinderHandler->m_metaRepoHandler->Close();
    EXPECT_EQ(ret, DATA_ALL_ZERO_IGNORE_WRITE);
}
/*
 * 测试用例： 增量备份, cindervolumehander调用readBlocks接口返回SUCCESS
 * 前置条件:  预置文件系统中的SAH256数据
 * CHECK点: ReadBlocks返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, IncrBackupReadBlocksSucc)
{
    m_stub.set(ADDR(DiskDeviceFile, Read), Stub_ReadVolume);
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
    const uint64_t offset = 0;
    uint64_t size = 10;
    auto jobHandler = FormJobHandler(m_job, m_repo);
    m_cinderHandler = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    int ret = m_cinderHandler->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, 0);
    memcpy_s(m_buffer.get(), size_t(VOLUME_DATABLOCK_SIZE), &volData[0], size_t(volData.length()));
    ret = m_cinderHandler->ReadBlocks(offset, size, m_buffer, m_calcHashBuffer, m_readHashBuffer);
    EXPECT_EQ(ret, SUCCESS);
    m_cinderHandler->m_metaRepoHandler->Close();
    m_stub.reset(ADDR(DiskDeviceFile, Read));
}


/*
 * 测试用例： cinder 更新文件
 * 前置条件:  更新卷信息成功
 * CHECK点: ReadBlocks返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, UpdateNewVolumeInfoToFileSucc)
{
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::FULL_BACKUP);
    auto jobHandler = FormJobHandler(m_job, m_repo);
    m_cinderHandler = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    int ret = m_cinderHandler->InitializeVolumeInfo("OpenStackConfig");
    EXPECT_EQ(ret, 0);
    memcpy_s(m_buffer.get(), size_t(VOLUME_DATABLOCK_SIZE), &volData[0], size_t(volData.length()));
    Volume newCreatedVolume;
    newCreatedVolume.m_id = "test_volume";
    newCreatedVolume.m_snapshotId = "test_snapshotId";
    ret = m_cinderHandler->UpdateNewVolumeInfoToFile(newCreatedVolume);
    EXPECT_EQ(ret, true);
    VolSnapInfo snapshot;
    snapshot.m_snapshotId = "test_snapshotId";
    ret = m_cinderHandler->LoadCreatedVolumeInfoFromeFile(snapshot, newCreatedVolume);
    EXPECT_EQ(ret, true);
}

/*
 * 测试用例： 等待卸载中的卷状态变成可用失败
 * 前置条件:  卷在卸载中
 * CHECK点:  返回FALSE
 */
/* TEST_F(CinderVolumeHandlerTest, WaitVolumeStatusFail)
{
    Volume volDetail;
    volDetail.m_status = "detaching";
    std::string status;
    int ret = m_cinderHandler->WaitVolumeStatus(volDetail, status);
    EXPECT_EQ(ret, false);
} */

static int32_t StubScanDisk(const std::string &volumeId, int32_t retryTimes)
{
    static int enterCount = 0;
    if (enterCount == 0) {
        enterCount += 1;
        return FAILED;
    } else {
        return SUCCESS;
    }
}

static bool StubChangeFilePriviledge(const std::string &file, const VolOpenMode &mode)
{
    return true;
}
static bool StubCheckAndDetachVolume(const std::string &volId)
{
    return true;
}
 

static int32_t StubDoAttachVolumeToHost(const std::string &volumeId)
{
    return SUCCESS;
}

static std::shared_ptr<VolumeResponse> StubGetVolumeDetail(VolumeRequest &request)
{
    static int count = 0;
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    if (count == 0) { // Get volume detail
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    } else if (count == 1) { // get if volume detached
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "available");
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(volResp);
        count ++;
    } else if (count >= 2) { // get if volume attached
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        response->SetGetBody(getVolumeResponse);
        count ++;
    }
    response->Serial();
    return response;
}

static std::shared_ptr<ServerResponse> StubDetachServerVolume(DetachVolumeRequest &request,
    const std::string& volumeId)
{
    std::shared_ptr<ServerResponse> response = std::make_shared<ServerResponse>();
    response->SetStatusCode(static_cast<uint32_t>(Module::SC_ACCEPTED));
    return response;
}

static std::shared_ptr<AttachServerVolumeResponse> StubAttachServerVolume(AttachServerVolumeRequest &request)
{
    std::shared_ptr<AttachServerVolumeResponse> response = std::make_shared<AttachServerVolumeResponse>();
    response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
    return response;
}

/*
 * 测试用例： 等待卸载中的卷状态变成可用失败
 * 前置条件:  卷在卸载中
 * CHECK点:  返回FALSE
 */
TEST_F(CinderVolumeHandlerTest, WaitVolumeStatusFailV2)
{
    VolSnapInfo snapshot;
    snapshot.m_snapshotId = "test_snapshotId";
    int ret = m_cinderHandler->DoCreateVolumeFromSnapShotId(snapshot);
    EXPECT_EQ(ret, SUCCESS);
    Volume volDetail;
    volDetail.m_status = "detaching";
    std::string status;
    m_stub.set(ADDR(CinderClient, GetVolumeDetail), Stub_GetVolumeInDetachingStatus);
    ret = m_cinderHandler->WaitVolumeStatus(volDetail, status);
    EXPECT_EQ(ret, false);
    m_stub.reset(ADDR(CinderClient, GetVolumeDetail));
}




/*
 * 测试用例： cinder尝试打开成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, TEST_Open_OK)
{
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
    auto jobHandler = FormJobHandler(m_job, m_repo);
    std::shared_ptr<CinderVolumeHandler> cinderVolClient = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    m_stub.set(ADDR(CinderVolumeHandler, DoScanDisk), StubScanDisk);
    m_stub.set(ADDR(CinderClient, GetVolumeDetail), StubGetVolumeDetail);
    m_stub.set(ADDR(NovaClient, DetachServerVolume), StubDetachServerVolume);
    m_stub.set(ADDR(NovaClient, AttachServerVolume), StubAttachServerVolume);
    m_stub.set(ADDR(DiskDeviceFile, Open), Stub_OpenVolume);
    m_stub.set(ADDR(CinderVolumeHandler, ChangeFilePriviledge), StubChangeFilePriviledge);
    int32_t ret = cinderVolClient->Open(VirtPlugin::VolOpenMode::READ_WRITE);
    EXPECT_EQ(ret, 0);
    m_stub.reset(ADDR(CinderVolumeHandler, DoScanDisk));
    m_stub.reset(ADDR(NovaClient, DetachServerVolume));
    m_stub.reset(ADDR(NovaClient, AttachServerVolume));
    m_stub.reset(ADDR(DiskDeviceFile, Open));
    m_stub.reset(ADDR(CinderVolumeHandler, DoAttachVolumeToHost));
}

static std::shared_ptr<VolumeResponse> StubGetVolumeDetail_Wait_Volume_ExpandTime_OK(VolumeRequest &request)
{
    std::shared_ptr<VolumeResponse> response = std::make_shared<VolumeResponse>();
    static int count = 0;
    if (count <= 15) { // Get volume attaching
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        volResp.replace(volResp.find("in-use"), 6, "attaching");
        response->SetGetBody(volResp);
        count ++;
    } else { // Get volume in use
        response->SetStatusCode(static_cast<uint32_t>(Module::SC_OK));
        std::string volResp = getVolumeResponse;
        response->SetGetBody(volResp);
        count ++;
    }
    response->Serial();
    return response;
}

/*
 * 测试用例： cinder尝试打开成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, TEST_Wait_Volume_ExpandTime_OK)
{
    m_job.jobParam.__set_backupType(AppProtect::BackupJobType::INCREMENT_BACKUP);
    auto jobHandler = FormJobHandler(m_job, m_repo);
    std::shared_ptr<CinderVolumeHandler> cinderVolClient = std::make_shared<CinderVolumeHandler>(jobHandler, m_volObj, "123", "");
    m_stub.set(ADDR(CinderClient, GetVolumeDetail), StubGetVolumeDetail_Wait_Volume_ExpandTime_OK);
    std::vector<std::string> intermediateState = {"reserved", "attaching"};
    std::string volumeId = "1-1-1-1";
    int32_t ret = cinderVolClient->DoWaitVolumeStatus(volumeId, "in-use", intermediateState);
    EXPECT_EQ(ret, 0);
    m_stub.reset(ADDR(CinderClient, GetVolumeDetail));
}

/*
 * 测试用例： cinder尝试打开成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, TEST_Open_OKV2)
{
    m_stub.set(ADDR(DiskDeviceFile, Open), Stub_OpenVolume);
    m_stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Scan_Success);
    m_stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const std::unordered_set<std::string>&, const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Change_Success);
    m_stub.set(ADDR(CinderClient, GetVolumeDetail), Stub_SendRequestToGetVolumeStatusSuccess);
    m_stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderTen);
    m_stub.set(ADDR(NovaClient, DetachServerVolume), Stub_SendRequestToDetachServerVolumeSuccess);
    m_stub.set(ADDR(NovaClient, AttachServerVolume), Stub_SendRequestToAttachServerVolumeSuccess);
    g_getStatusCount = 0;
    int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(DiskDeviceFile, Open));
    m_stub.reset((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand));
    m_stub.reset((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const std::unordered_set<std::string>&, const Module::CmdRunUser&))(Module::RunCommand));
    m_stub.reset(ADDR(CinderClient, GetVolumeDetail));
    m_stub.reset(ADDR(Module::ConfigReader, getInt));
    m_stub.reset(ADDR(NovaClient, DetachServerVolume));
    m_stub.reset(ADDR(NovaClient, AttachServerVolume));
}

/*
 * 测试用例： cinder尝试获取DirtyRanges成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, GetDirtyRanges_SUCCESS)
{
    VolSnapInfo preVolSnapshot;
    VolSnapInfo curVolSnapshot;
    DirtyRanges dirtyRanges;
    uint64_t startOffset = 0;
    uint64_t endOffset = 10;
    int ret = m_cinderHandler->GetDirtyRanges(preVolSnapshot, curVolSnapshot, dirtyRanges, startOffset, endOffset);
    EXPECT_EQ(ret, SUCCESS);
}


/*
 * 测试用例： cinder尝试通过快照打开成功后又关闭成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
// TEST_F(CinderVolumeHandlerTest, TEST_Close_OK)
// {
//     m_stub.set(ADDR(BaseTokenMgr, GetToken), Stub_Http_GetToken_Success);
//     m_stub.set(ADDR(HttpClient, Send), Stub_OpenfromSnapSucc);
//     m_stub.set(ADDR(DiskDeviceFile, Open), Stub_OpenVolume);
//     m_stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Snap_OK);
//     BackupSubJobInfo jobinfo;
//     VolSnapInfo curSnapshotInfo;
//     curSnapshotInfo.m_volUuid = "c1f1465b-6a63-4343-9761-7ad3e413f4ef";
//     curSnapshotInfo.m_createTime = "2022-07-21T07:00:21.862040";
//     curSnapshotInfo.m_size = 10;
//     curSnapshotInfo.m_snapshotName = "VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3";
//     curSnapshotInfo.m_status = "available";
//     curSnapshotInfo.m_snapshotId = "3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967";
//     curSnapshotInfo.m_extendInfo = g_volumeSnapDetailStr;
//     jobinfo.m_curSnapshotInfo = curSnapshotInfo;
//     int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE, jobinfo);
//     EXPECT_EQ(ret, 0);
//     ret = m_cinderHandler->Close();
//     EXPECT_EQ(ret, 0);
//     m_stub.reset(ADDR(BaseTokenMgr, GetToken));
//     m_stub.reset(ADDR(HttpClient, Send));
//     m_stub.reset(ADDR(DiskDeviceFile, Open));
//     m_stub.reset(Module::BaseRunShellCmdWithOutputWithOutLock);
// }

/*
 * 测试用例： cinder尝试通过快照打开成功后又关闭成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, TEST_Close_OKV2)
{
    // m_stub.set(ADDR(BaseTokenMgr, GetToken), Stub_Http_GetToken_Success);
    // m_stub.set(ADDR(HttpClient, Send), Stub_OpenfromSnapSucc);
    m_stub.set(ADDR(CinderClient, GetVolumeDetail), Stub_SendRequestToGetVolumeStatusSuccess);
    m_stub.set(ADDR(CinderClient, CreateVolume), Stub_SendRequestToCreateVolume);
    m_stub.set(ADDR(DiskDeviceFile, Open), Stub_OpenVolume);
    m_stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Snap_Success);
    m_stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const std::unordered_set<std::string>&, const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Change_Success);
    m_stub.set(ADDR(NovaClient, DetachServerVolume), Stub_SendRequestToDetachServerVolumeSuccess);
    m_stub.set(ADDR(NovaClient, AttachServerVolume), Stub_SendRequestToAttachServerVolumeSuccess);
    m_stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderTen);

    g_getStatusCount = 0;
    BackupSubJobInfo jobinfo;
    VolSnapInfo curSnapshotInfo;
    curSnapshotInfo.m_volUuid = "c1f1465b-6a63-4343-9761-7ad3e413f4ef";
    curSnapshotInfo.m_createTime = "2022-07-21T07:00:21.862040";
    curSnapshotInfo.m_size = 10;
    curSnapshotInfo.m_snapshotName = "VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3";
    curSnapshotInfo.m_status = "available";
    curSnapshotInfo.m_snapshotId = "3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967";
    curSnapshotInfo.m_extendInfo = g_volumeSnapDetailStr;
    jobinfo.m_curSnapshotInfo = curSnapshotInfo;
    int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE, jobinfo);
    EXPECT_EQ(ret, 0);
    ret = m_cinderHandler->Close();
    EXPECT_EQ(ret, 0);
    // m_stub.reset(ADDR(BaseTokenMgr, GetToken));
    // m_stub.reset(ADDR(HttpClient, Send));
    m_stub.reset(ADDR(CinderClient, GetVolumeDetail));
    m_stub.reset(ADDR(CinderClient, CreateVolume));
    m_stub.reset(ADDR(DiskDeviceFile, Open));
    m_stub.reset((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand));
    m_stub.reset((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const std::unordered_set<std::string>&, const Module::CmdRunUser&))(Module::RunCommand));
    m_stub.reset(ADDR(Module::ConfigReader, getInt));
    m_stub.reset(ADDR(NovaClient, DetachServerVolume));
    m_stub.reset(ADDR(NovaClient, AttachServerVolume));
}


/*
 * 测试用例： cinder尝试时第一次扫盘时即成功
 * 前置条件:  无
 * CHECK点: OPEN返回SUCCESS
 */
// TEST_F(CinderVolumeHandlerTest, TEST_Open_scandisk_OK)
// {
//     m_stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Snap_OK);
//     m_stub.set(ADDR(DiskDeviceFile, Open), Stub_OpenVolume);
//     int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE);
//     EXPECT_EQ(ret, 0);
//     m_stub.reset(ADDR(DiskDeviceFile, Open));
//     m_stub.reset(Module::BaseRunShellCmdWithOutputWithOutLock);
// }


// /*
//  * 测试用例： 由于磁盘设备打开失败导致cinder尝试打开失败
//  * 前置条件:  无
//  * CHECK点: OPEN返回Failed
//  */
// TEST_F(CinderVolumeHandlerTest, TEST_Open_diskdevice_Failed)
// {
//     m_stub.set(ADDR(HttpClient, Send), Stub_OpenSucc);
//     m_stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Scan_OK);
//     int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE);
//     EXPECT_EQ(ret, FAILED);
//     m_stub.reset(ADDR(HttpClient, Send));
//     m_stub.reset(Module::BaseRunShellCmdWithOutputWithOutLock);
// }

/*
 * 测试用例： cinder尝试通过快照打开失败
 * 前置条件:  无
 * CHECK点: OPEN返回Failed
 */
// TEST_F(CinderVolumeHandlerTest, TEST_OpenFromSnapshot_Fail)
// {
//     BackupJob job;
//     BackupSubJobInfo jobinfo;
//     VolSnapInfo curSnapshotInfo;
//     curSnapshotInfo.m_volUuid = "c1f1465b-6a63-4343-9761-7ad3e413f4ef";
//     curSnapshotInfo.m_createTime = "2022-07-21T07:00:21.862040";
//     curSnapshotInfo.m_size = 10;
//     curSnapshotInfo.m_snapshotName = "VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3";
//     curSnapshotInfo.m_status = "available";
//     curSnapshotInfo.m_snapshotId = "3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967";
//     curSnapshotInfo.m_extendInfo = g_volumeSnapDetailStr;
//     jobinfo.m_curSnapshotInfo = curSnapshotInfo;
//     m_stub.set(ADDR(BaseTokenMgr, GetToken), Stub_Http_GetToken_Success);
//     m_stub.set(ADDR(HttpClient, Send), Stub_OpenFailed);
//     int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE, jobinfo);
//     EXPECT_EQ(ret, FAILED);
//     m_stub.reset(ADDR(BaseTokenMgr, GetToken));
//     m_stub.reset(ADDR(HttpClient, Send));
// }

/*
 * 测试用例： cinder尝试通过快照打开失败
 * 前置条件:  无
 * CHECK点: OPEN返回Failed
 */
TEST_F(CinderVolumeHandlerTest, TEST_OpenFromSnapshot_FailV2)
{
    m_stub.set(ADDR(CinderClient, GetVolumeDetail), Stub_SendRequestToGetVolumeStatusFail);
    m_stub.set(ADDR(CinderClient, CreateVolume), Stub_SendRequestToCreateVolume);
    m_stub.set(ADDR(DiskDeviceFile, Open), Stub_OpenVolume);
    m_stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Snap_OK);
    m_stub.set(ADDR(NovaClient, DetachServerVolume), Stub_SendRequestToDetachServerVolumeSuccess);
    m_stub.set(ADDR(NovaClient, AttachServerVolume), Stub_SendRequestToAttachServerVolumeSuccess);
    m_stub.set(ADDR(Module::ConfigReader, getInt), Stub_OpenStackConfigReaderTen);

    g_getStatusCount = 0;
    
    BackupJob job;
    BackupSubJobInfo jobinfo;
    VolSnapInfo curSnapshotInfo;
    curSnapshotInfo.m_volUuid = "c1f1465b-6a63-4343-9761-7ad3e413f4ef";
    curSnapshotInfo.m_createTime = "2022-07-21T07:00:21.862040";
    curSnapshotInfo.m_size = 10;
    curSnapshotInfo.m_snapshotName = "VirtPlugin_ecs-4d45-0001-volume-0000_26B49564-E4CE-4330-AAD6-D2C42AF96CD3";
    curSnapshotInfo.m_status = "available";
    curSnapshotInfo.m_snapshotId = "3dd2c7fd-adbe-a1a1-5a36-0e2e7695c967";
    curSnapshotInfo.m_extendInfo = g_volumeSnapDetailStr;
    jobinfo.m_curSnapshotInfo = curSnapshotInfo;
    // m_stub.set(ADDR(BaseTokenMgr, GetToken), Stub_Http_GetToken_Success);
    // m_stub.set(ADDR(HttpClient, Send), Stub_OpenFailed);
    int ret = m_cinderHandler->Open(VirtPlugin::VolOpenMode::READ_WRITE, jobinfo);
    EXPECT_EQ(ret, FAILED);
    // m_stub.reset(ADDR(BaseTokenMgr, GetToken));
    // m_stub.reset(ADDR(HttpClient, Send));

    m_stub.reset(ADDR(CinderClient, GetVolumeDetail));
    m_stub.reset(ADDR(CinderClient, CreateVolume));
    m_stub.reset(ADDR(DiskDeviceFile, Open));
    m_stub.reset(Module::BaseRunShellCmdWithOutputWithOutLock);
    m_stub.reset(ADDR(Module::ConfigReader, getInt));
    m_stub.reset(ADDR(NovaClient, DetachServerVolume));
    m_stub.reset(ADDR(NovaClient, AttachServerVolume));
}

/*
 * 测试用例： cinder获取卷大小
 * 前置条件:  无
 * CHECK点: 获取卷大小正确
 */
TEST_F(CinderVolumeHandlerTest, TEST_GetVolumeSize)
{
    uint64_t volumeSize = m_cinderHandler->GetVolumeSize();
    EXPECT_EQ(volumeSize, VOLUME_DATABLOCK_SIZE);
}

/*
 * 测试用例： cinder刷回设备
 * 前置条件:  无
 * CHECK点: 正确获取刷回数据状态
 */
TEST_F(CinderVolumeHandlerTest, TEST_Flush)
{
    m_stub.set(ADDR(DiskDeviceFile, Flush), Stub_FlushSuccess);
    int ret = m_cinderHandler->Flush();
    EXPECT_EQ(ret, SUCCESS);

    m_stub.set(ADDR(DiskDeviceFile, Flush), Stub_FlushFail);
    ret = m_cinderHandler->Flush();
    EXPECT_EQ(ret, FAILED);

    m_stub.reset(ADDR(DiskDeviceFile, Flush));
}

/*
 * 测试用例： 测试设备连接
 * 前置条件:  无
 * CHECK点: 返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, TEST_TestDeviceConnection)
{
    string authExtendInfo = "test info";
    int32_t error;
    int ret = m_cinderHandler->TestDeviceConnection(authExtendInfo, error);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * 测试用例： 清理残留
 * 前置条件:  无
 * CHECK点: 返回SUCCESS
 */
TEST_F(CinderVolumeHandlerTest, TEST_CleanLeftovers)
{
    int ret = m_cinderHandler->CleanLeftovers();
    EXPECT_EQ(ret, SUCCESS);
}

}
