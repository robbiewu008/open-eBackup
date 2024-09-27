package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Builder;
import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * 创建保护请求体
 *
 * @author w30044259
 * @since 2024-03-06
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
@Builder
public class ModifyProtectionObjectSubmitReq {
    /**
     * SLA id
     */
    private String slaId;

    /**
     * 资源id
     */
    private String resourceId;

    /**
     * 是否为资源组
     */
    private Boolean isResourceGroup;

    /**
     * 是否为资源组子资源
     */
    private Boolean isGroupSubResource;

    /**
     * 扩展属性
     */
    private Map<String, Object> extParameters;
}
