package openbackup.system.base.common.enums;

import lombok.AllArgsConstructor;
import lombok.Getter;

/**
 * 认证方式枚举类
 *
 * @author swx1010572
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-28
 */
@AllArgsConstructor
@Getter
public enum AuthTypeEnum {
    /**
     * 业务认证
     */
    SERVICE_AUTH("serviceAuth", 0),

    /**
     * 管理认证
     */
    MANAGER_AUTH("managerAuth", 1),

    /**
     * 升级后无认证默认业务认证
     */
    NO_AUTH("", 0);

    /**
     * 认证登录Type
     */
    private final String loginAuthType;

    /**
     * 入库Type
     */
    private final int entryAuthType;


    /**
     * 获取获取认证入参的枚举类
     *
     * @param authType 认证登录Type
     * @return 认证方式枚举
     */
    public static AuthTypeEnum getLoginAndEntryAuthType(String authType) {
        for (AuthTypeEnum authTypeEnum : AuthTypeEnum.values()) {
            if (authTypeEnum.getLoginAuthType().equals(authType)) {
                return authTypeEnum;
            }
        }

        // 异常情况，未在设计内的密码状态码
        return AuthTypeEnum.NO_AUTH;
    }
}
