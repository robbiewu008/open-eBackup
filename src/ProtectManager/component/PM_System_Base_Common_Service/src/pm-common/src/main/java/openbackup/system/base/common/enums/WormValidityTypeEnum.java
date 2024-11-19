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

/**
 * 周期类型时间枚举定义
 *
 **/
@Getter
public enum WormValidityTypeEnum {
    /**
     * 不开启WORM
     */
    WORM_NOT_OPEN(0),

    /**
     * 同副本保留时间一致
     */
    COPY_RETENTION_TIME_CONSISTENT(1),

    /**
     * 自定义WORM过期时间
     */
    CUSTOM_RETENTION_TIME(2);

    private final Integer type;

    WormValidityTypeEnum(Integer type) {
        this.type = type;
    }
}
