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
package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 策略调度周期枚举定义
 *
 */
public enum ScheduledType {
    /**
     * 按时间间隔
     */
    PERIOD_SCHEDULE("period_schedule"),

    /**
     * 按副本生成后执行
     */
    AFTER_BACKUP_DONE("after_backup_done");

    private final String name;

    ScheduledType(String name) {
        this.name = name;
    }

    @JsonValue

    public String getName() {
        return name;
    }

    /**
     * get Scheduled type enum
     *
     * @param str str
     * @return RetentionUnit
     */
    public static ScheduledType get(String str) {
        return EnumUtil.get(ScheduledType.class, ScheduledType::getName, str);
    }
}
