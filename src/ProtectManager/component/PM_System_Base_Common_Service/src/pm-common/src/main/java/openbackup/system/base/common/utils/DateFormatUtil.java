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

import java.text.DateFormat;
import java.text.ParseException;
import java.util.Date;

/**
 * DateFormatUtil
 *
 * @author p00171530
 * @version [Lego V100R002C10, 2014-12-8]
 * @since 2019-10-31
 */
public class DateFormatUtil {
    private DateFormatUtil() {
    }

    /**
     * format
     *
     * @param dateformat dateformat
     * @param date       date
     * @return String String
     */
    public static synchronized String format(DateFormat dateformat, Date date) {
        return dateformat.format(date);
    }

    /**
     * format
     *
     * @param dateformat dateformat
     * @param obj        obj
     * @return String String
     */
    public static synchronized String format(DateFormat dateformat, Object obj) {
        return dateformat.format(obj);
    }

    /**
     * parse
     *
     * @param dateformat date format
     * @param date       date
     * @return Date
     * @throws ParseException Date
     */
    public static synchronized Date parse(DateFormat dateformat, String date) throws ParseException {
        return dateformat.parse(date);
    }
}
