/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
