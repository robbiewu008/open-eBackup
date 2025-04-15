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

import lombok.AllArgsConstructor;
import lombok.Getter;

import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 设备类型
 *
 */
@Getter
@AllArgsConstructor
public enum StorageDeviceTypeEnum {
    DORADO("dorado"),

    PACIFIC("pacific"),

    /**
     * 数据保护存储
     */
    OCEAN_PROTECT("OceanProtect"),

    /**
     * 数据保护存储，适配E1000部署形态新增的，实际和OceanProtect无区别
     */
    OCEAN_PROTECT_X("OceanProtectX"),

    /**
     * 安全一体机机OP
     */
    CYBER_ENGINE_OCEAN_PROTECT("CyberEngineOceanProtect"),

    /**
     * 安全一体机机Dorado
     */
    CYBER_ENGINE_DORADO_V6("CyberEngineDoradoV6"),

    /**
     * E1000本地盘
     */
    DEVICE_TYPE_BASIC_DISK("BasicDisk");

    private String type;

    /**
     * 获取存储设备类型Map
     */
    public static final Map<String, StorageDeviceTypeEnum> storageDeviceTypeMap = Stream.of(values())
        .collect(Collectors.toMap(StorageDeviceTypeEnum::getType, e -> e));

    /**
     * 获取类型对应枚举类
     *
     * @param type 设备类型
     * @return 设备类型枚举
     */
    public static StorageDeviceTypeEnum getByType(String type) {
        return storageDeviceTypeMap.get(type);
    }
}