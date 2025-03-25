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

import lombok.extern.slf4j.Slf4j;

import org.apache.ibatis.type.BaseTypeHandler;
import org.apache.ibatis.type.JdbcType;
import org.apache.ibatis.type.MappedTypes;

import java.sql.CallableStatement;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Time;
import java.sql.Types;
import java.time.ZoneId;
import java.time.ZonedDateTime;
import java.util.Calendar;
import java.util.TimeZone;

/**
 * 功能描述 将插入数据库中的Time类型的数据转换时区
 *
 */
@Slf4j
@MappedTypes({Time.class})
public class TimeTypeHandler extends BaseTypeHandler<Time> {
    private static final TimeZone DATABASE_TIME_ZONE = TimeZone.getTimeZone("Asia/Shanghai");

    /**
     * 初始化
     */
    public TimeTypeHandler() {
    }

    @Override
    public void setNonNullParameter(PreparedStatement ps, int i, Time parameter, JdbcType jdbcType)
            throws SQLException {
        if (parameter == null) {
            ps.setNull(i, Types.TIME);
            return;
        }
        ZonedDateTime systemTime = parameter.toLocalTime()
                .atDate(java.time.LocalDate.now()).atZone(ZoneId.systemDefault());
        ZonedDateTime beijingTime = systemTime.withZoneSameInstant(ZoneId.of("Asia/Shanghai"));
        ps.setTime(i, Time.valueOf(beijingTime.toLocalTime()));
    }

    @Override
    public Time getNullableResult(ResultSet rs, String columnName) throws SQLException {
        return rs.getTime(columnName, Calendar.getInstance(DATABASE_TIME_ZONE));
    }

    @Override
    public Time getNullableResult(ResultSet rs, int columnIndex) throws SQLException {
        return rs.getTime(columnIndex, Calendar.getInstance(DATABASE_TIME_ZONE));
    }

    @Override
    public Time getNullableResult(CallableStatement cs, int columnIndex) throws SQLException {
        return cs.getTime(columnIndex, Calendar.getInstance(DATABASE_TIME_ZONE));
    }
}