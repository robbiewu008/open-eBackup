package openbackup.system.base.common.constants;

/**
 * 错误码定义类
 *
 * @author p00511147
 * @since 2020-012-10
 */
public class StorageCommonErrorCode {
    /**
     * 参数错误。
     */
    public static final long PARA_ERROR = 50331651L;

    /**
     * 用户名或密码错误。
     */
    public static final long USER_PSW_ERROR = 1077949061L;

    /**
     * 用户名或密码错误
     */
    public static final long DORADO_USER_PSW = 1077987870L;

    /**
     * 系统繁忙。
     */
    public static final long SYSTEM_BUSY = 1077949006L;

    /**
     * 已达最大用户数。
     */
    public static final long USER_ACCESS_COUNT = 1077949067L;

    /**
     * 用户帐号已锁定。
     */
    public static final long ACCOUNT_LOCKED = 1077987871L;

    /**
     * 用户名或密码错误或LDAP用户与本地用户重名。
     */
    public static final long LDAP_LOCAL_USER_REPEAT_ERROR = 1077949081L;

    /**
     * 用户帐号已锁定。
     */
    public static final long USER_ACCOUNT_LOCKED = 1077949070L;

    /**
     * 用户帐号已锁定。
     */
    public static final long USER_ACCOUNT_LOCKED_IDLE = 1073793581L;

    /**
     * IP地址已锁定。
     */
    public static final long IP_LOCKED = 1077949071L;

    /**
     * 帐户不支持此登录方式。
     */
    public static final long UNSUPPORTED_LOGIN = 1073793593L;

    /**
     * 名称包含的字符数必须为1到31个。
     */
    public static final long NAME_ERROR = 1077949103L;

    /**
     * 名称中包含非法字符。
     */
    public static final long NAME_INVALID = 1077949106L;

    /**
     * 系统处于多控模式
     */
    public static final long SYSTEM_MULTI_CONTROLLER = 1077949113L;

    /**
     * 租户名不正确
     */
    public static final long VSTORE_ERROR = 1077951226L;

    /**
     * 连接数最大
     */
    public static final long ACCESS_COUNT_MAX = 1077949067L;

    /**
     * restFul 不支持登录
     */
    public static final long REST_UNSUPPORT_LOGIN = 1677929218L;

    /**
     * 存储token过期
     */
    public static final int TOKEN_IS_INVALID = -401;

    /**
     * 存储用户离线
     */
    public static final long THE_USER_IS_OFFLINE = 1077949069L;

    /**
     * 系统繁忙，稍后查询结果
     */
    public static final long SYSTEM_BUSY_RETRY_LATER = 16797698L;

    /**
     * 消息超时
     */
    public static final long MESSAGE_TIME_OUT = 1077949001L;
}
