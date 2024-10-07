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
#ifndef OBSCTX_CLOUDE_SERVICE_MANAGER
#define OBSCTX_CLOUDE_SERVICE_MANAGER

#include <memory>
#include "storage/huawei/HWCloudService.h"
#include "storage/alibaba/ALiCloudService.h"
#include "common/CloudServiceCommonStruct.h"

namespace Module {

    struct StorageConfig {
        StorageType storageType;
        StorageVerifyInfo verifyInfo;
    };

    class CloudServiceManager {
    public:
        static std::unique_ptr<CloudServiceInterface> CreateInst(const StorageConfig& storageConfig) {
            if (storageConfig.storageType == StorageType::PACIFIC) {
                return std::make_unique<HWCloudService>(storageConfig.verifyInfo, obs_auth_switch::OBS_S3_TYPE);
            } else if (storageConfig.storageType == StorageType::HUAWEI) {
                return std::make_unique<HWCloudService>(storageConfig.verifyInfo, obs_auth_switch::OBS_OBS_TYPE);
            } else if (storageConfig.storageType == StorageType::ALI) {
                return std::make_unique<ALiCloudService>(storageConfig.verifyInfo);
            } else {
                return nullptr;
            }
        };
    };
}

#endif  // OBSCTX_CLOUDE_SERVICE_MANAGER
