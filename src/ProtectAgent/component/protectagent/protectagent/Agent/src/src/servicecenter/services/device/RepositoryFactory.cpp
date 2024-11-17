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
#include "servicecenter/services/device/RepositoryFactory.h"
#include "servicecenter/services/device/CacheRepository.h"
#include "servicecenter/services/device/LogRepository.h"
namespace AppProtect {
std::shared_ptr<Repository> RepositoryFactory::CreateRepository(RepositoryDataType::type repositoryType)
{
    std::shared_ptr<Repository> pRepository;
    if (repositoryType == RepositoryDataType::type::CACHE_REPOSITORY) {
        pRepository = std::make_shared<AppProtect::CacheRepository>();
    } else if (repositoryType == RepositoryDataType::type::LOG_REPOSITORY) {
        pRepository = std::make_shared<AppProtect::LogRepository>();
    } else {
        pRepository = std::make_shared<AppProtect::Repository>();
    }
    return pRepository;
}
}