package openbackup.data.protection.access.provider.sdk.plugin;

import lombok.Data;

/**
 *  收集的配置
 *
 * @since 2022-05-23
 */
@Data
public class CollectableConfig {
    private String resource;

    private String environment;
}
