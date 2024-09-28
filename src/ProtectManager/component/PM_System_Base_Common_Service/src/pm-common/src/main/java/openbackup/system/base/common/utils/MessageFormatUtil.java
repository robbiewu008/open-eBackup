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

import java.text.MessageFormat;

/**
 * MessageFormatUtil
 *
 */
public class MessageFormatUtil {
    private MessageFormatUtil() {
    }

    /**
     * format
     *
     * @param pattern pattern
     * @param arguments arguments
     * @return String
     */
    public static synchronized String format(String pattern, Object[] arguments) {
        MessageFormat format = new MessageFormat(pattern);
        return format.format(arguments);
    }
}
