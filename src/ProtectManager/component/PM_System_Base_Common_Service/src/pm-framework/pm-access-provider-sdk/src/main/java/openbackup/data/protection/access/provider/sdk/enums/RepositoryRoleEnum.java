package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 存储库角色 枚举类
 *
 * @author: swx1010572
 * @version: [OceanProtect X8000 2.1.0]
 * @since: 2022/07/13
 **/
public enum RepositoryRoleEnum {
    /**
     *  repository role 角色 master
     */
    MASTER_ROLE(0),
    /**
     * repository role 角色 slave
     */
    SLAVE_ROLE(1),

    /**
     * repository role 角色 archive
     */
    ARCHIE_ROLE(2);

    private final int roleType;

    RepositoryRoleEnum(int roleType) {
        this.roleType = roleType;
    }

    public int getRoleType() {
        return roleType;
    }
}
