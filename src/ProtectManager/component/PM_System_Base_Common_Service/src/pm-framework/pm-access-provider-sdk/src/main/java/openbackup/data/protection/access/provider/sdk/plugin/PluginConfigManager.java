package openbackup.data.protection.access.provider.sdk.plugin;

import java.util.List;
import java.util.Optional;

/**
 * 插件配置管理
 *
 * @since 2022-05-20
 */
public interface PluginConfigManager {
    /**
     * 初始化
     */
    void init();

    /**
     * 获取配置
     *
     * @return 配置
     */
    List<PluginConfig> getPluginConfigs();

    /**
     * 根据subtype获取配置
     *
     * @param subType subType
     * @return 配置
     */
    Optional<PluginConfig> getPluginConfig(String subType);
}
