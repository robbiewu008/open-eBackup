package openbackup.system.base.sdk.user;

import java.util.List;

/**
 * 用户权限接口
 *
 * @author z00842230
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-07-11
 */
public interface AuthServiceApi {
    /**
     * 查询指定域下默认用户权限列表
     *
     * @param domainId 域id
     * @param authOperationList 权限列表
     * @return 是否存在
     */
    boolean isDefaultRoleHasAuthOperation(String domainId, List<String> authOperationList);
}
