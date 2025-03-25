#ifndef _DME_JOB_STATE_DB_H__
#define _DME_JOB_STATE_DB_H__

#include "taskmanager/externaljob/Job.h"
#include "common/Types.h"
#include <vector>
#include <memory>

namespace AppProtect {
class JobStateDB {
public:
    ~JobStateDB()
    {}
    static JobStateDB& GetInstance();

    mp_int32 QueryAllJob(std::vector<PluginJobData>& result);

    mp_int32 QueryJob(const mp_string& mainId, const mp_string& subId, PluginJobData& jobData);

    mp_int32 InsertRecord(const PluginJobData& jobData);

    mp_int32 DeleteRecord(const mp_string& mainId, const mp_string& subId);

    mp_int32 DeleteRecord(const mp_string& mainId);

    mp_int32 UpdateStatus(const mp_string& mainId, const mp_string& subId, mp_uint32 status);

    mp_int32 UpdateMountPoints(const mp_string& mainId, const mp_string& subId,
        const std::vector<mp_string>& mountPoints);

    mp_int32 UpdateRunEnable(const mp_string& mainId, mp_int32 runEnable);

private:
    JobStateDB(){};
};
}  // namespace AppProtect
#endif