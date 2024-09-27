package openbackup.system.base.sdk.repository.model;

import openbackup.system.base.sdk.cluster.model.StorageUnitVo;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import org.springframework.beans.BeanUtils;

import java.math.BigDecimal;

/**
 * 存储单元信息
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-29
 */
@Setter
@Getter
@AllArgsConstructor
@NoArgsConstructor
public class BackupUnitVo {
    private String unitId;

    private String unitName;

    // 备份存储单元在指定策略下的顺序
    private Integer strategyOrder;

    // 可使用容量阀值 0~100%
    private Integer availableCapacityRatio;

    private String deviceId;

    private String poolId;

    private String deviceType;

    private String poolName;

    private BigDecimal totalCapacity;

    private BigDecimal usedCapacity;

    private String threshold;

    private Integer healthStatus;

    private Integer runningStatus;

    private String deviceName;

    private boolean isAutoAdded;

    private BackupClusterVo backupClusterVo;

    /**
     * 构造函数
     *
     * @param storageUnitVo StorageUnitVo
     */
    public BackupUnitVo(StorageUnitVo storageUnitVo) {
        BeanUtils.copyProperties(storageUnitVo, this);
        this.unitId = storageUnitVo.getId();
        this.unitName = storageUnitVo.getName();
    }

    /**
     * 映射成备份存储单元视图对象
     *
     * @return 备份存储单元视图
     */
    public StorageUnitVo toStorageUnitVo() {
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        BeanUtils.copyProperties(this, storageUnitVo);
        storageUnitVo.setId(this.unitId);
        storageUnitVo.setName(this.unitName);
        return storageUnitVo;
    }
}
