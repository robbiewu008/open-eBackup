/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file RepositoryFactory.h
 * @brief Mount And Umount File System Repository Factory
 * @version 1.1.0
 * @date 2022-02-22
 * @author lixilong wx1101878
 */
#ifndef _REPOSITORY_FACTORY_H
#define _REPOSITORY_FACTORY_H

#include <vector>
#include "common/Types.h"
#include "servicecenter/services/device/Repository.h"

namespace AppProtect {
class RepositoryFactory {
public:
    RepositoryFactory()
    {}
    ~RepositoryFactory()
    {}

    std::shared_ptr<Repository> CreateRepository(RepositoryDataType::type stRep);
};
}
#endif