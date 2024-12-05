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
#ifndef OBSCTX_HW_SDK_CALLBACK_H
#define OBSCTX_HW_SDK_CALLBACK_H

#include "eSDKOBS.h"

namespace Module {
    /**
    * 列举桶回调函数
    *
    * @param ownerId 用户id
    * @param bucketName 桶名
    * @param creationDate 创建日期
    * @param location 桶所在区域
    * @param callbackData 用户自定义数据
    * @return status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    */
    obs_status HWCloudServiceCallback(
        const char *ownerId, const char *bucketName, int64_t creationDate, const char *location, void *callbackData);
    /**
    * 列举桶回调函数 - S3
    *
    * @param ownerId 用户id
    * @param owner_display_name 用户名
    * @param bucketName 桶名
    * @param creationDate 创建日期
    * @param callbackData 用户自定义数据
    * @return status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    */
    obs_status HWCloudServiceS3Callback(const char *ownerId, const char *owner_display_name, const char *bucketName,
        int64_t creationDate, void *callbackData);
    // 接口参照 ResponseCompleteCallback
    void HWCloudServiceCompleteCallback(obs_status status, const obs_error_details *error, void *callbackData);
    obs_status CommonPropertiesCallback(const obs_response_properties* properties, void* callback_data);
    /* ListObjects */
    // 接口参照 ResponseCompleteCallback
    void ListObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callback_data);
    /**
    * 列举对象回调函数
    *
    * @param is_truncated 是否截断
    * @param next_marker 列举未结束，下次列举的起始prefix
    * @param contents_count 列举对象的数量
    * @param contents 列举对象信息数组
    * @param common_prefixes_count 公共前缀数量
    * @param common_prefixes 公共前缀信息
    * @param callbackData 用户自定义数据
    * @return status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    */
    obs_status ListObjectsCallback(int is_truncated, const char* next_marker, int contents_count,
        const obs_list_objects_content* contents, int common_prefixes_count, const char** common_prefixes,
        void* callback_data);
    /* GetObjectMetaData */
    /**
    * 获取元数据属性回调函数
    *
    * @param properties 对象或桶元数据信息
    * @param callbackData 用户自定义数据
    * @return status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    */
    obs_status GetObjectMetaDataPropertiesCallback(const obs_response_properties* properties, void* callback_data);
    // 接口参照 ResponseCompleteCallback
    void GetObjectMetaDataCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData);
    // 接口参照 ResponseCompleteCallback
    void GetObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callback_data);
    /**
    * 指定分片下载数据回调函数
    *
    * @param buffer_size 下载数据大小
    * @param buffer 下载数据内容
    * @return status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    */
    obs_status GetObjectDataCallback(int buffer_size, const char* buffer, void* callbackData);
    /* MultiPartDownloadObject */
    // 接口参照 ResponseCompleteCallback
    void MultiPartDownloadObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData);
    /**
    * 分段下载回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    * @param resultMsg 下载结果整体描述
    * @param partCountReturn 分片数量
    * @param downloadInfoList 分片详细信息
    * @param callbackData 用户自定义数据
    */
    void MultiPartDownloadObjectCallback(obs_status status, char*, int partCountReturn,
        obs_download_file_part_info* downloadInfoList, void* callbackData);
    /* common */
    /**
    * 通用调用完成回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败.
    * @param error 错误详细描述
    * @param callbackData 用户自定义数据
    */
    void ResponseCompleteCallback(obs_status status, const obs_error_details *error, void* callbackData);

    /**
    * 判断桶是否存在完成回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败
    * @param error 错误详细描述
    * @param callbackData 用户自定义数据
    */
    void HeadBucketCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData);

    /**
    * 多段上传属性回调函数
    *
    * @param properties 属性
    * @param callbackData 用户自定义数据
    */
    obs_status MultiPartUploadObjectPropertiesCallback(const obs_response_properties *properties, void *callback_data);

    /**
    * 多段上传完成回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败
    * @param error 错误详细描述
    * @param callbackData 用户自定义数据
    */
    void MultiPartUploadObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData);

    /**
    * 多段上传完成回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败
    * @param resultMsg 上传结果整体描述
    * @param partCountReturn 分片数量
    * @param uploadInfoList 分片详细信息
    * @param callbackData 用户自定义数据
    */
    void MultiPartUploadObjectCallback(obs_status status, char* resultMsg, int partCountReturn,
        obs_upload_file_part_info* uploadInfoList, void* callbackData);

    /**
    * 多段上传完成回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败
    * @param error 错误详细描述
    * @param callbackData 用户自定义数据
    */
    void GetUploadIdCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData);

    /**
    * 多段上传完成回调函数
    *
    * @param status 调用返回值 = OBS_STATUS_OK 调用成功,其他调用失败
    * @param error 错误详细描述
    * @param callbackData 用户自定义数据
    */
    void PutPartObjectCompleteCallback(obs_status status, const obs_error_details* error, void* callbackData);

    /**
    * 多段上传完成回调函数
    *
    * @param properties 回调数据
    * @param callbackData 用户自定义数据
    */
    obs_status PutPartObjectPropertiesCallback(const obs_response_properties *properties, void *callbackData);

    /**
    * 多段上传回调函数
    *
    * @param buffer_size 缓存大小
    * @param buffer 缓存
    * @param callbackData 用户自定义数据
    */
    int PutPartObjectCallback(int buffer_size, char *buffer, void *callbackData);

    /**
    * 合并多段上传回调函数
    *
    * @param location 位置
    * @param bucket 桶
    * @param key 名称
    * @param eTag eTag
    * @param callbackData 用户自定义数据
    */
    obs_status CompletePutPartObjectCallback(
        const char *location, const char *bucket, const char *key, const char *eTag, void *callbackData);
}

#endif  // OBSCTX_HW_SDK_CALLBACK_H
