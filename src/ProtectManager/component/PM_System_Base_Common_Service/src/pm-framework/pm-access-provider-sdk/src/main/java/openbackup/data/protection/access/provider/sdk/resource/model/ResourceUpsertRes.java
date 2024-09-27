/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource.model;

import lombok.Data;

/**
 * 资源upsert返回结果
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-05-18
 */
@Data
public class ResourceUpsertRes {
    /**
     * 新增资源的id列表
     */
    private String[] increaseResourceUuids;

    /**
     * 是否超过资源数量限制
     */
    private boolean isOverLimit;
}
