package openbackup.system.base.config.configmap;

import lombok.Data;

/**
 * configmap 参数请求对象
 *
 * @author y30000858
 * @since 2021-10-12
 */
@Data
public class ConfigMapReq {
    private String nameSpace;

    private String configMap;

    private String configKey;

    private String configValue;
}
