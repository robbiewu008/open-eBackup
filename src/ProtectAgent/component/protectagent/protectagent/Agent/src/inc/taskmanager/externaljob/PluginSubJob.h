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
#ifndef _PLUGIN_SUB_JOB_H_
#define _PLUGIN_SUB_JOB_H_

#include <functional>
#include "taskmanager/externaljob/Job.h"
#include "servicecenter/timerservice/include/ITimer.h"

namespace AppProtect {

class ProtectServiceIf;

class PluginSubJob : public Job {
public:
    enum class ActionEvent {
        JOB_START_EXEC,
        MOUNT_NAS,
        EXEC_BUSI_SUBJOB,
        EXEC_POST_SCRIPT,
        EXEC_POST_SUBJOB,
        UNMOUNT_NAS,
        SUB_JOB_FINISHED
    };

    enum class EventResult {
        START,
        EXECUTING,
        SUCCESS,
        FAILED,
        EXEC_SCRIPT_SUCCESS,
        EXEC_SCRIPT_FAILED
    };

    PluginSubJob(const PluginJobData& data);
    virtual ~PluginSubJob();
    mp_int32 Exec();
    static bool CheckSubJobCompeleteStatus(const AppProtect::SubJobDetails& jobDetails);
    void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) override;
    bool NeedReportJobDetail() override;
    mp_int32 Abort();
    virtual Executor GetJobSuccess();
    virtual Executor GetJobFailed();

    virtual Executor GetPluginCall()
    {
        return GetEmptyExcutor();
    };

    virtual Executor GetWait()
    {
        return GetEmptyExcutor();
    };
    
    static bool DetermineWhetherToReportAction(PluginSubJob::ActionEvent event, PluginSubJob::EventResult result,
        int32_t resultCode, const PluginJobData& data);
    static mp_int32 ReportAction(PluginSubJob::ActionEvent event, PluginSubJob::EventResult result, int32_t resultCode,
        const PluginJobData& data);
    static Executor GetReportExecutor(PluginSubJob::ActionEvent event, PluginSubJob::EventResult result,
        const PluginJobData& data);
protected:
    virtual mp_int32 ExecBackupJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecRestoreJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecDelCopyJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecCheckCopyJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecLivemountJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecCancelLivemountJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecBuildIndexJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 ExecInrestoreJob(std::shared_ptr<ProtectServiceIf> pClient)
    {
        return MP_SUCCESS;
    }
    void ReportDetails();  // this function only used when the logDetail is generated by agent

    void ChangeState(SubJobState state);
    bool AbortTimeoutHandler();
    void AbortBeforePluginRun();
    void NotifyPluginAbort();

    Executor GetEmptyExcutor();

    virtual bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID);

    mp_int32 GetSignedFromJobDetail(AppProtect::SubJobStatus::type jobStatus);
protected:
    static bool IsSubJobFinish(SubJobState state);
    void RemovePluginTimer();
    
private:
    std::map<MainJobType, std::function<mp_int32(std::shared_ptr<ProtectServiceIf>)>> m_mapFunc;
    // m_pluginUdTimeoutHandler: the handlers under different state when external plugin updating detail timout
    std::shared_ptr<timerservice::ITimer> m_pluginUdTimer;  // external update detail timeout timer
    int32_t m_pluginUdTimeId;                               // external update detail timeout timer id
    std::map<SubJobState, timerservice::TimeoutExecuter> m_pluginUdTimeoutHandler;
    std::map<SubJobState, AbortHandleFun> m_abortHandleMap;

    std::mutex m_mutexChangeState;
};

}  // namespace AppProtect

#endif
