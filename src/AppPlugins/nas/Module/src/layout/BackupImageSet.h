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
#ifndef BACKUP_IMAGESET_H
#define BACKUP_IMAGESET_H

#include <memory>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include "BackupImageSetInterface.h"

namespace Module {
class BackupImageSet : public BackupImageSetInterface {
public:
    BackupImageSet(BackupIOInterface &backupIO, const AggregationInfo &aggregateInfo);
    ~BackupImageSet() {}

    LayoutRetCode UpdateBackupSetInfo(const ImageSetInfo &imageSetInfo) override;
    std::string GetBackupSetID() const override;
    void GetBackupSetInfo(ImageSetInfo &imageSetInfo) const override;

    bool CheckIfImageSetExists(const std::string &imageSetId) override;
    LayoutRetCode CreateBackupSet(const std::string &imageSetId) override;
    LayoutRetCode ReadBackupSetInfo(const std::string &imageSetId) override;
    LayoutRetCode RemoveBackupSet() override;

    LayoutRetCode ListBackupImages(std::vector<std::string> &imageList) override;

    std::unique_ptr<BackupImageInterface> CreateBackupImageInstance(bool aggregated) override;
    std::unique_ptr<BackupImageInterface> CreateBackupImageInstance(uint64_t smallFileThreshold,
        uint64_t targetPackSize, int packFormat) override;

protected:
    bool WriteImageSetInfo(const ImageSetInfo &imageSetInfo);

private:
    BackupIOInterface &m_backupIO;
    ImageSetInfo m_imageSetInfo;
};  
} // namespace Module
#endif // BACKUP_IMAGESET_H