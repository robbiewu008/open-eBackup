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

import org.springframework.stereotype.Component;

/**
 * Number Converter
 *
 */
@Component
public class NumberConverter extends AbstractConverter {
    /**
     * constructor
     */
    public NumberConverter() {
        super("number");
    }

    /**
     * data cast
     *
     * @param data data
     * @return result
     */
    @Override
    protected Object cast(Object data) {
        if (data == null || data instanceof Number) {
            return data;
        }
        if (data instanceof CharSequence) {
            String text = data.toString();
            if (text.matches("\\d+")) {
                return Long.valueOf(text);
            } else if (text.matches("(\\d+)?\\.\\d+")) {
                return Double.valueOf(text);
            } else {
                return null;
            }
        }
        return null;
    }
}
