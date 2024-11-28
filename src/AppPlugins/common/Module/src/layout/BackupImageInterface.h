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
#ifndef BACKUP_IMAGE_INTERFACE_H
#define BACKUP_IMAGE_INTERFACE_H

#include <memory>
#include <list>
#include <map>

#include "LayoutCommon.h"
#include "BackupIOInterface.h"

namespace Module {
constexpr int FILE_LIST_SIZE = 50;

class BackupImageInterface {
public:
    virtual ~BackupImageInterface() = default;

    virtual LayoutRetCode UpdateImageInfo(const ImageInfo &imageInfo) = 0;
    virtual ImageInfo GetImageInfo() const = 0;

    virtual void IncreaseImageWrittenSize(uint64_t size) = 0;
    virtual void IncreaseImageRealWrittenSize(uint64_t size) = 0;
    virtual void IncreaseImageDeletedCount(uint64_t size) = 0;

    /* image operation */
    virtual bool CheckIfImageExists(const ImageInfo &imageInfo) = 0;
    virtual LayoutRetCode CreateImage(const ImageInfo &imageInfo) = 0;
    virtual LayoutRetCode DeleteImage(const ImageInfo &imageInfo) = 0;
    virtual LayoutRetCode OpenImage(ImageInfo &imageInfo) = 0;
    virtual LayoutRetCode CloseImage() = 0;
};
} // namespace Module
#endif // BACKUP_IMAGE_INTERFACE_H
