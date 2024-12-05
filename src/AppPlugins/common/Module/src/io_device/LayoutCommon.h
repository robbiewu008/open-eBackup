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
#ifndef LAYOUT_COMMON_H
#define LAYOUT_COMMON_H

#include <memory>
#include <vector>
#include <functional>
#include <unordered_set>
#include "common/JsonHelper.h"

#include "json/version.h"
#include "log/Log.h"

namespace Module {
enum class LayoutRetCode {
    SUCCESS = 0,
    PACK_AGAIN,
    PACK_DONE,
    FAILED,
    REMOTE_MEDIA_NOT_REACHABLE,
    REMOTE_MEDIA_SPACE_NOT_ENOUGH,
    LOCAL_FS_NOT_REACHABLE,
    TARGET_BACKUPSET_NOT_EXIST,
    TARGET_IMAGE_NOT_EXIST,
    TARGET_DIR_NOT_EXIST,
    TARGET_FILE_NOT_EXIST,
    IMAGESET_INFOFILE_FORMAT_INCORRECT,
    IMAGE_INFOFILE_FORMAT_INCORRECT,
    DELLIST_FILE_FORMAT_INCORRECT,
    AGGREGATE_FAILED,
    GET_METADATA_FAILED,
    GET_FILELIST_FAILED
};

enum class LayoutBackupType {
    FULL = 0,
    INC
};

struct MetaInfo {
    uint64_t offset {0};
    uint64_t length {0};
    std::string metaFile;
};

struct IndexNode {
    std::string fileName;     /* name of file */
    std::string aggTarget;    /* empty if 'aggregated' is false, or 'agg_{guid}.{packFormat}' */
    MetaInfo metaInfo;
};

struct BinaryData {
    uint64_t length {0};                   // content length
    std::shared_ptr<uint8_t []> data;          // package binary content pointer
};

/* Image Info File */
struct ImageInfo {
    ImageInfo() {}
    ImageInfo(const ImageInfo &imageInfo)
    {
        jsonVersion = imageInfo.jsonVersion;
        imageIndex = imageInfo.imageIndex;
        imageId = imageInfo.imageId;
        backupType = imageInfo.backupType;
        creationDatetime = imageInfo.creationDatetime;
        imageState = imageInfo.imageState;
        resourceId = imageInfo.resourceId;
        description = imageInfo.description;
        dependencies = imageInfo.dependencies;
        imageFileSize = imageInfo.imageFileSize;
        imageFileCount = imageInfo.imageFileCount;
        imageDirCount = imageInfo.imageDirCount;
        imageDeletedFileCount = imageInfo.imageDeletedFileCount;
    }
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(jsonVersion)
    SERIAL_MEMEBER(imageIndex)
    SERIAL_MEMEBER(imageId)
    SERIAL_MEMEBER(backupType)
    SERIAL_MEMEBER(creationDatetime)
    SERIAL_MEMEBER(imageState)
    SERIAL_MEMEBER(resourceId)
    SERIAL_MEMEBER(description)
    SERIAL_MEMEBER(dependencies)
    SERIAL_MEMEBER(imageFileSize)
    SERIAL_MEMEBER(imageFileCount)
    SERIAL_MEMEBER(imageDirCount)
    SERIAL_MEMEBER(imageDeletedFileCount)
    END_SERIAL_MEMEBER

    std::string jsonVersion;                    /* image info json version */
    int64_t imageIndex {0};                     /* image index, 1~N */
    std::string imageId;                        /* image id */
    int32_t backupType {0};                     /* backup type, FULL/INC */
    std::string creationDatetime;               /* image creation date */
    int32_t imageState {0};                     /* image state*/
    std::string resourceId;                     /* resource id */
    std::string description;                    /* description of this image */
    std::vector<std::string> dependencies;      /* list of recent FULL image to current(include) */
    uint64_t imageFileSize {0};                 /* image size */
    uint64_t imageFileCount {0};                /* image file count */
    uint64_t imageDirCount {0};                 /* image directory count */
    uint64_t imageDeletedFileCount {0};         /* deleted file count */
};

struct AggregationInfo {
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(aggregated)
    SERIAL_MEMEBER(packFormat)
    END_SERIAL_MEMEBER
    bool aggregated { false };        /* aggregate enabled or not in the imageset */
    int packFormat {0};               /* pack format */
};

struct ImageSetInfo {
    ImageSetInfo() {}
    ImageSetInfo(const ImageSetInfo& setInfo)
    {
        jsonVersion = setInfo.jsonVersion;
        imageSetId = setInfo.imageSetId;
        creationDatetime = setInfo.creationDatetime;
        aggregateInfo.aggregated = setInfo.aggregateInfo.aggregated;
        aggregateInfo.packFormat = setInfo.aggregateInfo.packFormat;
        imageCount = setInfo.imageCount;
        newestIndex = setInfo.newestIndex;
    }
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(imageSetId)
    SERIAL_MEMEBER(creationDatetime)
    SERIAL_MEMEBER(aggregateInfo)
    SERIAL_MEMEBER(jsonVersion)
    SERIAL_MEMEBER(imageCount)
    SERIAL_MEMEBER(newestIndex)
    END_SERIAL_MEMEBER
    std::string imageSetId;             /* imageset id */
    std::string creationDatetime;       /* imageset creation date */
    AggregationInfo aggregateInfo;      /* aggregate info */
    std::string jsonVersion;            /* imageset info json version */
    uint32_t imageCount {0};
    uint32_t newestIndex {0};           // newest index of image in imageSet
};

using ChangeListMap = std::map<std::string, MetaInfo>;
using ReadFileCallback = std::function<void(LayoutRetCode, uint64_t, bool &)>;
using WriteFileCallback = void (*)(LayoutRetCode status, uint64_t writtenSize, uint64_t realWrittenSize,
    void *callbackData);
using DeleteFileCallback = void (*)(LayoutRetCode status, uint64_t deletedFileCount, void *callbackData);

struct CallBackHandle {
    void *callBackFunc {nullptr};
    void *callBackData {nullptr};
};
} // namespace Module
#endif