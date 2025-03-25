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
package openbackup.system.base.common.typehandler;

import static openbackup.system.base.common.constants.DateFormatConstant.DATE_TIME;
import static openbackup.system.base.common.constants.DateFormatConstant.TIME;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.commons.lang3.StringUtils;
import org.apache.ibatis.type.BaseTypeHandler;
import org.apache.ibatis.type.JdbcType;
import org.apache.ibatis.type.MappedTypes;

import java.sql.CallableStatement;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Time;
import java.sql.Types;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.time.format.DateTimeFormatter;

/**
 * 功能描述 将插入数据库中的Time类型的数据转换时区
 *
 */
@Slf4j
@MappedTypes({String.class})
public class WindowStartTimeTypeHandler extends BaseTypeHandler<String> {
    private static final String DATABASE_TIME_ZONE_ID = "Asia/Shanghai";
    private static final DateTimeFormatter DATA_TIME_FORMATTER = DateTimeFormatter.ofPattern(DATE_TIME);
    private static final DateTimeFormatter TIME_FORMATTER = DateTimeFormatter.ofPattern(TIME);
    private static final String START_TIME = "start_time";
    private static final String CREATED_TIME = "created";

    @Override
    public void setNonNullParameter(PreparedStatement ps, int i, String parameter, JdbcType jdbcType)
            throws SQLException {
        String time = convertTimeToDatabaseZone(parameter);
        if (VerifyUtil.isEmpty(time)) {
            ps.setNull(i, Types.TIME);
            return;
        }
        ps.setTime(i, Time.valueOf(time));
    }

    @Override
    public String getNullableResult(ResultSet rs, String columnName) throws SQLException {
        String windowStart = rs.getString(columnName);
        if (VerifyUtil.isEmpty(windowStart)) {
            return windowStart;
        }
        String startTime = rs.getString(START_TIME);
        if (VerifyUtil.isEmpty(startTime)) {
            startTime = rs.getString(CREATED_TIME).replaceAll("\\.\\d+$", "");
        }
        return getSystemZoneTime(startTime, windowStart);
    }

    @Override
    public String getNullableResult(ResultSet rs, int columnIndex) throws SQLException {
        String windowStart = rs.getString(columnIndex);
        if (VerifyUtil.isEmpty(windowStart)) {
            return windowStart;
        }
        String startTime = rs.getString(START_TIME);
        if (VerifyUtil.isEmpty(startTime)) {
            startTime = rs.getString(CREATED_TIME).replaceAll("\\.\\d+$", "");
        }
        return getSystemZoneTime(startTime, windowStart);
    }

    @Override
    public String getNullableResult(CallableStatement cs, int columnIndex) throws SQLException {
        String windowStart = cs.getString(columnIndex);
        if (VerifyUtil.isEmpty(windowStart)) {
            return windowStart;
        }
        String startTime = cs.getString(START_TIME);
        if (VerifyUtil.isEmpty(startTime)) {
            startTime = cs.getString(CREATED_TIME).replaceAll("\\.\\d+$", "");
        }
        return getSystemZoneTime(startTime, windowStart);
    }

    private String convertTimeToDatabaseZone(String dateTime) {
        if (VerifyUtil.isEmpty(dateTime)) {
            return StringUtils.EMPTY;
        }
        LocalDateTime localDateTime = LocalDateTime.parse(dateTime, DATA_TIME_FORMATTER);
        ZonedDateTime systemTime = localDateTime.atZone(ZoneId.systemDefault());
        ZonedDateTime beijingTime = systemTime.withZoneSameInstant(ZoneId.of(DATABASE_TIME_ZONE_ID));
        return beijingTime.toLocalTime().format(TIME_FORMATTER);
    }

    private String convertTimeToSystemZone(String dateTime) {
        if (VerifyUtil.isEmpty(dateTime)) {
            return StringUtils.EMPTY;
        }
        LocalDateTime localDateTime = LocalDateTime.parse(dateTime, DATA_TIME_FORMATTER);
        ZonedDateTime beijingTime = localDateTime.atZone(ZoneId.of(DATABASE_TIME_ZONE_ID));
        ZonedDateTime systemTime = beijingTime.withZoneSameInstant(ZoneId.systemDefault());
        return systemTime.toLocalTime().format(TIME_FORMATTER);
    }

    private String getSystemZoneTime(String dateTime, String timeWindow) {
        if (VerifyUtil.isEmpty(timeWindow)) {
            return StringUtils.EMPTY;
        }
        LocalTime time = LocalTime.parse(timeWindow, TIME_FORMATTER);
        String startDateTime = LocalDateTime.parse(dateTime, DATA_TIME_FORMATTER)
                .with(time).format(DATA_TIME_FORMATTER);
        return convertTimeToSystemZone(startDateTime);
    }
}