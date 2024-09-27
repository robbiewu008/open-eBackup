package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 环境删除provider
 * 根据resourceSubType区分
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/26
 */
public interface ProtectedEnvironmentDeleteProvider extends DataProtectionProvider<String> {
    /**
     * 删除时检查：前端调用时检查
     * 删除environment时调用
     *
     * @param env 资源
     * @return 检查结果，true：继续执行删除，false：终止删除，直接return
     */
    default boolean frontCheck(ProtectedEnvironment env) {
        return true;
    }

    /**
     * 删除时检查：通用检查
     * 删除environment时调用
     *
     * @param env 资源
     * @return 检查结果，true：继续执行删除，false：终止删除，直接return
     */
    default boolean check(ProtectedEnvironment env) {
        return true;
    }

    /**
     * 删除后处理
     *
     * @param env 资源id
     */
    default void afterDelete(ProtectedEnvironment env) {
    }
}
