package openbackup.system.base.sdk.infrastructure.model.beans;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * config map key-value request
 *
 * @author xwx1016404
 * @since 2021-02-03
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class ConfigMapRequest {
    /**
     * 命名空间
     */
    private String nameSpace;

    /**
     * ConfigMap名称
     */
    private String configMap;

    /**
     * 配置键
     */
    private String configKey;

    /**
     * 配置值
     */
    private String configValue;
}
