package openbackup.system.base.sdk.protection.model;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.List;

/**
 * Sla 数据模型
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.0.0]
 * @since 2020/10/9
 **/
@Data
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class SlaBo {
    /**
     * uuid
     */
    private String uuid;

    /**
     * sla name
     */
    private String name;

    /**
     * user id
     */
    private String userId;

    /**
     * is predefine sla
     */
    @JsonProperty("is_global")
    private Boolean isGlobal;

    /**
     * sla type
     */
    private String type;

    /**
     * application
     */
    private String application;

    /**
     * policy list
     */
    private List<PolicyBo> policyList;

    /**
     * resource count
     */
    private String resourceCount;

    /**
     * archival count
     */
    private String archivalCount;

    /**
     * replication count
     */
    private String replicationCount;
}
