package openbackup.data.protection.access.provider.sdk.plugin;

import lombok.Data;

/**
 * 资源配置扩展上下文
 *
 * @author h30027154
 * @since 2022-05-30
 */
@Data
public abstract class PluginExtensionInvokeContext<T, R> {
    private T params;

    private R result;
}
