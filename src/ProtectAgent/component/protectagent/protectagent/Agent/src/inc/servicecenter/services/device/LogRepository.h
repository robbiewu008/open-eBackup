#ifndef _LOG_REPOSITORY_H
#define _LOG_REPOSITORY_H

#include <vector>
#include "common/Types.h"
#include "servicecenter/services/device/Repository.h"

namespace AppProtect {

class LogRepository : public Repository {
public:
    LogRepository() : Repository()
    {}
    ~LogRepository()
    {}

protected:
    mp_int32 AssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_void QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path);
private:
    mp_bool isNeedOracleMigrate(const PluginJobData &data);
    mp_bool isSkipLogRepoCompose(const Json::Value& jobParam);
    mp_void AssembleLogMetaPath(StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_void AssembleLogPath(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
};
}
#endif