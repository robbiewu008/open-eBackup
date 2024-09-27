package openbackup.data.protection.access.provider.sdk.constants;

/**
 * 恢复任务扩展参数常量定义
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/8/1
 **/
public final class RestoreTaskExtendInfoConstant {
    /**
     * 恢复任务是否开启副本校验
     */
    public static final String ENABLE_COPY_VERIFY = "copyVerify";

    /**
     * 是否需要强制恢复
     */
    public static final String FORCE_RECOVERY = "force_recovery";

    /**
     * 副本校验子任务进度范围开始
     */
    public static final int COPY_VERIFY_RANGE_START = 0;

    /**
     * 副本校验子任务进度范围结束
     */
    public static final int COPY_VERIFY_RANGE_END = 40;

    /**
     * 副本恢复子任务进度范围开始
     */
    public static final int RESTORE_RANGE_START = 41;

    /**
     * 副本恢复子任务进度范围结束
     */
    public static final int RESTORE_RANGE_END = 100;

    private RestoreTaskExtendInfoConstant() {
    }
}
