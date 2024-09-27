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