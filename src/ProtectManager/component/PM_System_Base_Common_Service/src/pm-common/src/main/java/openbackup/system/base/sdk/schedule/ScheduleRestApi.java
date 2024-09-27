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
package openbackup.system.base.sdk.schedule;

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.schedule.model.Schedule;
import openbackup.system.base.sdk.schedule.model.ScheduleResponse;

import java.util.Date;

/**
 * Schedule Rest Api
 *
 * @author l00272247
 * @since 2020-09-21
 */
public interface ScheduleRestApi {
    /**
     * create schedule
     *
     * @param schedule schedule
     * @return schedule response
     */
    ScheduleResponse createSchedule(Schedule schedule);

    /**
     * create immediate schedule, context default false
     *
     * @param action action
     * @param params params
     * @param task   task
     * @return schedule response
     */
    default ScheduleResponse createImmediateSchedule(String action, JSONObject params, JSONObject task) {
        return createSchedule(Schedule.immediate(action, false, params, task));
    }

    /**
     * create immediate schedule
     *
     * @param action      action
     * @param isNeedContext isNeedContext
     * @param params      params
     * @param task        task
     * @return schedule response
     */
    default ScheduleResponse createImmediateSchedule(
            String action, boolean isNeedContext, JSONObject params, JSONObject task) {
        return createSchedule(Schedule.immediate(action, isNeedContext, params, task));
    }

    /**
     * create interval schedule
     *
     * @param action    action
     * @param interval  interval
     * @param startDate start date
     * @param params    params
     * @return schedule response
     */
    default ScheduleResponse createIntervalSchedule(String action, String interval, Date startDate, JSONObject params) {
        return createSchedule(Schedule.interval(null, action, interval, startDate, params));
    }

    /**
     * create interval schedule
     *
     * @param scheduleName scheduleName
     * @param action action
     * @param interval interval
     * @param startDate start date
     * @param params params
     * @return schedule response
     */
    default ScheduleResponse createIntervalSchedule(
            String scheduleName, String action, String interval, Date startDate, JSONObject params) {
        return createSchedule(Schedule.interval(scheduleName, action, interval, startDate, params));
    }

    /**
     * delete schedule by id
     *
     * @param scheduleId schedule id
     */
    void deleteSchedule(String scheduleId);
}
