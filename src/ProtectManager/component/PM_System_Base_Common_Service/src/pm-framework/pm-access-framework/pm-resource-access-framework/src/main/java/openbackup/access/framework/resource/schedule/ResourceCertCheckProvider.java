package openbackup.access.framework.resource.schedule;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.Optional;

/**
 * 资源证书check provider
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/12
 */
public interface ResourceCertCheckProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 从auth的扩展信息里获取证书
     *
     * @param protectedResource 资源
     * @return 证书内容
     */
    Optional<String> getCertContent(ProtectedResource protectedResource);

    /**
     * 从auth的扩展信息里获取吊销列表
     *
     * @param protectedResource 资源
     * @return 证吊销列表内容
     */
    Optional<String> getCrlContent(ProtectedResource protectedResource);
}
