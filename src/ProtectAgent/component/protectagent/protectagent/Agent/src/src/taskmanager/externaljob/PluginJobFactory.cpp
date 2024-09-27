#include "common/Log.h"
#include "taskmanager/externaljob/PluginMainJob.h"
#include "taskmanager/externaljob/PluginSubPrepJob.h"
#include "taskmanager/externaljob/PluginSubGeneJob.h"
#include "taskmanager/externaljob/PluginSubBusiJob.h"
#include "taskmanager/externaljob/PluginSubPostJob.h"
#include "taskmanager/externaljob/ExecutorBuilder.h"
#include "taskmanager/externaljob/PluginJobFactory.h"

namespace AppProtect {
std::shared_ptr<Job> PluginJobFactory::CreatePluginJob(const PluginJobData& data)
{
    std::shared_ptr<Job> pJob;
    if (data.subID.empty()) {
        pJob = BuildMainJob(data);
    } else {
        if (data.subType == SubJobType::type::PRE_SUB_JOB) {
            pJob = BuildPreSubJob(data);
        } else if (data.subType == SubJobType::type::GENERATE_SUB_JOB) {
            pJob = BuildGeneSubJob(data);
        } else if (data.subType == SubJobType::type::BUSINESS_SUB_JOB) {
            pJob = BuildSubBusiJob(data);
        } else if (data.subType == SubJobType::type::POST_SUB_JOB) {
            pJob = BuildSubPostJob(data);
        } else {
            return pJob;
        }
    }
    INFOLOG("Create plugin job succ, jobId=%s", data.mainID.c_str());
    return pJob;
}

std::shared_ptr<Job> PluginJobFactory::BuildMainJob(const PluginJobData& data)
{
    DBGLOG("Build for BuildMainJob, jobId=%s", data.mainID.c_str());
    std::shared_ptr<AppProtect::PluginMainJob> pJob = std::make_shared<AppProtect::PluginMainJob>(data);
    ExecutorBuilder builder;
    auto mainJobExec = pJob->GetMainJobExec();
    auto reportSuccess = PluginMainJob::GetReportExecutor(PluginMainJob::ActionEvent::MAIN_JOB_FINISHED,
        PluginMainJob::EventResult::SUCCESS, pJob->GetData());
    auto reportFailed = PluginMainJob::GetReportExecutor(PluginMainJob::ActionEvent::MAIN_JOB_FINISHED,
        PluginMainJob::EventResult::FAILED, pJob->GetData());
    auto executor = builder.ConditonNext(mainJobExec, reportFailed, reportSuccess).Build();
    pJob->SetExecutor(executor);
    return pJob;
}

std::shared_ptr<Job> PluginJobFactory::BuildGeneSubJob(const PluginJobData& data)
{
    DBGLOG("Build for BuildGeneSubJob, jobId=%s", data.mainID.c_str());
    std::shared_ptr<PluginSubGeneJob> pJob = std::make_shared<PluginSubGeneJob>(data);
    ExecutorBuilder builder;
    auto reportSuccess = PluginMainJob::GetReportExecutor(PluginMainJob::ActionEvent::EXEC_GENE_SUBJOB,
        PluginMainJob::EventResult::EXECUTING, pJob->GetData());
    auto reportFailed = PluginMainJob::GetReportExecutor(PluginMainJob::ActionEvent::EXEC_GENE_SUBJOB,
        PluginMainJob::EventResult::FAILED, pJob->GetData());
    auto call = pJob->GetPluginCall();
    // 调用成功后阻塞等待结果
    auto wait = pJob->GetWait();
    // 任务失败
    auto jobFailed = pJob->GetJobFailed();
    // 任务成功
    auto jobSuccess = pJob->GetJobSuccess();

    // thrift接口调用失败，上报日志label，置任务失败
    auto callFailed = builder.Next(reportFailed).Next(jobFailed).Build();

    // 上次已经上报过日志label，重试分支不上报，且因插件异常重启只重试一次
    auto redoCallSuccess = builder.ConditonNext(wait, jobFailed, jobSuccess).Build();
    // 重试流程，调用thrift接口，调用失败任务失败，调用成功阻塞等待
    auto redoCall = builder.ConditonNext(call, callFailed, redoCallSuccess).Build();

    // 调用成功等待过程中有三个分支，收到插件上报失败，收到插件上报成功，插件重启重试
    auto callSuccess = builder.Next(reportSuccess).ConditonNext(wait, jobFailed, jobSuccess, redoCall).Build();

    auto executor = builder.ConditonNext(call, callFailed, callSuccess).Build();
    pJob->SetExecutor(executor);
    return pJob;
}

std::shared_ptr<Job> PluginJobFactory::BuildPreSubJob(const PluginJobData& data)
{
    DBGLOG("Build for BuildPreSubJob, jobId=%s", data.mainID.c_str());
    std::shared_ptr<PluginSubPrepJob> pJob = std::make_shared<PluginSubPrepJob>(data);
    ExecutorBuilder builder;
    auto reportSuccess = PluginMainJob::GetReportExecutor(PluginMainJob::ActionEvent::EXEC_PRE_SUBJOB,
        PluginMainJob::EventResult::EXECUTING, pJob->GetData());
    auto reportFailed = PluginMainJob::GetReportExecutor(PluginMainJob::ActionEvent::EXEC_PRE_SUBJOB,
        PluginMainJob::EventResult::FAILED, pJob->GetData());
    auto call = pJob->GetPluginCall();
    // 调用成功后阻塞等待结果
    auto wait = pJob->GetWait();
    // 任务失败
    auto jobFailed = pJob->GetJobFailed();
    // 任务成功
    auto jobSuccess = pJob->GetJobSuccess();

    // thrift接口调用失败，上报日志label，置任务失败
    auto callFailed = builder.Next(reportFailed).Next(jobFailed).Build();

    // 上次已经上报过日志label，重试分支不上报，且因插件异常重启只重试一次
    auto redoCallSuccess = builder.ConditonNext(wait, jobFailed, jobSuccess).Build();
    // 重试流程，调用thrift接口，调用失败任务失败，调用成功阻塞等待
    auto redoCall = builder.ConditonNext(call, callFailed, redoCallSuccess).Build();

    // 调用成功等待过程中有三个分支，收到插件上报失败，收到插件上报成功，插件重启重试
    auto callSuccess = builder.Next(reportSuccess).ConditonNext(wait, jobFailed, jobSuccess, redoCall).Build();

    auto executor = builder.ConditonNext(call, callFailed, callSuccess).Build();
    pJob->SetExecutor(executor);
    return pJob;
}

std::shared_ptr<Job> PluginJobFactory::BuildSubBusiJob(const PluginJobData& data)
{
    DBGLOG("Build for BuildSubBusiJob, jobId=%s", data.mainID.c_str());
    std::shared_ptr<PluginSubBusiJob> pJob = std::make_shared<PluginSubBusiJob>(data);
    ExecutorBuilder builder;
    auto reportNasFailed = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::MOUNT_NAS,
        PluginSubJob::EventResult::FAILED, pJob->GetData());
    auto reportCallFailed = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::EXEC_BUSI_SUBJOB,
        PluginSubJob::EventResult::FAILED, pJob->GetData());

    auto nasExecutor = pJob->GetMountNasExecutor();
    auto nasFailedExecutor = builder.Next(reportNasFailed).Next(pJob->GetJobFailed()).Build();
    auto call = pJob->GetPluginCall();
    // 任务失败
    auto callFailed = builder.Next(reportCallFailed).Next(pJob->GetJobFailed()).Build();
    auto jobSuccess = pJob->GetJobSuccess();
    
    auto pluginCallExecutor = builder.ConditonNext(call, callFailed, jobSuccess).Build();
    auto executor = builder.ConditonNext(nasExecutor, nasFailedExecutor, pluginCallExecutor).Build();
    pJob->SetExecutor(executor);
    return pJob;
}

