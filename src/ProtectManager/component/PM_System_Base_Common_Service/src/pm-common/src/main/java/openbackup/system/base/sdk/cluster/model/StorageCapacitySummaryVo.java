package openbackup.system.base.sdk.cluster.model;

import lombok.Data;

import java.math.BigDecimal;

/**
 * 存储容量统计信息
 *
 * @author c30047317
 * @since 2023-07-18
 */
@Data
public class StorageCapacitySummaryVo {
    /**
     * 类型
     */
    private String type;

    /**
     * 总容量(单位KB)
     */
    private BigDecimal totalCapacity = BigDecimal.ZERO;

    /**
     * 已使用容量(单位KB)
     */
    private BigDecimal usedCapacity = BigDecimal.ZERO;

    /**
     * 剩余容量(单位KB)
     */
    private BigDecimal freeCapacity = BigDecimal.ZERO;
}
