package openbackup.gaussdbt.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * GaussDBT资源更新Provider
 *
 * @author dwx1009286
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-13
 */
@Slf4j
@Component
public class GaussDBTResourceProvider extends DatabaseResourceProvider {
    /**
     * GaussDBTResourceProvider
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     */
    public GaussDBTResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager) {
        super(providerManager, pluginConfigManager);
    }

    /**
     * 更新GaussDBT资源，EnvironmentProvider中做连通性检查，不使用数据库框架再做校验
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        log.info("GaussDBT healthCheck and update GaussDBT environment nodes info.environment:{}", resource.getUuid());
    }

    @Override
    public boolean applicable(ProtectedResource resource) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(resource.getSubType());
    }
}
