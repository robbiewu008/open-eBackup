#include "taskmanager/externaljob/PluginSubGeneJob.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"

namespace AppProtect {
namespace {
    const mp_int32 GENERATE_TIMEOUT = 5 * 60 * 1000;
}

void PluginSubGeneJob::NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails)
{
    m_ret.store(GetSignedFromJobDetail(jobDetails.jobStatus));
    m_sem.Signal();
    PluginSubJob::NotifyJobDetail(jobDetails);
}

mp_int32 PluginSubGeneJob::ExecBackupJob()
{
    ActionResult ret;
    BackupJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Backup gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncBackupGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecRestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Restore gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncRestoreGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecDelCopyJob()
{
    ActionResult ret;
    DelCopyJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Delcopy gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncDelCopyGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecCheckCopyJob()
{
    ActionResult ret;
    CheckCopyJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Checkcopy gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncCheckCopyGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecLivemountJob()
{
    ActionResult ret;
    LivemountJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Livemount gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncLivemountGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecCancelLivemountJob()
{
    ActionResult ret;
    CancelLivemountJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Cancel livemount gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncCancelLivemountGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecInrestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Instant restore gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncInstantRestoreGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_int32 PluginSubGeneJob::ExecBuildIndexJob()
{
    ActionResult ret;
    BuildIndexJob jobParam;
    mp_int32 nodeNum;
    ParserNodeNum(nodeNum);
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Build index gen job, jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncBuildIndexGenerateSubJob, ret, jobParam, nodeNum);
    return ret.code;
}

mp_void PluginSubGeneJob::ParserNodeNum(mp_int32& nodeNum)
{
    nodeNum = 0;
    if (m_data.param.isMember("agents") && m_data.param["agents"].isArray()) {
        nodeNum = m_data.param["agents"].size();
    }
}

mp_int32 PluginSubGeneJob::WaitPluginNotify()
{
    mp_int32 ret = MP_SUCCESS;
    while (true) {
        if (m_sem.TimedWait(GENERATE_TIMEOUT)) {
            ret = m_ret.load();
            DBGLOG("Generator job, jobId=%s receive ret: %d.", m_data.mainID.c_str(), ret);
            if (ret == MP_EAGAIN) {
                continue;
            }
            INFOLOG("Generator job, jobId=%s finish, ret: %d.", m_data.mainID.c_str(), ret);
        } else {
            WARNLOG("Generator job, jobId=%s timeout.", m_data.mainID.c_str());
            ret = MP_FAILED;
        }
        break;
    }
    return ret;
}

bool PluginSubGeneJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    INFOLOG("jobId=%s notify plugin reload, m_pluginPID:%s newPluginPID:%s.",
        m_data.mainID.c_str(), m_pluginPID.c_str(), newPluginPID.c_str());
    if (!m_pluginPID.empty() && m_pluginPID != newPluginPID) {
        m_ret.store(MP_REDO);
        m_sem.Signal();
        return true;
    }
    return false;
}

Executor PluginSubGeneJob::GetPluginCall()
{
    std::map<MainJobType, Executor> ExcuterMap = {
        { MainJobType::BACKUP_JOB, [this](int32_t)
            {
                return ExecBackupJob();
            }
        },
        { MainJobType::RESTORE_JOB, [this](int32_t)
            {
                return ExecRestoreJob();
            }
        },
        { MainJobType::DELETE_COPY_JOB, [this](int32_t)
            {
                return ExecDelCopyJob();
            }
        },
        { MainJobType::LIVEMOUNT_JOB, [this](int32_t)
            {
                return ExecLivemountJob();
            }
        },
        { MainJobType::CANCEL_LIVEMOUNT_JOB, [this](int32_t)
            {
                return ExecCancelLivemountJob();
            }
        },
        { MainJobType::BUILD_INDEX_JOB, [this](int32_t)
            {
                return ExecBuildIndexJob();
            }
        },
        { MainJobType::INSTANT_RESTORE_JOB, [this](int32_t)
            {
                return ExecInrestoreJob();
            }
        },
        { MainJobType::CHECK_COPY_JOB, [this](int32_t)
            {
                return ExecCheckCopyJob();
            }
        }
    };
    auto it = ExcuterMap.find(m_data.mainType);
    if (it != ExcuterMap.end()) {
        return it->second;
    }
    return GetEmptyExcutor();
}

Executor PluginSubGeneJob::GetWait()
{
    std::set<MainJobType> ExcuterSet = {
        MainJobType::BACKUP_JOB,
        MainJobType::RESTORE_JOB,
        MainJobType::LIVEMOUNT_JOB,
        MainJobType::CANCEL_LIVEMOUNT_JOB,
        MainJobType::BUILD_INDEX_JOB,
        MainJobType::INSTANT_RESTORE_JOB,
        MainJobType::DELETE_COPY_JOB,
        MainJobType::CHECK_COPY_JOB
    };
    auto it = ExcuterSet.find(m_data.mainType);
    if (it != ExcuterSet.end()) {
        return [this](int32_t result) {
            return WaitPluginNotify();
        };
    }
    INFOLOG("GetAfterCall Job jobId=%s, No Need to wait.", m_data.mainID.c_str());
    return GetEmptyExcutor();
}

mp_int32 PluginSubGeneJob::NotifyPauseJob()
{
    INFOLOG("Main Job jobId=%s, pause job.", m_data.mainID.c_str());
    m_ret.store(MP_FAILED);
    m_sem.Signal();
    return MP_SUCCESS;
}
}