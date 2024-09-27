package openbackup.system.base.sdk.job.constants;

/**
 * 任务 进度值常量
 *
 * @author t30028453
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-05-06
 */
public class JobProgress {
    /**
     * 任务下发到其他组件时的进度临界值
     */
    public static final int DELIVER_JOB_PROGRESS = 5;

    /**
     * 其他组件上报任务时的进度临界值
     */
    public static final int REPORT_JOB_PROGRESS = 96;
}
