/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 存储单元接口响应返回类
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
@Setter
@Getter
@AllArgsConstructor
@NoArgsConstructor
public class StorageUnitInfo {
    private String id;

    private String storageUnitName;

    private String storageType;

    private String poolName;

    private String totalCapacity;

    private String usedCapacity;

    private String threshold;

    private Integer healthStatus;

    private Integer runningStatus;

    private String clusterName;
}
