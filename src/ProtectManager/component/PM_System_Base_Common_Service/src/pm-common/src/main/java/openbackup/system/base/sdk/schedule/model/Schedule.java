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
package openbackup.system.base.sdk.schedule.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import openbackup.system.base.common.utils.JSONObject;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Schedule
 *
 */
@Data
public class Schedule {
    @JsonProperty("schedule_type")
    private String scheduleType;

    @JsonProperty("schedule_name")
    private String scheduleName;

    private String action;

    private Object params;

    private String interval;

    @JsonProperty("start_date")
    private String startDate;

    @JsonProperty("end_date")
    private String endDate;

    @JsonProperty("day_of_week")
    private String datOfWeek;

    @JsonProperty("day_of_month")
    private String dayOfMonth;

    @JsonProperty("day_of_year")
    private String dayOfYear;

    @JsonProperty("daily_start_time")
    private String dailyStartTime;

    @JsonProperty("daily_end_time")
    private String dailyEndTime;

    @JsonProperty("replace_existing")
    private boolean isReplaceExisting = false;

    @JsonProperty("context")
    private boolean needContext = false;

    private Object task;

    /**
     * create a immediate schedule object
     *
     * @param action action topic
     * @param isNeedContext isNeedContext
     * @param params params
     * @param task task
     * @return a immediate schedule object
     */
    public static Schedule immediate(String action, boolean isNeedContext, JSONObject params, JSONObject task) {
        Schedule schedule = create("immediate", action);
        if (isNeedContext) {
            schedule.setNeedContext(true);
        }
        if (params != null) {
            schedule.setParams(params.toString());
        } else {
            schedule.setParams(null);
        }
        if (task != null) {
            schedule.setTask(task.toString());
        } else {
            schedule.setTask(null);
        }
        return schedule;
    }

    /**
     * create a interval schedule object
     *
     * @param scheduleName scheduleName
     * @param action action topic
     * @param interval interval
     * @param startDate start date
     * @param params params
     * @return a interval schedule object
     */
    public static Schedule interval(String scheduleName, String action, String interval, Date startDate,
        JSONObject params) {
        Schedule schedule = create("interval", action);
        schedule.setAction(action);
        schedule.setScheduleName(scheduleName);
        schedule.setInterval(interval);
        schedule.setStartDate(new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(startDate));
        if (params != null) {
            schedule.setParams(params.toString());
        } else {
            schedule.setParams(null);
        }
        return schedule;
    }

    /**
     * 创建定期调度任务
     *
     * @return 调度任务对象
     */
    public static Schedule interval() {
        Schedule schedule = new Schedule();
        schedule.setScheduleType("interval");
        return schedule;
    }

    private static Schedule create(String type, String action) {
        Schedule schedule = new Schedule();
        schedule.setScheduleType(type);
        schedule.setAction(action);
        return schedule;
    }
}
