package openbackup.db2.protection.access.enums;

import java.util.Arrays;

/**
 * db2 hadr数据库角色
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-20
 */
public enum Db2HadrRoleEnum {
    PRIMARY("PRIMARY"),

    STANDBY("STANDBY");

    Db2HadrRoleEnum(String role) {
        this.role = role;
    }

    private final String role;

    public String getRole() {
        return role;
    }

    /**
     * 根据role获取到对应的枚举
     *
     * @param role 枚举值
     * @return enum
     */
    public static Db2HadrRoleEnum getByRole(String role) {
        return Arrays.stream(Db2HadrRoleEnum.values())
            .filter(location -> location.role.equals(role))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
