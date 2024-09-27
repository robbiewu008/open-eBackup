/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.client.sdk.api.framework.dme;

import lombok.Data;

/**
 * 存储快照信息，用于存放副本在存储（NAS文件系统的快照信息）
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-08
 */
@Data
public class StorageSnapshot {
    // 快照ID
    private String id;

    // 快照的父对象名称，如果是文件系统则为快照所属文件系统的名字
    private String parentName;
}
