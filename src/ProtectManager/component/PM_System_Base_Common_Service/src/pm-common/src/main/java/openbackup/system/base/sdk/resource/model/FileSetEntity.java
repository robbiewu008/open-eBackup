package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;

import java.util.List;

/**
 * FileSet Entity
 *
 * @author l00272247
 * @since 2020-07-14
 */
@EqualsAndHashCode(callSuper = true)
@Data
public class FileSetEntity extends ResourceEntity {
    @JsonProperty("sla_id")
    private String slaId;

    @JsonProperty("sla_name")
    private String slaName;

    @JsonProperty("sla_status")
    private String slaStatus;

    @JsonProperty("sla_compliance")
    private String slaCompliance;

    // 文件集中包含的全路径
    private List<String> paths;

    // 四种过滤方式
    private List<Filter> filters;
}
