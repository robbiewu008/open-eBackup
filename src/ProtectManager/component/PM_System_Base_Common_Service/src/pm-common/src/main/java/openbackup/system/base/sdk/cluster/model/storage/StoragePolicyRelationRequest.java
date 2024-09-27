package openbackup.system.base.sdk.cluster.model.storage;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

/**
 * 策略与存储单元关联关系
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-03-20
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class StoragePolicyRelationRequest {
    @Length(max = 256)
    private String storageId;

    @Length(max = 256)
    private String remoteEsn;

    @Length(max = 64)
    private String policyName;

    @Length(max = 64)
    private String storageName;
}
