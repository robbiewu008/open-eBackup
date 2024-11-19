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
package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

import java.util.Objects;

/**
 * 防勒索调度计划
 *
 */
@Data
public class AntiRansomwareScheduleRes {
    // 间隔时间
    private Integer interval;

    // 间隔时间单位
    private String intervalUnit;

    // 调度计划
    private String schedulePolicy;

    // 仅检测该时间后的的副本
    private String copyTime;

    // 首次检测时间
    private String startDetectionTime;

    // 检测类型
    private Integer detectionType;

    // 是否设置防篡改
    @JsonProperty("setWorm")
    private boolean isSetWorm;

    /**
     * 是否进行防勒索检测
     */
    @JsonProperty("needDetect")
    private boolean isNeedDetect;

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }
        if (obj == null || getClass() != obj.getClass()) {
            return false;
        }
        AntiRansomwareScheduleRes that = null;
        if (obj instanceof AntiRansomwareScheduleRes) {
            that = (AntiRansomwareScheduleRes) obj;
        }

        if (!Objects.equals(interval, that.interval)) {
            return false;
        }
        if (!Objects.equals(intervalUnit, that.intervalUnit)) {
            return false;
        }
        if (!Objects.equals(schedulePolicy, that.schedulePolicy)) {
            return false;
        }
        if (!Objects.equals(copyTime, that.copyTime)) {
            return false;
        }
        if (!Objects.equals(startDetectionTime, that.startDetectionTime)) {
            return false;
        }
        return Objects.equals(detectionType, that.detectionType);
    }

    @Override
    public int hashCode() {
        return Objects.hash(interval, intervalUnit, schedulePolicy, copyTime, startDetectionTime, detectionType);
    }
}