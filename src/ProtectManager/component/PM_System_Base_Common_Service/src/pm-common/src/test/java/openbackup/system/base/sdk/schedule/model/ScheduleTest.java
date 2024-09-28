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

import openbackup.system.base.sdk.schedule.model.Schedule;

import org.junit.Assert;
import org.junit.Test;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Schedule test
 *
 */
public class ScheduleTest {

    public static final String JOB_MONITOR_START = "schedule.job.monitor.start";

    @Test
    public void interval() {
        Schedule interval = Schedule.interval();
        Assert.assertNotNull(interval);
    }

    @Test
    public void Returninterval() {
        Schedule schedule = getSchedule();
        Schedule.interval(schedule.getScheduleName(),
                schedule.getAction(), schedule.getInterval(), new Date(), null);
    }

    public static Schedule getSchedule() {
        Schedule schedule = new Schedule();
        schedule.setScheduleType("interval");
        schedule.setAction(JOB_MONITOR_START);
        schedule.setStartDate(new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(new Date()));
        schedule.setParams(null);

        return schedule;
    }
}
