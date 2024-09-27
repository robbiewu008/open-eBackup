package openbackup.data.access.framework.protection.listener.v1.replication;

import com.fasterxml.jackson.annotation.JsonProperty;
import com.fasterxml.jackson.databind.PropertyNamingStrategy;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Data;

/**
 * Replication Ext Param
 *
 * @author l00272247
 * @since 2020-11-19
 */
@Data
@JsonNaming(PropertyNamingStrategy.SnakeCaseStrategy.class)
public class ReplicationExtParam {
    private String externalSystemId;
    @JsonProperty("encryption")
    private boolean isEncryption;
}
