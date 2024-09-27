package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.data.protection.access.provider.sdk.base.Parameter;
import openbackup.data.protection.access.provider.sdk.sla.Policy;
import openbackup.data.protection.access.provider.sdk.sla.Sla;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.List;

/**
 * Backup Object
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-22
 */
@Data
public class BackupObject {
    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("task_id")
    private String taskId;

    @JsonProperty("chain_id")
    private String chainId;

    @JsonProperty("backup_type")
    private String backupType;

    @JsonProperty("protected_object")
    private ProtectedObject protectedObject;

    @JsonProperty("sla")
    private Sla sla;

    private Repository repository;

    private List<Parameter> parameters;

    private Policy policy;
}
