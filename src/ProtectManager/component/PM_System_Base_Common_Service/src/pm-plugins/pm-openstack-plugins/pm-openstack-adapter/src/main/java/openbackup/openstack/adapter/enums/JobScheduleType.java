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
package openbackup.openstack.adapter.enums;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

import java.util.stream.Stream;

/**
 * 云核OpenStack备份任务调度类型
 *
 */
public enum JobScheduleType {
    /**
     * 每隔N天在指定时间执行
     */
    DAYS("days"),
    /**
     * 每周指定某天在指定时间执行
     */
    WEEKS("weeks");

    private final String type;

    JobScheduleType(String type) {
        this.type = type;
    }

    @JsonValue
    public String getType() {
        return type;
    }

    /**
     * 根据type获取JobScheduleType枚举实例
     *
     * @param type type
     * @return JobScheduleType
     */
    @JsonCreator
    public static JobScheduleType create(String type) {
        return Stream.of(JobScheduleType.values())
                .filter(scheduleType -> scheduleType.type.equals(type))
                .findFirst()
                .orElseThrow(() -> new IllegalArgumentException("Schedule type is illegal."));
    }
}
