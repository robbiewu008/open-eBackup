package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 创建保护请求体中资源对象DTO
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-17
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ProtectionResourceDto {
    /**
     * 资源id
     */
    private String resourceId;

    /**
     * 过滤规则列表
     */
    private List<ResourceFilterDto> filters;
}
