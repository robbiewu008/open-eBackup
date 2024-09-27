package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

/**
 * 功能描述: 定时检查环境状态接口定义
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-16
 */
public interface EnvironmentHealthCheckProvider extends DataProtectionProvider<ProtectedEnvironment> {
    /**
     * 受保护环境健康状态检查，
     * 状态异常抛出com.huawei.oceanprotect.data.protection.access.provider.sdk.exception.DataProtectionAccessException，
     * 并返回具体的错误码
     *
     * @param environment 受保护环境
     */
    void healthCheck(ProtectedEnvironment environment);
}