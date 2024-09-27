/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.enums;

/**
 * 对象存储类型
 *
 * @author w00639094
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-06
 */
public enum StorageTypeEnum {
    /**
     * Ocean Storage Pacific
     */
    OCEAN_STORAGE_PACIFIC(0),

    /**
     * FusionStorage OBS
     */
    FUSION_STORAGE_OBS(1),

    /**
     * 华为云 OBS
     */
    HW_OBS(3),

    /**
     *  AWS S3
     */
    AWS(4),

    /**
     * Azure Blob
     */
    AZURE_BLOB(5);

    private final int storageType;

    /**
     * 构造函数
     *
     * @param storageType storageType
     */
    StorageTypeEnum(int storageType) {
        this.storageType = storageType;
    }

    /**
     * get storage type
     *
     * @return storage type
     */
    public int getStorageType() {
        return storageType;
    }
}
