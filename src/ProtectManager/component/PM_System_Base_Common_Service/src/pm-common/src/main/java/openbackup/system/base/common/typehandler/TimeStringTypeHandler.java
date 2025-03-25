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
import static openbackup.system.base.common.constants.DateFormatConstant.DATE_TIME_WITH_T;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;

import org.apache.ibatis.type.BaseTypeHandler;
import org.apache.ibatis.type.JdbcType;
import org.apache.ibatis.type.MappedTypes;

import java.sql.CallableStatement;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.sql.Types;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.time.format.DateTimeParseException;
import java.util.Calendar;
import java.util.Date;
import java.util.TimeZone;

/**
 * 功能描述 将插入数据库中的String类型的数据转换时区
 *
 */
@Slf4j
@MappedTypes({String.class})
public class TimeStringTypeHandler extends BaseTypeHandler<String> {
    private static final TimeZone DATABASE_TIME_ZONE = TimeZone.getTimeZone("Asia/Shanghai");
    private static final SimpleDateFormat DATABASE_SDF = new SimpleDateFormat(DATE_TIME);
    private static final SimpleDateFormat SYSTEM_SDF = new SimpleDateFormat(DATE_TIME);
    private static final SimpleDateFormat SYSTEM_SDF_WITH_T = new SimpleDateFormat(DATE_TIME_WITH_T);

    /**
     * 初始化
     */
    public TimeStringTypeHandler() {
    }

    @Override
    public void setNonNullParameter(PreparedStatement ps, int i, String parameter, JdbcType jdbcType)
            throws SQLException {
        if (parameter == null) {
            ps.setNull(i, Types.TIMESTAMP);
            return;
        }
        ps.setString(i, doToDatabaseTime(parameter));
    }

    @Override
    public String getNullableResult(ResultSet rs, String columnName) throws SQLException {
        Timestamp timestamp = rs.getTimestamp(columnName, Calendar.getInstance(DATABASE_TIME_ZONE));
        return timestamp != null ? SYSTEM_SDF.format(new Date(timestamp.getTime())) : null;
    }

    @Override
    public String getNullableResult(ResultSet rs, int columnIndex) throws SQLException {
        Timestamp timestamp = rs.getTimestamp(columnIndex, Calendar.getInstance(DATABASE_TIME_ZONE));
        return timestamp != null ? SYSTEM_SDF.format(new Date(timestamp.getTime())) : null;
    }

    @Override
    public String getNullableResult(CallableStatement cs, int columnIndex) throws SQLException {
        Timestamp timestamp = cs.getTimestamp(columnIndex, Calendar.getInstance(DATABASE_TIME_ZONE));
        return timestamp != null ? SYSTEM_SDF.format(new Date(timestamp.getTime())) : null;
    }

    /**
     * 切换String类型date
     *
     * @param dateTime 系统时间
     * @return 数据库时区时间
     */
    private String toDatabaseTime(String dateTime) {
        if (VerifyUtil.isEmpty(dateTime)) {
            return dateTime;
        }
        DATABASE_SDF.setTimeZone(DATABASE_TIME_ZONE);
        SYSTEM_SDF.setTimeZone(TimeZone.getDefault());
        try {
            Date systemTime = SYSTEM_SDF.parse(dateTime);
            return DATABASE_SDF.format(systemTime);
        } catch (DateTimeParseException | ParseException e) {
            log.error("database dateTime parse error", e);
            try {
                Date systemTime = SYSTEM_SDF_WITH_T.parse(dateTime);
                return DATABASE_SDF.format(systemTime);
            } catch (DateTimeParseException | ParseException exception) {
                log.error("database dateTime parse error", exception);
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "request param date parse error");
            }
        }
    }

    /**
     * 捕获异常，不影响入库
     *
     * @param dateTime 系统时间
     * @return 数据库时区时间
     */
    private String doToDatabaseTime(String dateTime) {
        try {
            return toDatabaseTime(dateTime);
        } catch (Exception e) {
            log.error("extParameters convert error", ExceptionUtil.getErrorMessage(e));
            return dateTime;
        }
    }
}
