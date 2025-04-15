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
package openbackup.system.base.common.desensitization;

import openbackup.system.base.common.utils.VerifyUtil;

import java.util.HashMap;
import java.util.Map;

/**
 * 功能描述 crlf过滤，防日志注入
 *
 */
public class StringCrlfUtil {
    private static final Map<Character, String> CRLF_MAP = new HashMap<>();

    static {
        CRLF_MAP.put('\t', "\\t");
        CRLF_MAP.put('\r', "\\r");
        CRLF_MAP.put('\n', "\\n");
        // BS
        CRLF_MAP.put('\u0008', "\\b");
        // VT
        CRLF_MAP.put('\u000B', "\\u000B");
        // FF
        CRLF_MAP.put('\u000C', "\\f");
        // DEL
        CRLF_MAP.put('\u007F', "\\u007F");
    }

    private StringCrlfUtil() {
    }

    /**
     * escape crlf for input string
     *
     * @param originalMsg input string
     * @return escape crlf
     */
    public static String escapeCrlf(String originalMsg) {
        if (VerifyUtil.isEmpty(originalMsg)) {
            return originalMsg;
        }
        StringBuilder sb = new StringBuilder();
        char[] array = originalMsg.toCharArray();
        for (char currentChar : array) {
            if (CRLF_MAP.containsKey(currentChar)) {
                sb.append(CRLF_MAP.get(currentChar));
            } else {
                sb.append(currentChar);
            }
        }
        return sb.toString();
    }
}