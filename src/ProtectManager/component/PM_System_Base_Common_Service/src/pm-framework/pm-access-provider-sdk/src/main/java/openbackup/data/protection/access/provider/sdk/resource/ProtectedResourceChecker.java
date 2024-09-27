package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;

import java.util.List;
import java.util.Map;

/**
 * The ResourceConnectionChecker
 *
 * @author g30003063
 * @since 2022-05-20
 */
public interface ProtectedResourceChecker<T> extends DataProtectionProvider<ProtectedResource> {
    /**
     * 获取受保护资源环境矩阵
     *
     * @param resource 需要检查的受保护资源
     * @return 需要检查的资源集合
     */
    Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(ProtectedResource resource);

    /**
     * 检查连通性
     *
     * @param protectedResource 需要检查的资源
     * @return 检查报告
     */
    CheckResult<T> generateCheckResult(ProtectedResource protectedResource);

    /**
     * 获取检查结果
     *
     * @param checkReports 检查报告
     * @param context 上下文
     * @return 检查结果
     */
    List<ActionResult> collectActionResults(List<CheckReport<T>> checkReports, Map<String, Object> context);
}