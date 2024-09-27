package openbackup.data.protection.access.provider.sdk.resource.model;

/**
 * resource extend info的内置key
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-29
 */
public class ResourceExtendInfoKeyConstants {
    /**
     * 受信key，表示该资源是否守信
     * 用于主机
     *
     * 取值 "true" "false"
     */
    public static final String TRUSTWORTHINESS = "trustworthiness";

    /**
     * agent扩展信息中区分内外置类型的字段
     */
    public static final String EXT_INFO_SCENARIO = "scenario";

    /**
     * agent注册用户id
     */
    public static final String REGISTER_USER_ID = "register_user_id";

    /**
     * 管理数据恢复内置agent注册版本
     */
    public static final String RECOVER_REGISTER_VERSION = "recover_register_version";

    /**
     * 管理数据恢复内置agent注册版本
     */
    public static final String SYSTEM_RECOVER_VERSION = "system_recover_version";
}
