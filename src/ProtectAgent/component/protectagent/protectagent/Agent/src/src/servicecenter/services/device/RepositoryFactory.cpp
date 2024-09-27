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