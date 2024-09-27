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
#include "interface/CloudServiceResponse.h"
#include "HWInnerDef.h"
#include "HWSDKCallback.h"

using namespace Module;

namespace {
const std::string MODULE_NAME = "HWSDKCallback";
}  // namespace



namespace Module {

void PrintCompleteCallbackErrorDesc(obs_status status, const obs_error_details *error)
{
    if (status == OBS_STATUS_OK) {
        return;
    }
    HCP_Log(ERR, MODULE_NAME) << "call error, code - " << obs_get_status_name(status) << HCPENDLOG;
    if (error == nullptr) {
        return;
    }
    if (error->message == nullptr) {
        return;
    }
    HCP_Log(ERR, MODULE_NAME) << "error detail desc - " << error->message << HCPENDLOG;
}

obs_status HWCloudServiceS3Callback(const char *ownerId, const char *owner_display_name, const char *bucketName,
    int64_t creationDate, void *callbackData)
{
    listServiceData *data = (listServiceData *)callbackData;
    if (data->bucketListSize >= MAX_BUCKET_NUM) {
        HCP_Log(WARN, MODULE_NAME) << "bucketListSize too big." << HCPENDLOG;
        return OBS_STATUS_OK;
    }
    data->bucketList.push_back(bucketName);
    data->ownerId = ownerId;
    data->bucketListSize++;

    return OBS_STATUS_OK;
}

obs_status HWCloudServiceCallback(
    const char *ownerId, const char *bucketName, int64_t creationDate, const char *location, void *callbackData)
{
    listServiceData *data = (listServiceData *)callbackData;
    if (data->bucketListSize >= MAX_BUCKET_NUM) {
        HCP_Log(WARN, MODULE_NAME) << "bucketListSize too big." << HCPENDLOG;
        return OBS_STATUS_OK;
    }
    data->bucketList.push_back(bucketName);
    data->ownerId = ownerId;
    data->bucketListSize++;

    return OBS_STATUS_OK;
}

void HWCloudServiceCompleteCallback(obs_status status, const obs_error_details *error, void *callbackData)
{
    listServiceData *data = (listServiceData *)callbackData;
    data->ret_status = status;

    PrintCompleteCallbackErrorDesc(status, error);
}

obs_status CommonPropertiesCallback(const obs_response_properties* properties, void* callback_data)
{
    return OBS_STATUS_OK;
}

void ListObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callback_data)
{
    ListObjectCallBackData* data = (ListObjectCallBackData*) callback_data;
    data->retStatus = status;

    PrintCompleteCallbackErrorDesc(status, error);
}

obs_status ListObjectsCallback(int is_truncated, const char* next_marker, int contents_count,
    const obs_list_objects_content* contents, int common_prefixes_count, const char** common_prefixes,
    void* callback_data)
{
    ListObjectCallBackData* data = (ListObjectCallBackData*) callback_data;

    data->isTruncated = is_truncated > 0;
    if ((!next_marker || !next_marker[0]) && contents_count) {
        next_marker = contents[contents_count - 1].key;
    }
    if (next_marker) {
        data->nextMarker = next_marker;
    } else {
        data->nextMarker = "";
    }

    for (int i = 0; i < contents_count; ++i) {
        const auto& CStyleContent = contents[i];
        ListObjectsContent content;
        content.key = ConvertCStr2Str(CStyleContent.key);
        content.lastModified = CStyleContent.last_modified;
        content.etag = ConvertCStr2Str(CStyleContent.etag);
        content.size = CStyleContent.size;
        content.ownerId = ConvertCStr2Str(CStyleContent.owner_id);
        content.ownerDisplayName = ConvertCStr2Str(CStyleContent.owner_display_name);
        content.storageClass = ConvertCStr2Str(CStyleContent.storage_class);
        content.type = ConvertCStr2Str(CStyleContent.type);
        data->contents.emplace_back(content);
    }
    for (int i = 0; i < common_prefixes_count; ++i) {
        data->commonPrefixes.emplace_back(common_prefixes[i]);
    }

    return OBS_STATUS_OK;
}

