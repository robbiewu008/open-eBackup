package openbackup.system.base.common.constants;

/**
 * FeignClient策略常量
 *
 * @since 2022-04-29
 */
public class FeignClientConstant {
    /**
     * FeignClient连接默认超时时间(ms)
     */
    public static final int CONNECT_TIMEOUT = 30 * 1000; // 30s

    /**
     * FeignClient连接默认超时时间(s)
     */
    public static final int CONNECT_TIMEOUT_SEC = 30; // 30s

    /**
     * FeignClient读取默认超时(ms)
     */
    public static final int READ_TIMEOUT = 2 * 60 * 1000; // 2分钟

    /**
     * FeignClient读取默认超时(s)
     */
    public static final int READ_TIMEOUT_SEC = 2 * 60; // 2分钟

    /**
     * VMwareFeignClient读取默认超时(ms)
     */
    public static final int VMWARE_READ_TIMEOUT = 60 * 60 * 1000; // 60分钟

    /**
     * Member FeignClient读取默认超时(ms)
     */
    public static final int MEMBER_READ_TIMEOUT = 5 * 60 * 1000; // 5分钟

    /**
     * FeignClient Retry 间隔周期(ms)
     */
    public static final int PERIOD = 60 * 1000; // 1分钟

    /**
     * DME Retry 间隔周期(ms)
     */
    public static final int DME_PERIOD = 5 * 1000; // 5秒钟

    /**
     * FeignClient Retry 最大间隔周期(ms)
     */
    public static final int MAX_PERIOD = 60 * 1000; // 1分钟

    /**
     * DME Retry 最大间隔周期(ms)
     */
    public static final int DME_MAX_PERIOD = 5 * 1000; // 5秒钟

    /**
     * FeignClient Retry 重试次数
     */
    public static final int MAX_ATTEMPTS = 3;
}
