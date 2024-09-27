/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.backup.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * Storage Info
 *
 * @author l00853347
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-26
 */
@Data
public class StorageInfoDto {
    @JsonProperty("storage_device")
    private String storageDevice;

    @JsonProperty("storage_pool")
    private String storagePool;
}
