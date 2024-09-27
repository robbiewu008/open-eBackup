package openbackup.system.base.common.enums;

import java.util.Arrays;
import java.util.List;

/**
 * 用户类型
 *
 * @author c00318653
 * @since 2022-12-15
 */
public enum UserTypeEnum {
    /**
     * 本地用户
     */
    COMMON("COMMON"),

    /**
     * LDAP用户
     */
    LDAP("LDAP"),

    /**
     * LDAP用户组用户
     */
    LDAPGROUP("LDAPGROUP"),

    /**
     * SAML用户
     */
    SAML("SAML"),

    /**
     * HCS
     */
    HCS("HCS"),

    /**
     * ADFS用户
     */
    ADFS("ADFS"),
    /**
     * DME用户
     */
    DME("DME");

    private final String value;

    UserTypeEnum(String value) {
        this.value = value;
    }

    public String getValue() {
        return value;
    }

    /**
     * 限制页面创建用户数量的的用户类型
     *
     * @return 用户类型集和
     */
    public static List<String> createLimitUserType() {
        return Arrays.asList(UserTypeEnum.LDAPGROUP.getValue(), UserTypeEnum.COMMON.getValue(),
            UserTypeEnum.LDAP.getValue());
    }
}
