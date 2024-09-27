/*
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.resource.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 磁盘和存储位置的对应关系
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023/12/2
 */
@Getter
@Setter
@AllArgsConstructor
@NoArgsConstructor
public class DiskStorageLocation {
    private String diskId;
    private String storageName;
    private String diskType;
    private String storageIp;
}