obs_status GetObjectMetaDataPropertiesCallback(const obs_response_properties* properties, void* callback_data)
{
    if (properties == NULL) {
        HCP_Log(INFO, MODULE_NAME) << "properties is null" << HCPENDLOG;
        return OBS_STATUS_OK;
    }

    GetObjectMetaDataCallBackData* data = (GetObjectMetaDataCallBackData*) callback_data;
    data->lastModified = properties->last_modified;
    data->etag = ConvertCStr2Str(properties->etag);
    data->size = properties->content_length;
    data->AddSysDefMetaData("ContentType", properties->content_type);
    data->AddSysDefMetaData("WebsiteRedirectLocation", properties->website_redirect_location);
    data->AddSysDefMetaData("Expires", properties->expiration);

    for (int i = 0; i < properties->meta_data_count; ++i) {
        data->AddUserDefMetaData(properties->meta_data[i].name, properties->meta_data[i].value);
    }

    return OBS_STATUS_OK;
}

void GetObjectMetaDataCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        GetObjectMetaDataCallBackData* data = (GetObjectMetaDataCallBackData*) callbackData;
        data->retStatus = status;
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

void GetObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        GetObjectCallBackData* data = static_cast<GetObjectCallBackData*>(callbackData);
        data->retStatus = status;
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

obs_status GetObjectDataCallback(int buffer_size, const char* buffer, void* callbackData)
{
    if (buffer_size < 0) {
        HCP_Log(ERR, MODULE_NAME) << "Buffer_size is smaller than 0" << HCPENDLOG;
        return OBS_STATUS_InternalError;
    }
    if (callbackData == nullptr) {
        HCP_Log(ERR, MODULE_NAME) << "CallbackData is nullptr" << HCPENDLOG;
        return OBS_STATUS_InternalError;
    }

    GetObjectCallBackData* data = static_cast<GetObjectCallBackData*>(callbackData);
    errno_t ret = memcpy_s(data->buffer + data->doneSize, data->bufferSize - data->doneSize, buffer, buffer_size);
    if (ret != 0) {
        HCP_Log(ERR, MODULE_NAME) << "Copy failed, done size " << data->doneSize
            << " total buffer size " << data->bufferSize
            << " cur buffer size " << buffer_size
            << " memcpy rest " << ret
            << HCPENDLOG;
        return OBS_STATUS_InternalError;
    }
    data->doneSize += buffer_size;

    return OBS_STATUS_OK;
}

void MultiPartDownloadObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        MultiPartDownloadObjectCallBackData* data = (MultiPartDownloadObjectCallBackData*) callbackData;
        data->retStatus = status;
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

void MultiPartDownloadObjectCallback(obs_status status, char* resultMsg, int partCountReturn,
    obs_download_file_part_info* downloadInfoList, void* callbackData)
{
    MultiPartDownloadObjectCallBackData* data = (MultiPartDownloadObjectCallBackData*) callbackData;

    obs_download_file_part_info* pstDownloadInfoList = downloadInfoList;
    for(int i = 0; i < partCountReturn; i++) {
        DownloadObjectPartInfo downloadObjectPartInfo;
        downloadObjectPartInfo.partId = pstDownloadInfoList[i].part_num;
        downloadObjectPartInfo.startByte = pstDownloadInfoList[i].start_byte;
        downloadObjectPartInfo.partSize = pstDownloadInfoList[i].part_size;
        downloadObjectPartInfo.status = pstDownloadInfoList[i].status_return;
        data->downloadObjectPartInfo.emplace_back(downloadObjectPartInfo);
    }
}

void ResponseCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        obs_status* retStatus = (obs_status*)callbackData;
        *retStatus = status;
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

void HeadBucketCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        HeadBucketCallBackData* data = (HeadBucketCallBackData*) callbackData;
        data->retStatus = status;
    }
}

obs_status MultiPartUploadObjectPropertiesCallback(const obs_response_properties *properties, void *callback_data)
{
    (void) callback_data;
    return OBS_STATUS_OK;
}

void MultiPartUploadObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        MultiPartUploadObjectCallBackData* data = (MultiPartUploadObjectCallBackData*) callbackData;
        data->retStatus = status;
        if (data->callBack) {
            data->partCount++;
            data->callBack(data->partSize, data->partCount, data->callBackData);
        }
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

void MultiPartUploadObjectCallback(obs_status status, char* resultMsg, int partCountReturn,
    obs_upload_file_part_info* uploadInfoList, void* callbackData)
{
    if (uploadInfoList == NULL || callbackData == NULL) {
        HCP_Log(WARN, MODULE_NAME) << "upload object uploadInfoList or callbackData is null" << HCPENDLOG;
        return;
    }

    MultiPartUploadObjectCallBackData* data = (MultiPartUploadObjectCallBackData*) callbackData;

    for(int i = 0; i < partCountReturn; i++) {
        UploadObjectPartInfo tmpUploadObjectInfo;
        tmpUploadObjectInfo.partId = uploadInfoList[i].part_num;
        tmpUploadObjectInfo.startByte = uploadInfoList[i].start_byte;
        tmpUploadObjectInfo.partSize = uploadInfoList[i].part_size;
        tmpUploadObjectInfo.status = uploadInfoList[i].status_return;
        data->uploadObjectPartInfo.emplace_back(tmpUploadObjectInfo);
    }
}

void GetUploadIdCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        GetUploadIdCallData* data = (GetUploadIdCallData*)callbackData;
        data->retStatus = status;
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

void PutPartObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData)
{
    if (callbackData) {
        PutObjectPartCallData* data = (PutObjectPartCallData*) callbackData;
        data->retStatus = status;
    }

    PrintCompleteCallbackErrorDesc(status, error);
}

obs_status PutPartObjectPropertiesCallback(const obs_response_properties *properties, void *callbackData)
{
    if (properties == NULL || callbackData == NULL) {
        HCP_Log(WARN, MODULE_NAME) << "put object buffer or callbackData is null" << HCPENDLOG;
        return OBS_STATUS_OK;;
    }

    PutObjectPartCallData *data = (PutObjectPartCallData *)callbackData;

    if (properties->etag) {
        data->etag = properties->etag;
        HCP_Log(DEBUG, MODULE_NAME) << "Etag: " << std::string(properties->etag) << HCPENDLOG;
    }

    return OBS_STATUS_OK;
}

int PutPartObjectCallback(int buffer_size, char *buffer, void *callbackData)
{
    int ret = 0;
    if (buffer == NULL || callbackData == NULL) {
        HCP_Log(WARN, MODULE_NAME) << "put object buffer or callbackData is null" << HCPENDLOG;
        return ret;
    }

    PutObjectPartCallData* data = (PutObjectPartCallData*) callbackData;
    char* tmpPtr = data->bufPtr + data->startByte;
    uint64_t toRead = ((data->partSize > static_cast<unsigned>(buffer_size)) ?
                static_cast<unsigned>(buffer_size) : data->partSize);
    if (toRead <= 0) {
        HCP_Log(WARN, MODULE_NAME) << "nothing to read" << HCPENDLOG;
        return ret;
    }
    ret = memcpy_s(buffer, toRead, tmpPtr, toRead);
    if (ret != 0) {
        HCP_Log(ERR, MODULE_NAME) << "copy failed!, ret code:" << ret
            << " data->startByte: " << data->startByte << ", buffer_size: " << buffer_size
            << " data->partSize: " << data->partSize << ", toRead: " << toRead << HCPENDLOG;
        return -1;
    }
    data->startByte += toRead;
    data->partSize -= toRead;

    return static_cast<int>(toRead);
}

obs_status CompletePutPartObjectCallback(const char *location, const char *bucket, const char *key,
    const char* eTag, void *callbackData)
{
    (void)callbackData;
    HCP_Log(DEBUG, MODULE_NAME) << "Location = " << location << ", Bucket = " << bucket << ", Key = " << key
                           << ", ETag = " << eTag << HCPENDLOG;
    return OBS_STATUS_OK;
}

}
