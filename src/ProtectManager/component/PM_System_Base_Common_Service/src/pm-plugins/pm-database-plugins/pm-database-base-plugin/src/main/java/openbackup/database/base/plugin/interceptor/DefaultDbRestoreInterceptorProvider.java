package openbackup.database.base.plugin.interceptor;

import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;

import java.util.Collection;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 数据库恢复通用功能,默认实现
 *
 * @author h30027154
 * @version OceanProtect X8000 1.2.1
 * @since 2022-06-15
 */
public class DefaultDbRestoreInterceptorProvider extends AbstractDbRestoreInterceptorProvider {
    private PluginConfigManager pluginConfigManager;

    public DefaultDbRestoreInterceptorProvider(PluginConfigManager pluginConfigManager) {
        this.pluginConfigManager = pluginConfigManager;
    }

    @Override
    public boolean applicable(String subType) {
        Set<String> subTypeSet = pluginConfigManager.getPluginConfigs()
            .stream()
            .map(PluginConfig::getSubType)
            .collect(Collectors.toSet());
        Collection<RestoreInterceptorProvider> providers = providerManager.findProviders(
            RestoreInterceptorProvider.class);
        for (RestoreInterceptorProvider provider : providers) {
            if (Objects.equals(provider.getClass(), DefaultDbRestoreInterceptorProvider.class)) {
                continue;
            }
            // 有其他的实现，则跳过该实现
            if (provider.applicable(subType)) {
                return false;
            }
        }
        return subTypeSet.contains(subType);
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        return task;
    }
}
