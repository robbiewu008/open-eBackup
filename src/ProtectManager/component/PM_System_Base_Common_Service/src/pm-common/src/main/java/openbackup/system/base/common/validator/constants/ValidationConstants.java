package openbackup.system.base.common.validator.constants;

/**
 * 验证错误码类
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-05
 */
public class ValidationConstants {
    /**
     * MTU最小值
     */
    public static final int MIN_MTU = 1280;

    /**
     * MTU最大值
     */
    public static final int MAX_MTU = 9600;

    /**
     * MTU校验错误返回message信息
     */
    public static final String WRONG_MTU_MESSAGE = "Please input mtu in range [" + MIN_MTU + "," + MAX_MTU + "]";
}
