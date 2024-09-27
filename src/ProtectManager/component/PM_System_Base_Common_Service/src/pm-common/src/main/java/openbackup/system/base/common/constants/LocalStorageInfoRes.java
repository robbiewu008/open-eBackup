package openbackup.system.base.common.constants;

import lombok.Data;

/**
 * LocalStorageInfoRes
 *
 * @author p00511147
 * @since 2020-12-17
 */
@Data
public class LocalStorageInfoRes {
    private String version;

    private String mode;

    private String esn;

    private String wwn;

    private String totalCapacity;

    private String vasaAltrnateName;

    private String healthStatus;

    private String patchVersion;

    private String type = "OceanProtect A8000";

    private String location;

    private String clusterName;

    private String clusterId;
}
