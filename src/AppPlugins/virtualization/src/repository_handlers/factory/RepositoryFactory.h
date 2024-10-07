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
#ifndef REPOSITORY_FACTORY_HANDLER_H
#define REPOSITORY_FACTORY_HANDLER_H

#include <memory>
#include <common/Macros.h>
#include <common/Structs.h>
#include <repository_handlers/RepositoryHandler.h>

namespace VirtPlugin {
class RepositoryFactory {
public:
    /**
     *  @brief 根据仓类型构造对应的仓实例
     *
     *  @param storageRepo [IN] 仓信息
     *  @return RepositoryHandler 对应类型的仓实例
     */
    static std::shared_ptr<RepositoryHandler> CreateRepositoryHandler(
        const AppProtect::StorageRepository &storageRepo);
};
}

#endif