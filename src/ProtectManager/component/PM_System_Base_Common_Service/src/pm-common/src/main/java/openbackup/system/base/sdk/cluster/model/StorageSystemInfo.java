package openbackup.system.base.sdk.cluster.model;

import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import lombok.Data;

import java.math.BigDecimal;
import java.util.List;

/**
 * Storage system info
 *
 * @author p30001902
 * @since 2020-12-17
 */
@Data
public class StorageSystemInfo {
    // storage esn
    private String storageEsn;

    // storage work port
    private int storagePort;

    private String username;

    private String password;

    // 集群已使用容量 kb
    private BigDecimal usedCapacity;

    // 集群容量 kb
    private BigDecimal capacity;

    private List<NodePodInfo> netplaneInfo;

    private DeviceNetworkInfo deviceNetworkInfo;
}
