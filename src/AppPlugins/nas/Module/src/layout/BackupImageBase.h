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
#ifndef BACKUP_IMAGE_BASE_H
#define BACKUP_IMAGE_BASE_H

#include <memory>
#include <list>
#include <map>
#include <fstream>
#include <atomic>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/algorithm/string.hpp>
#include "BackupImageInterface.h"

namespace Module {
const std::string DATA_TYPE = "data";
const std::string AGG_LIST_FILE = "aggregation";
const std::string INDEX_PREFIX = "meta/index";
const std::string DEL_PREFIX = "meta/deleteList";
const std::string AGG_PREFIX = "meta/aggregation";
const std::string METAFILE_PREFIX = "meta/metafile";

const int LAYOUT_READ_SUCCESS = 0;
const int LAYOUT_READ_ERR = -1;
const int LAYOUT_FILE_NOT_EXIST = -2;

/*
 * BackupImageBase provides basic 1:1 layout implementation.
 */
class BackupImageBase : public BackupImageInterface {
public:
    BackupImageBase(BackupIOInterface &backupIO);
    ~BackupImageBase() = default;
    LayoutRetCode UpdateImageInfo(const ImageInfo &imageInfo) override;
    ImageInfo GetImageInfo() const override;

    void IncreaseImageWrittenSize(uint64_t size) override;
    void IncreaseImageRealWrittenSize(uint64_t size) override;
    void IncreaseImageDeletedCount(uint64_t size) override;

    /* image operation */
    bool CheckIfImageExists(const ImageInfo &imageInfo) override;
    LayoutRetCode CreateImage(const ImageInfo &imageInfo) override;
    LayoutRetCode DeleteImage(const ImageInfo &imageInfo) override;
    LayoutRetCode OpenImage(ImageInfo &imageInfo) override;
    LayoutRetCode CloseImage() override;
protected:
    static void WriteCallBack(LayoutRetCode status, uint64_t writtenSize, uint64_t realWrittenSize, void *callbackData);
    static void UploadCallBack(LayoutRetCode status, uint64_t writtenSize, uint64_t realWrittenSize, void *callbackObj);
    static void DeleteCallBack(LayoutRetCode status, uint64_t deletedSize, void *callbackData);
    bool WriteImageInfo(const ImageInfo &imageInfo, const std::string &fileName);
    std::string FormatImageName(const ImageInfo &imageInfo, const std::string &continuePart);
    void LogImageInfo(const int level, const std::string &message, const ImageInfo &imageInfo);
    std::string GetImageFileName(const ImageInfo &imageInfo, const std::string &continuePart, const std::string &file);
    int32_t GetFileNameSplitPos(const std::string localFile, int32_t availableLength);
    bool GetRemoteFileName(std::string prefixPath, std::string localFile, const std::string &dataType,
        std::vector<std::string> &result);

protected:
    BackupIOInterface &m_backupIO;
    std::string m_fsId;
    ImageInfo m_imageInfo;

    CallBackHandle m_handle;

    // statistics progress
    std::atomic<uint64_t> m_writtenSize {0};
    std::atomic<uint64_t> m_realWrittenSize {0};
    std::atomic<uint64_t> m_deletedFileCount {0};
};
} // namespace Module
#endif