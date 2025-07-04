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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.enums.TimeUnitEnum;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * 时间处理类
 *
 */
public final class TimeUtil {
    private static final String YYYY_MM_DD_HH_MM_SS_UTC_Z = "yyyy-MM-dd HH:mm:ss 'UTC'Z";

    // 分钟换为毫秒
    private static final int MILLISECOND = 60000;

    private TimeUtil() {
    }

    /**
     * 得到当前时间N分钟以后的时间
     *
     * @param minutes 分钟
     * @return Date 日期
     */
    public static Date getAfterTime(int minutes) {
        Date date = new Date();
        // 超时时间设置为60秒
        long time = date.getTime() + (long) minutes * MILLISECOND;
        date.setTime(time);
        return date;
    }

    /**
     * 得到当前时间
     *
     * @return String [返回类型说明]
     */
    public static String getTimeStringFormat() {
        SimpleDateFormat format = new SimpleDateFormat(YYYY_MM_DD_HH_MM_SS_UTC_Z);
        return format.format(new Date());
    }

    /**
     * 计算过期时间
     *
     * @param time date
     * @param durationUnit 保留时间单位
     * @param duration 保留时间
     * @return Date 副本过期日期
     */
    public static Date computeExpirationTime(long time, TimeUnitEnum durationUnit, Integer duration) {
        Calendar calendar = Calendar.getInstance();
        calendar.setTimeInMillis(time);
        switch (durationUnit) {
            case MINUTES:
                calendar.add(Calendar.MINUTE, duration);
                break;
            case HOURS:
                calendar.add(Calendar.HOUR, duration);
                break;
            case DAYS:
                calendar.add(Calendar.HOUR, duration * 24);
                break;
            case WEEKS:
                calendar.add(Calendar.HOUR, duration * 7 * 24);
                break;
            case MONTHS:
                calendar.add(Calendar.MONTH, duration);
                break;
            case YEARS:
                calendar.add(Calendar.YEAR, duration);
                break;
            default:
        }
        return calendar.getTime();
    }

    /**
     * 获取系统默认的时区信息
     * 如 UTC-11:00
     *
     * @return String [返回类型说明]
     */
    public static String getDefaultTimeZone() {
        TimeZone defaultTimezone = TimeZone.getDefault();

        DateFormat format = new SimpleDateFormat(YYYY_MM_DD_HH_MM_SS_UTC_Z);
        format.setTimeZone(defaultTimezone);
        Date gmtTime = new Date();
        String str = format.format(gmtTime);
        StringBuilder sb = new StringBuilder(str);
        sb.insert(str.length() - LegoNumberConstant.TWO, ":");
        if (defaultTimezone.useDaylightTime()) {
            sb.append(" DST");
        }
        return sb.substring(LegoNumberConstant.TWENTY);
    }

    /**
     * 获取指定时间对应的时区信息
     * 如 UTC-11:00
     *
     * @param gmtTime 需要计算时区的时间
     * @return String [返回类型说明]
     */
    public static String getDefaultTimeZone(Date gmtTime) {
        TimeZone defaultTimezone = TimeZone.getDefault();

        DateFormat format = new SimpleDateFormat(YYYY_MM_DD_HH_MM_SS_UTC_Z);
        format.setTimeZone(defaultTimezone);
        String str = format.format(gmtTime);
        StringBuilder sb = new StringBuilder(str);
        sb.insert(str.length() - LegoNumberConstant.TWO, ":");
        if (defaultTimezone.useDaylightTime()) {
            sb.append(" DST");
        }
        return sb.substring(LegoNumberConstant.TWENTY);
    }
}
