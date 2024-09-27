package openbackup.data.access.framework.restore.constant;

/**
 * 恢复任务中任务步骤国际化标签常量类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/10
 **/
public class RestoreJobLabelConstant {
    /**
     * 恢复任务初始化
     */
    public static final String RESTORE_INIT = "job_log_copy_recovery_schedule_label";

    /**
     * 恢复任务执行
     */
    public static final String RESTORE_START = "job_log_protection_restore_execute_label";

    /**
     * 恢复任务完成
     */
    public static final String RESTORE_COMPLETE = "job_log_copy_recovery_complete_label";

    private RestoreJobLabelConstant() {}
}
