package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.database.base.plugin.provider.DatabaseResourceProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * GaussDBDWS DWS资源相关接口的具体实现类
 * 实现了：健康状态检查，环境信息检查相关等接口
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.5.0]
 * @since 2023-02-22
 */
@Slf4j
@Component
public class GaussDBDWSClusterResourceProvider extends DatabaseResourceProvider {
    /**
     * Constructor
     *
     * @param providerManager providerManager
     * @param pluginConfigManager pluginConfigManager
     */
    public GaussDBDWSClusterResourceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager) {
        super(providerManager, pluginConfigManager);
    }

    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.GAUSSDB_DWS.getType().equals(protectedResource.getSubType());
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }
}
