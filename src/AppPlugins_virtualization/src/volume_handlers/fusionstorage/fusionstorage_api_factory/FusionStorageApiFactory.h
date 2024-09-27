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
#ifndef FUSION_STORAGE_API_FACTORY_H
#define FUSION_STORAGE_API_FACTORY_H

#include <string>
#include <memory>
#include <common/Structs.h>
#include "FusionStorageRestApiOperator.h"

namespace VirtPlugin {
class FusionStorageApiFactory {
public:
    FusionStorageApiFactory(const FusionStorageApiFactory &) = delete;
    FusionStorageApiFactory &operator=(const FusionStorageApiFactory &) = delete;

    static FusionStorageApiFactory *GetInstance();

    std::shared_ptr<FusionStorageApi> CreateFusionStorageApi(
        const std::string &apiMode, const std::string &fusionStorMgrIp, const std::string &poolID);

    ~FusionStorageApiFactory();

private:
    FusionStorageApiFactory();

private:
    std::string m_apiMode;
};
}  // namespace VirtPlugin

#endif  // _FUSION_STORAGE_API_FACTORY_H_