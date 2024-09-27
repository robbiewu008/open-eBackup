package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

import java.util.List;

/**
 * 调用Agent返回的插件信息主数据模型，包括主机uuid和支持的插件列表
 *
 * @author w00616953
 * @since 2021-11-30
 */
@Data
public class PluginsDto {
    /**
     * 主机uuid
     */
    private String uuid;

    /**
     * 支持的插件列表
     */
    private List<SupportPluginDto> supportPlugins;
}
