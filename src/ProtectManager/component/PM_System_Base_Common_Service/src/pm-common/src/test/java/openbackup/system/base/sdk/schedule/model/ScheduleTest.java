package openbackup.system.base.sdk.schedule.model;

import openbackup.system.base.sdk.schedule.model.Schedule;

import org.junit.Assert;
import org.junit.Test;

import java.text.SimpleDateFormat;
import java.util.Date;

/**
 * Schedule test
 *
 * @author jwx701567
 * @since 2021-03-17
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
