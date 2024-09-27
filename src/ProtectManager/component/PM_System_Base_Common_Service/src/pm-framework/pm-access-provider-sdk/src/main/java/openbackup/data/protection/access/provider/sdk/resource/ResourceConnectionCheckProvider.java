package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;

/**
 * 资源连通性检查provider
 *
 * @author g30003063
 * @since 2022-05-20
 */
public interface ResourceConnectionCheckProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 检查资源连通性
     * 该方法不会抛出异常，需要对返回结果进行检查
     *
     * @param protectedResource 受保护资源
     * @param protectedResourceChecker 检查checker
     * @return 检查结果
     */
    ResourceCheckContext tryCheckConnection(ProtectedResource protectedResource,
        ProtectedResourceChecker protectedResourceChecker);

    /**
     * 检查资源连通性
     * 该方法不会抛出异常，需要对返回结果进行检查
     *
     * @param protectedResource 受保护资源
     * @return 检查结果
     */
    default ResourceCheckContext tryCheckConnection(ProtectedResource protectedResource) {
        return tryCheckConnection(protectedResource, null);
    }

    /**
     * 检查资源连通性
     *
     * @param protectedResource 受保护资源
     * @return 检查结果
     */
    default ResourceCheckContext checkConnection(ProtectedResource protectedResource) {
        ResourceCheckContext context = tryCheckConnection(protectedResource, null);
        ResourceCheckContextUtil.check(context, "check connection failed.");
        return context;
    }
}