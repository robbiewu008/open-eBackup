package openbackup.data.protection.access.provider.sdk.sla;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

import java.util.List;

/**
 * SLA实体类
 *
 * @author y00559272
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-19
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class Sla {
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
    private List<Policy> policyList;

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
