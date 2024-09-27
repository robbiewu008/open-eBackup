package openbackup.data.access.framework.agent;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;
import java.util.Map;

/**
 * 保护代理提供者接口定义，通过该接口
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-26
 */
public interface ProtectAgentSelector extends DataProtectionProvider<String> {
    /**
     * 选择备份/备份恢复代理
     *
     * @param protectedResource 备份/恢复资源{@link ProtectedResource}
     * @param parameters 备份恢复参数
     * @return 返回备份/恢复代理
     */
    List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters);
}
