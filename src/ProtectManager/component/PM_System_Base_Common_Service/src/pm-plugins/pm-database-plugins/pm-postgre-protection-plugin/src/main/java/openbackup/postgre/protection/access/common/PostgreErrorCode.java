package openbackup.postgre.protection.access.common;

/**
 * PostgreSQL相关错误码
 *
 * @author wwx1013713
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/09/03
 */
public class PostgreErrorCode {
    /**
     * 副本版本和目标实例版本不匹配
     */
    public static final long VERSION_NOT_MATCH_BEFORE_RESTORE = 1577210056L;

    /**
     * 副本的OS用户和目标实例的OS用户不一致
     */
    public static final long OS_USER_NOT_EQUAL_BEFORE_RESTORE = 1577210057L;

    /**
     * IP地址不正确
     */
    public static final long SERVICE_IP_IS_INVALID = 1577213580L;
}
