/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.system.base.sdk.cluster.model;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 根据用户ID查询授权存储单元（组）响应VO对象
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-02
 */
@Getter
@Setter
@NoArgsConstructor
@AllArgsConstructor
public class StorageUserAuthRelationStorageVo {
    private String storageId;

    private String storageName;

    private Integer authType;

    private Integer generatedType;

    private boolean hasEnableParallelStorage;

    private String storageType;
}
