package openbackup.data.protection.access.provider.sdk.copy;

import lombok.Data;

/**
 * Copy Data Replication Target
 *
 * @author j00364432
 * @version [BCManager 8.0.0]
 * @since 2020-06-17
 */
@Data
public class CopyDataReplicationTarget {
    String endpoint;
    String username;
    String password;
}
