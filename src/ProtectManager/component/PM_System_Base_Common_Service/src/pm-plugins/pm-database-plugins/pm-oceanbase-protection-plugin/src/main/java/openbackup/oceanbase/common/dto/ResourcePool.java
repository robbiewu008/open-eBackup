package openbackup.oceanbase.common.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-26
 */
@Data
public class ResourcePool {
    @JsonProperty("resource_pool_id")
    private String resourcePoolId;

    @JsonProperty("resource_pool_name")
    private String resourcePoolName;
}
