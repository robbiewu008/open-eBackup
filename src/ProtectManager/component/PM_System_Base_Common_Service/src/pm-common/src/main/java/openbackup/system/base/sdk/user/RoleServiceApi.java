package openbackup.system.base.sdk.user;

import openbackup.system.base.sdk.user.model.RolePo;

/**
 * 角色sdk接口
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-07-04
 */
public interface RoleServiceApi {
    /**
     * 通过用户id查询用户的默认角色
     *
     * @param userId 用户id
     * @return 默认角色id
     */
    RolePo getDefaultRolePoByUserId(String userId);
}
