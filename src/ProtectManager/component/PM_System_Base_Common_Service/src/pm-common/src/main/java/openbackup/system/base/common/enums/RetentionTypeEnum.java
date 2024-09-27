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
 * 副本保留类型枚举类
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.0.0]
 * @since: 2020/10/9
 **/
@Getter
public enum RetentionTypeEnum {
    PERMANENT(1),
    TEMPORARY(2),
    QUANTITY(3);

    private final Integer type;

    RetentionTypeEnum(Integer type) {
        this.type = type;
    }

    /**
     * 根据类型获取保留类型枚举类
     *
     * @param type 类型
     * @return RetentionTypeEnum
     */
    public static RetentionTypeEnum getByType(int type) {
        return Arrays.stream(RetentionTypeEnum.values())
            .filter(retention -> retention.type - type == 0)
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
