package openbackup.database.base.plugin.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Collection;
import java.util.Objects;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 数据备份插件通用功能,默认实现
 *
 * @author h30027154
 * @version OceanProtect X8000 1.2.1
 * @since 2022-05-25
 */
@Component
public class DefaultDbBackupInterceptor extends AbstractDbBackupInterceptor {
    private PluginConfigManager pluginConfigManager;

    public DefaultDbBackupInterceptor(PluginConfigManager pluginConfigManager) {
        this.pluginConfigManager = pluginConfigManager;
    }

    /**
     * 抽象方法实现
     *
     * @param backupTask backupTask
     * @return BackupTask
     */
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        return backupTask;
    }

    /**
     * 匹配DATABASE
     *
     * @param subType subType
     * @return boolean
     */
    @Override
    public boolean applicable(String subType) {
        Set<String> subTypeSet = pluginConfigManager.getPluginConfigs()
            .stream()
            .filter(e -> Objects.equals(e.getType(), ResourceTypeEnum.DATABASE.getType()))
            .map(PluginConfig::getSubType)
            .collect(Collectors.toSet());
        Collection<BackupInterceptorProvider> providers = providerManager.findProviders(
            BackupInterceptorProvider.class);
        for (BackupInterceptorProvider provider : providers) {
            if (Objects.equals(provider.getClass(), DefaultDbBackupInterceptor.class)) {
                continue;
            }
            // 有其他的实现，则跳过该实现
            if (provider.applicable(subType)) {
                return false;
            }
        }
        return subTypeSet.contains(subType);
    }
}
