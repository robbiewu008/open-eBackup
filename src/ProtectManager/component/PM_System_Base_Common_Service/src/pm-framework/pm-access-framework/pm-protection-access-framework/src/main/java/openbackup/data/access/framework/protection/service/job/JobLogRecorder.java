package openbackup.data.access.framework.protection.service.job;

import openbackup.data.access.framework.protection.common.constants.JobStatusLabelConstant;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Locale;

/**
 * 任务日志记录器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/27
 **/
@Component
public class JobLogRecorder {
    private static final String JOB_STATUS_INFO_TEMPLATE = "job_status_%s_label";

    private final JobService jobService;

    /**
     * 任务步骤记录器构造函数
     *
     * @param jobService 任务服务
     */
    public JobLogRecorder(JobService jobService) {
        this.jobService = jobService;
    }

    /**
     * 构造任务步骤参数
     *
     * @param jobStatus 任务装填
     * @return 参数列表
     */
    public static List<String> buildStepParam(ProviderJobStatusEnum jobStatus) {
        return Collections.singletonList(
            String.format(Locale.ENGLISH, JOB_STATUS_INFO_TEMPLATE, jobStatus.name().toLowerCase(Locale.ROOT)));
    }

    /**
     * 记录错误的任务步骤
     *
     * @param jobId 任务id
     * @param label 任务步骤的标签，用于国际化展示
     * @param errorCode 错误码
     * @param errorParams 错误码描述中占位符对应的参数
     */
    public void recordJobStepWithError(String jobId, String label, Long errorCode, List<String> errorParams) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo(label);
        jobLogBo.setUnique(true);
        jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
        jobLogBo.setLogInfoParam(Collections.singletonList(JobStatusLabelConstant.JOB_FAIL_LABEL));
        jobLogBo.setLogDetail(errorCode + "");
        jobLogBo.setLogDetailParam(errorParams);
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(Collections.singletonList(jobLogBo));
        jobService.updateJob(jobId, request);
    }

    /**
     * 记录正常任务步骤
     *
     * @param jobId 任务id
     * @param level 任务步骤的级别，用于任务界面图标显示的类型
     * @param label 任务步骤的标签，用于国际化展示
     * @param params 任务步骤中的信息参数
     */
    public void recordJobStep(String jobId, JobLogLevelEnum level, String label, List<String> params) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo(label);
        jobLogBo.setUnique(true);
        jobLogBo.setLevel(level.getValue());
        jobLogBo.setLogInfoParam(params);
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(Collections.singletonList(jobLogBo));
        jobService.updateJob(jobId, request);
    }

    /**
     * 根据任务状态获取log的级别信息
     * <p>
     * 当前只支持任务完结状态
     * </p>
     *
     * @param jobStatus 任务状态
     * @return 任务log级别 {@code JobLogLevelEnum}
     */
    public JobLogLevelEnum getLevelByJobStatus(ProviderJobStatusEnum jobStatus) {
        switch (jobStatus) {
            case SUCCESS:
            case PARTIAL_SUCCESS:
            case ABORTED:
            case CANCELLED:
                return JobLogLevelEnum.INFO;
            case FAIL:
            case ABORT_FAILED:
                return JobLogLevelEnum.ERROR;
            default:
                throw new IllegalArgumentException("function not support status=" + jobStatus.name());
        }
    }

    /**
     * 更新任务
     *
     * @param jobId 任务id
     * @param request 任务更新内容
     */
    public void updateJob(String jobId, UpdateJobRequest request) {
        jobService.updateJob(jobId, request);
    }
}
