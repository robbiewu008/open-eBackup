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
 * CloudCheckTypeEnum
 *
 */
@Getter
public enum CloudCheckTypeEnum {
    /**
     * 定时任务检查对象存储连通性
     */
    CLOUD_STORAGE_SCHEDULE_CHECK(0),

    /**
     * 添加或修改时检查对象存储连通性
     */
    CLOUD_STORAGE_UPSERT_CHECK(1);

    private final Integer type;

    CloudCheckTypeEnum(Integer type) {
        this.type = type;
    }
}
