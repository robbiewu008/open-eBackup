#ifndef _PLUGIN_MAIN_JOB_H
#define _PLUGIN_MAIN_JOB_H

#include <vector>
#include <map>
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "taskmanager/externaljob/Job.h"
#include "taskmanager/externaljob/JobStateAction.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"


namespace AppProtect {
/**
 * plugin main job, main job will execute external plugin main job, as follow
 * 1.generate sub job, including prerequisite,generate jobs
 * 2.execute prerequisite job
 * 3.execute generate job
 */
class PluginMainJob : public Job {
    using mainJobAction = JobStateAction<MainJobState>;
    using mainJobActionPtr = std::shared_ptr<mainJobAction>;
    using mainJobActionMap = std::map<MainJobState, mainJobActionPtr>;
public:
    enum class ActionEvent {
        JOB_START_EXEC,
        GENE_POST_JOB,
        MOUNT_NAS,
        CHECK_BACKUP_TYPE,
        PLUGIN_LOG_BACKUP,
        CONFIG_QOS,
        EXEC_PRE_SCRIPTR,
        EXEC_PRE_SUBJOB,
        EXEC_GENE_SUBJOB,
        UNMOUNT_NAS,
        MAIN_JOB_FINISHED
    };

    enum class EventResult {
        START,
        EXECUTING,
        SUCCESS,
        FAILED
    };

public:
    PluginMainJob(const PluginJobData& data);
    virtual ~PluginMainJob();
    mp_int32 Initialize();

    mp_int32 Exec();
    mp_int32 ExecMainJob();
    mp_int32 Abort();
    void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) override;
    bool NeedReportJobDetail() override;
    void FetchJobDetail(AppProtect::SubJobDetails& jobDetails) override;
    void StopReportJobDetail() override;

    mp_int32 CanbeRunInLocalNode() override;
    mp_int32 UpdateStatus(mp_uint32 status) override;
    void GetPermission(AppProtect::JobPermission &jobPermit) override;

    Executor GetMainJobExec();

    static mp_int32 ReportInSubJob(ActionEvent event, EventResult result, mp_int32 resultCode,
        const PluginJobData& data);
    static mp_void ReportJobDetail(ActionEvent event, EventResult result, mp_int32 resultCode,
        const PluginJobData& data);
    static bool DetermineWhetherToReportAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
        mp_int32 resultCode, const PluginJobData& data);
    static mp_int32 ReportAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
        mp_int32 resultCode, const PluginJobData& data);
    static Executor GetReportExecutor(ActionEvent event, EventResult result, const PluginJobData& data);

    virtual bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID);
    mp_int32 NotifyPauseJob() override;
private:
    // state enum and job state action relationship
    mainJobActionMap m_mapStates;
    std::shared_ptr<Job> m_pPrepJob;
    std::shared_ptr<Job> m_pGeneJob;

    std::map<MainJobState, AbortHandleFun> m_abortHandleMap;
    bool m_reportFlag {true};

private:
    mp_int32 GenerateMainJob();  // generate main job and save the post sub job to DME

    mp_int32 CheckBackupJobType();  // check the main job type, whether it will convert incremental to full backup

    mp_int32 ExecutePreScript();  // execute pre script

    mp_int32 ExecutePreSubJob();  // execute pre sub job

    mp_int32 ExecGenerateSubJob();  // execute generate sub job

    bool IsTimeoutLastJobReport();

    bool IsDataturboOpen(const PluginJobData &data);
 
    mp_int32 CheckESN();
    
    void GeneratorMainJobDetail(AppProtect::SubJobDetails& jobDetails);
    void AbortBeforePreJobComplete();
    void NotifyPluginAbort();  // need to notify plugin stop
    void ReportDmeIncToFull();
    void ReportDmeIncToDiff();

    mp_int32 SetQosStrategy();
    mp_int32 LogBackup();
    mp_int32 SendQosRequest(const Json::Value &qosJsonValue);
private:
    StateAction MakeAction(const StateAction& action, ActionEvent event);
    mp_int32 BeforeAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result, mp_int32 resultCode);
    mp_int32 AfterAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result, mp_int32 resultCode);
    mp_int32 LogBeforeAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
        mp_int32 resultCode);
    mp_int32 LogAfterAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result, mp_int32 resultCode);
    mp_int32 LogJobAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result, mp_int32 resultCode);
    mp_int32 ReportBeforeAction(ActionEvent event, EventResult result, mp_int32 resultCode);
    mp_int32 ReportAfterAction(ActionEvent event, EventResult result, mp_int32 resultCode);
    mp_bool IsCheckBackupJobType();
};
}  // namespace AppProtect

#endif
