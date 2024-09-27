package openbackup.data.protection.access.provider.sdk.plugin;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.Data;

import java.util.Map;

/**
 * 插件配置类
 *
 * @since 2022-05-20
 */
@Data
public class PluginConfig {
    private String type;

    private String subType;

    /**
     * 配置Map，通用结构
     * key：顶层key
     * value: 对应值
     */
    private Map<String, JsonNode> configMap;
}
