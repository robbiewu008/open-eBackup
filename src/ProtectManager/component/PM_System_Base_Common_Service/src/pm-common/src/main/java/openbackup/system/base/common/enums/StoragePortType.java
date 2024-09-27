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

import lombok.Getter;

import java.util.Arrays;

/**
 * OceanStorageEthernetPortQuery
 *
 * @author 存储设备端口类型
 * @since 2021-05-06
 */
@Getter
public enum StoragePortType {
    MANAGEMENT_PORT(2);

    private final int type;

    StoragePortType(int type) {
        this.type = type;
    }

    /**
     * 根据端口类型获取枚举
     *
     * @param type 端口类型
     * @return StoragePortType
     */
    public static StoragePortType getByPort(int type) {
        return Arrays.stream(StoragePortType.values())
            .filter(portType -> portType.type == type)
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
