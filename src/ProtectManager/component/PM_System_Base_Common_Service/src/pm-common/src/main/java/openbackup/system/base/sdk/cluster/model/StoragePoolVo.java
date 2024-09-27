package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.math.BigDecimal;

/**
 * 存储池VO
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class StoragePoolVo {
    private String id;

    private String deviceId;

    private String poolId;

    private String name;

    private Integer healthStatus;

    private Integer runningStatus;

    private BigDecimal totalCapacity;

    private BigDecimal usedCapacity;

    private String threshold;

    private double spaceReductionRate;
}
