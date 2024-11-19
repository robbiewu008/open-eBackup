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
package openbackup.system.base.common.aspect;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.Constants;

import org.springframework.stereotype.Component;

import java.text.ParseException;
import java.util.Date;
import java.util.Optional;

/**
 * Date Converter
 *
 */
@Slf4j
@Component
public class DateConverter extends AbstractConverter {
    /**
     * constructor
     */
    public DateConverter() {
        super("date");
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    @Override
    protected Object cast(Object data) {
        Optional<Date> date = parse(data);
        if (!date.isPresent()) {
            return null;
        }
        return Constants.SIMPLE_DATE_FORMAT.format(date);
    }

    private Optional<Date> parse(Object data) {
        if (data == null || data instanceof Date) {
            return Optional.of((Date) data);
        }
        if (data instanceof String) {
            String text = (String) data;
            try {
                return Optional.of(Constants.SIMPLE_DATE_FORMAT.parse(text));
            } catch (ParseException e) {
                log.error("parse date failed. text: {}", text);
                return Optional.empty();
            }
        }
        if (data instanceof Number) {
            Number num = (Number) data;
            return Optional.of(new Date(num.longValue()));
        }
        return Optional.empty();
    }
}
