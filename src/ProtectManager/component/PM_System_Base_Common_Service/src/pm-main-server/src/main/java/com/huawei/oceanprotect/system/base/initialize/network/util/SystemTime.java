/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.util;

import com.huawei.oceanprotect.system.base.user.common.utils.CurrentSystemTime;

import openbackup.system.base.common.constants.LegoNumberConstant;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * 记录时间
 *
 * @author swx1010572
 * @since 2021-04-08
 */
public class SystemTime {
    /**
     * 总时间
     */
    public static final String ALL_TIME = "all_time";

    /**
     * 单一流程时间
     */
    public static final String PART_TIME = "part_time";

    private static final double DEFAULT_SECONDS = 1000.0d;

    private long allStart;

    private long allEnd;

    private long start;

    private long end;

    /**
     * 结束时间查询
     *
     * @param time 是否所有时间参数
     * @return 时间
     */
    public CurrentSystemTime getEndDateTime(String time) {
        TimeZone.setDefault(null);
        System.setProperty("user.timezone", "");
        CurrentSystemTime systemTime = new CurrentSystemTime();
        Date date = new Date();
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        DateFormat timeZoneDateFormat = new SimpleDateFormat("Z");
        systemTime.setTime(dateFormat.format(date));
        StringBuilder idString = new StringBuilder("GMT" + timeZoneDateFormat.format(date));
        idString.insert(LegoNumberConstant.SIX, ":");
        systemTime.setId(idString.toString());
        dateFormat.setTimeZone(TimeZone.getTimeZone(TimeZone.getDefault().getID()));
        TimeZone defaultTime = TimeZone.getDefault();
        if (ALL_TIME.equals(time)) {
            allEnd = date.getTime();
        } else {
            end = date.getTime();
        }
        systemTime.setOffset(defaultTime.getOffset(date.getTime()));
        Calendar calendar = Calendar.getInstance();
        if (calendar.get(Calendar.DST_OFFSET) == 0) {
            systemTime.setDstSavings(0);
            systemTime.setUseDaylight(false);
        } else {
            systemTime.setDstSavings(LegoNumberConstant.ONE);
            systemTime.setUseDaylight(true);
        }
        String id = timeZoneDateFormat.format(new Date());
        StringBuilder nameString = new StringBuilder("UTC" + id);
        nameString.insert(LegoNumberConstant.SIX, ":");
        systemTime.setDisplayName(nameString.toString());
        return systemTime;
    }

    /**
     * 结束时间查询
     *
     * @param time 是否所有时间参数
     * @return 时间
     */
    public CurrentSystemTime getStartDateTime(String time) {
        TimeZone.setDefault(null);
        System.setProperty("user.timezone", "");
        CurrentSystemTime systemTime = new CurrentSystemTime();
        Date date = new Date();
        SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
        DateFormat timeZoneDateFormat = new SimpleDateFormat("Z");
        systemTime.setTime(dateFormat.format(date));
        StringBuilder idString = new StringBuilder("GMT" + timeZoneDateFormat.format(date));
        idString.insert(LegoNumberConstant.SIX, ":");
        systemTime.setId(idString.toString());
        dateFormat.setTimeZone(TimeZone.getTimeZone(TimeZone.getDefault().getID()));
        TimeZone defaultTime = TimeZone.getDefault();
        if (ALL_TIME.equals(time)) {
            allStart = date.getTime();
        } else {
            start = date.getTime();
        }
        systemTime.setOffset(defaultTime.getOffset(date.getTime()));
        Calendar calendar = Calendar.getInstance();
        if (calendar.get(Calendar.DST_OFFSET) == 0) {
            systemTime.setDstSavings(0);
            systemTime.setUseDaylight(false);
        } else {
            systemTime.setDstSavings(LegoNumberConstant.ONE);
            systemTime.setUseDaylight(true);
        }
        String id = timeZoneDateFormat.format(new Date());
        StringBuilder nameString = new StringBuilder("UTC" + id);
        nameString.insert(LegoNumberConstant.SIX, ":");
        systemTime.setDisplayName(nameString.toString());
        return systemTime;
    }

    /**
     * 所有时间
     *
     * @return time
     */
    public double updateDateTime() {
        return (end - start) / DEFAULT_SECONDS;
    }

    /**
     * 单个模块时间
     *
     * @return time
     */
    public double allUpdateDateTime() {
        return (allEnd - allStart) / DEFAULT_SECONDS;
    }
}
