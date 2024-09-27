package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 保护批量操作请求体
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-19
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ProtectionBatchOperationReq {
    /**
     * 资源id列表
     */
    private List<String> resourceIds;

    /**
     * 资源id列表
     */
    private Boolean isResourceGroup;
}
