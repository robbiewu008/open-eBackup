package openbackup.system.base.sdk.job.util;

import openbackup.system.base.sdk.job.constants.JobProgress;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

/**
 * 任务更新工具类
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-20
 *
 */
public class JobUpdateUtil {
    /**
     * 获取 进度为5 的任务更新体
     *
     * @return 进度为5 的任务更新体
     */
    public static UpdateJobRequest getDeliverReq() {
        return getUpdateJobRequest(JobProgress.DELIVER_JOB_PROGRESS);
    }

    /**
     * 获取 进度为96 的任务更新体
     *
     * @return 进度为96 的任务更新体
     */
    public static UpdateJobRequest getReportReq() {
        return getUpdateJobRequest(JobProgress.REPORT_JOB_PROGRESS);
    }

    private static UpdateJobRequest getUpdateJobRequest(int progress) {
        UpdateJobRequest jobRequest = new UpdateJobRequest();
        jobRequest.setProgress(progress);
        return jobRequest;
    }
}

