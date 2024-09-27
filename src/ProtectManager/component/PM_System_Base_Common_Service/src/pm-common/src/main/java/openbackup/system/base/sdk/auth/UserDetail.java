package openbackup.system.base.sdk.auth;

import lombok.Data;

import java.util.Set;

/**
 * UserDetail from rest request
 *
 * @author dwx1009286
 * @version [OceanProtect 1.1.0]
 * @since 2022-02-24
 */
@Data
public class UserDetail {
    // 用户id
    private String userId;

    // 用户名
    private String userName;

    // 用户角色
    private Set<RoleInfo> rolesSet;

    // 用户类型
    private String userType;
}
