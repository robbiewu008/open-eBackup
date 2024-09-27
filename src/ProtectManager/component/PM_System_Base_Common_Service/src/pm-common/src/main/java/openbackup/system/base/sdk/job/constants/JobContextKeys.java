package openbackup.system.base.sdk.job.constants;

/**
 * 任务上下文keys
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-22
 */
public class JobContextKeys {
    /**
     * 还在运行中的Kafka流程数
     */
    public static final String RUNNING_STATE_COUNT = "RunningStateCount";

    /**
     * 尝试在PM侧停止任务次数
     */
    public static final String ABORTING_STATE_COUNT = "AbortingStateCount";
}
