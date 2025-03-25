#ifndef _CACHE_REPOSITORY_H
#define _CACHE_REPOSITORY_H

#include <vector>
#include "common/Types.h"
#include "servicecenter/services/device/Repository.h"

namespace AppProtect {

class CacheRepository : public Repository {
public:
    CacheRepository() : Repository()
    {}
    ~CacheRepository()
    {}

    mp_int32 Umount(
        const std::vector<mp_string> &mountPoints, const mp_string &jobID, const bool &isFileClientMount = false);

protected:
    mp_int32 AssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_void QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path);
};
}
#endif