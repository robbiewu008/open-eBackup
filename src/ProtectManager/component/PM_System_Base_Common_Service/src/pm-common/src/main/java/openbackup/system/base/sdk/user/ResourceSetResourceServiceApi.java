package openbackup.system.base.sdk.user;

import java.util.List;

/**
 * 资源集-资源接口
 *
 * @author z00842230
 * @since 2024-07-11
 */
public interface ResourceSetResourceServiceApi {
    /**
     * 检查用户对资源是否有对应的操作权限
     *
     * @param domainId 域id
     * @param authOperationList 操作权限集合
     * @param resourceObjectId 资源id
     * @param type 资源类型
     * @return 检查结果
     */
    boolean checkHasResourceOperation(String domainId, List<String> authOperationList,
        String resourceObjectId, String type);
}
