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
package openbackup.system.base.sdk.alarm.model;

import lombok.Data;
import lombok.EqualsAndHashCode;

import org.hibernate.validator.constraints.Range;

/**
 * Lego告警
 *
 */
@Data
public class InternalAlarm {
    private Long sequence;

    private String alarmSource;

    private String sourceType;

    private String alarmId;

    private String name;

    private String desc;

    // 告警参数，多个用“,”分隔
    private String params;

    @Range(min = 1, max = 6, message = "out of range")
    private Integer alarmType;

    private String advice;

    private String effect;

    private String location;

    @Range(min = 1, max = 4, message = "out of range")
    private Integer severity;

    @EqualsAndHashCode.Exclude
    private Long createTime;

    @Range(min = 0, max = 6, message = "out of range")
    private Integer type = 1;

    private String userId;

    private String resourceId;
}
