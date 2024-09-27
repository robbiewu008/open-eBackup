/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file afs_error.h
 * @brief AFS (- Error code definition
 *
 */

#ifndef INCLUDE_AFS_ERROR_H_
#define INCLUDE_AFS_ERROR_H_

#define AFS_SUCCESS 0               // 处理成功
#define AFS_ERR_INNER (-1)            // 程序内部错误
#define AFS_ERROE (-2)               // 处理失败
#define AFS_ERR_IMAGE_READ (-101)     // 调用回调函数失败
#define AFS_ERR_API (-102)            // 调用API错误
#define AFS_VOLUME_GROUP_EXIST (-103) // LVM 卷组已经存在
#define AFS_PV_NO_VG_METADAT (-104)   // pv 不存在 VG metadata

// 内存失败
// API错误

#define AFS_ERR_OFFSET 100

#define AFS_ERR_OFFSET_PARA (2 * AFS_ERR_OFFSET)

// 参数错误
#define AFS_ERR_PARAMETER (-(AFS_ERR_OFFSET_PARA + 1))          // 参数错误(-201)
#define AFS_ERR_PARA_BUFFER_SIZE (-(AFS_ERR_OFFSET_PARA + 2))   // Buffer参数(-202)
#define AFS_ERR_PARA_BIT_SIZE (-(AFS_ERR_OFFSET_PARA + 3))      // Bitmap位大小(-203)
#define AFS_ERR_PART_INDEX_INVALID (-(AFS_ERR_OFFSET_PARA + 4)) // 指定的分区号错误(-204)
#define AFS_ERR_PARA_PATH (-(AFS_ERR_OFFSET_PARA + 5))          // 指定Path为空(-205)
#define AFS_ERR_NOT_EXIST_PATH (-(AFS_ERR_OFFSET_PARA + 6))     // 指定Path不存在(-206)

// 分区错误
#define AFS_ERR_PART_OFFSET (3 * AFS_ERR_OFFSET)

#define AFS_ERR_PARTITION (-(AFS_ERR_PART_OFFSET + 1))   // 分区错误(-301)
#define AFS_ERR_LVM_PART (-(AFS_ERR_PART_OFFSET + 2))    // LVM分析错误(-302)
#define AFS_ERR_LVM_VERSION (-(AFS_ERR_PART_OFFSET + 3)) // LVM版本不支持(-303)

// 文件系统分析错误
#define AFS_ERR_FS (4 * AFS_ERR_OFFSET)
#define AFS_ERR_FS_VERSION (-(AFS_ERR_FS + 1)) // 不支持的文件系统版本(-401)
#define AFS_ERR_FS_SUPPORT (-(AFS_ERR_FS + 2)) // 无法识别分区类型(-402)
#define AFS_ERR_FS_ANALYZE (-(AFS_ERR_FS + 3)) // 未知原因分析失败

// 文件过滤错误
#define AFS_ERR_FILTER (5 * AFS_ERR_OFFSET)
#define AFS_ERR_FILE_SYMLINKS (-(AFS_ERR_FILTER + 1)) // 不能支持的符号链接(-501)
#define AFS_ERR_FILE_TYPE (-(AFS_ERR_FILTER + 2))     // 不能支持的文件类型(-502)

#endif /* INCLUDE_AFS_ERROR_H_ */
