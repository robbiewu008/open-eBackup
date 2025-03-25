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

import static openbackup.system.base.common.constants.DateFormatConstant.DATE_TIME_WITH_MILLIS;

import lombok.extern.slf4j.Slf4j;

import org.apache.ibatis.type.BaseTypeHandler;
import org.apache.ibatis.type.JdbcType;
import org.apache.ibatis.type.MappedTypes;

import java.sql.CallableStatement;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.sql.Types;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.TimeZone;

/**
 * 功能描述 将插入数据库中的Timestamp类型的数据转换时区
 *
 */
@Slf4j
@MappedTypes({Timestamp.class})
public class TimestampTypeHandler extends BaseTypeHandler<Timestamp> {
    private static final TimeZone DATABASE_TIME_ZONE = TimeZone.getTimeZone("Asia/Shanghai");

    /**
     * 初始化
     */
    public TimestampTypeHandler() {
    }

    @Override
    public void setNonNullParameter(PreparedStatement ps, int i, Timestamp parameter, JdbcType jdbcType)
            throws SQLException {
        if (parameter == null) {
            ps.setNull(i, Types.TIMESTAMP);
            return;
        }
        SimpleDateFormat databaseSDF = new SimpleDateFormat(DATE_TIME_WITH_MILLIS);
        databaseSDF.setTimeZone(DATABASE_TIME_ZONE);
        String databaseTime = databaseSDF.format(parameter);
        ps.setString(i, databaseTime);
    }

    @Override
    public Timestamp getNullableResult(ResultSet rs, String columnName) throws SQLException {
        Timestamp timestamp = rs.getTimestamp(columnName, Calendar.getInstance(DATABASE_TIME_ZONE));
        return timestamp != null ? new Timestamp(timestamp.getTime()) : null;
    }

    @Override
    public Timestamp getNullableResult(ResultSet rs, int columnIndex) throws SQLException {
        Timestamp timestamp = rs.getTimestamp(columnIndex, Calendar.getInstance(DATABASE_TIME_ZONE));
        return timestamp != null ? new Timestamp(timestamp.getTime()) : null;
    }

    @Override
    public Timestamp getNullableResult(CallableStatement cs, int columnIndex) throws SQLException {
        Timestamp timestamp = cs.getTimestamp(columnIndex, Calendar.getInstance(DATABASE_TIME_ZONE));
        return timestamp != null ? new Timestamp(timestamp.getTime()) : null;
    }
}