std::shared_ptr<Job> PluginJobFactory::BuildSubPostJob(const PluginJobData& data)
{
    DBGLOG("Build for BuildSubPostJob, jobId=%s", data.mainID.c_str());
    std::shared_ptr<PluginSubPostJob> pJob = std::make_shared<PluginSubPostJob>(data);
    ExecutorBuilder builder;
    auto reportNasFailed = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::MOUNT_NAS,
        PluginSubJob::EventResult::FAILED, pJob->GetData());
    auto reportCallFailed = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::EXEC_POST_SUBJOB,
        PluginSubJob::EventResult::FAILED, pJob->GetData());
    auto reportPostScriptFailed = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::EXEC_POST_SCRIPT,
        PluginSubJob::EventResult::EXEC_SCRIPT_FAILED, pJob->GetData());
    auto reportPostScriptSuccess = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::EXEC_POST_SCRIPT,
        PluginSubJob::EventResult::EXEC_SCRIPT_SUCCESS, pJob->GetData());
    auto reportPostStartExec = PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent::EXEC_POST_SUBJOB,
        PluginSubJob::EventResult::EXECUTING, pJob->GetData());
    auto nasExecutor = pJob->GetMountNasExecutor();
    auto call = pJob->GetPluginCall();

    // execute post script
    auto execPostScript = pJob->ExecPostScript();
    
    auto jobFailed = pJob->GetJobFailed();
    auto nasFailed = builder.Next(reportNasFailed).Next(jobFailed).Build();
    auto callFailed = builder.Next(reportCallFailed).Next(jobFailed).Build();

    auto jobSuccess = pJob->GetJobSuccess();
    auto callSuccess = builder.Next(jobSuccess).Build();
    auto pluginCallExecutor = builder.ConditonNext(call, callFailed, callSuccess).Build();
    auto postScriptSuccess = builder.Next(reportPostScriptSuccess).Next(pluginCallExecutor).Build();
    auto postScriptFailed = builder.Next(reportPostScriptFailed).Next(pluginCallExecutor).Build();
    auto postScriptExecutor = builder.Next(reportPostStartExec).ConditonNext(execPostScript, postScriptFailed,
        postScriptSuccess).Build();

    auto executor = builder.ConditonNext(nasExecutor, nasFailed, postScriptExecutor).Build();
    pJob->SetExecutor(executor);
    return pJob;
}
}