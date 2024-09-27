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
#ifndef BACKUP_IMAGESET_INTERFACE_H
#define BACKUP_IMAGESET_INTERFACE_H

#include <memory>

#include "LayoutCommon.h"
#include "BackupIOInterface.h"
#include "BackupImageBase.h"

namespace Module {
class BackupImageSetInterface {
public:
    BackupImageSetInterface() {};
    virtual ~BackupImageSetInterface() {};

    virtual LayoutRetCode UpdateBackupSetInfo(const ImageSetInfo &imageSetInfo) = 0;
    virtual std::string GetBackupSetID() const = 0;
    virtual void GetBackupSetInfo(ImageSetInfo &imageSetInfo) const = 0;

    virtual bool CheckIfImageSetExists(const std::string &imageSetId) = 0;
    virtual LayoutRetCode CreateBackupSet(const std::string &imageSetId) = 0;
    virtual LayoutRetCode ReadBackupSetInfo(const std::string &imageSetId) = 0;
    virtual LayoutRetCode RemoveBackupSet() = 0;

    virtual LayoutRetCode ListBackupImages(std::vector<std::string> &imageList) = 0;

    virtual std::unique_ptr<BackupImageInterface> CreateBackupImageInstance(bool aggregated) = 0;
    
    // For backup aggregation mode 
    virtual std::unique_ptr<BackupImageInterface> CreateBackupImageInstance(uint64_t smallFileThreshold,
        uint64_t targetPackSize, int packFormat) = 0;
};
}
#endif // BACKUP_IMAGESET_INTERFACE_H