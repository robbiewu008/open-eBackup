package openbackup.data.access.framework.protection.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 副本归档请求体
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-07
 */
@Data
public class ArchiveRequestDto {
    // 副本Id
    @JsonProperty("copy_id")
    private String copyId;

    // 保护策略
    @JsonProperty("policy")
    private String policy;

    // 副本所对应保护对象的类型，如Oracle
    @JsonProperty("resource_sub_type")
    private String resourceSubType;

    @JsonProperty("resource_type")
    private String resourceType;

    @JsonProperty("sla_name")
    private String slaName;
}
