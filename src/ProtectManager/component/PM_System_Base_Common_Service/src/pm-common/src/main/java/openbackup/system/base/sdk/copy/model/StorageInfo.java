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
package openbackup.system.base.sdk.copy.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 存储设备信息
 *
 */
@Data
public class StorageInfo {
    /**
     * 存储池
     */
    @JsonProperty("storage_pool")
    private String storagePool;

    /**
     * 设备Id
     */
    @JsonProperty("device_id")
    private String deviceId;

    /**
     * 设备类型
     */
    @JsonProperty("device_type")
    private String deviceType;
}