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

/**
 * 替换特殊字符
 * 把特殊字符的有实体替换成原状
 *
 */
public final class HtmlStringConverter {
    private HtmlStringConverter() {
    }

    /**
     * 解码
     *
     * @param encodedChar 解码串
     * @return 解码后的值
     */
    public static String decode(String encodedChar) {
        if (encodedChar == null) {
            return "";
        }
        return encodedChar.replaceAll("&quot;", "\"")
            .replaceAll("&#39;", "'")
            .replaceAll("&#96;", "`")
            .replaceAll("&#37;", "%")
            .replaceAll("&lt;", "<")
            .replaceAll("&gt;", ">")
            .replaceAll("&#40;", "\\\\(")
            .replaceAll("&#41;", "\\\\)")
            .replaceAll("&#58;", ":")
            .replaceAll("&#46;", ".");
    }

    /**
     * 编码
     *
     * @param encodedChar 编码串
     * @return 编码后的串
     */
    public static String decodeByLength(String encodedChar) {
        if (encodedChar == null) {
            return "";
        }
        return encodedChar.replaceAll("&quot;", "\"")
            .replaceAll("&#39;", "'")
            .replaceAll("&#96;", "`")
            .replaceAll("&#37;", "%")
            .replaceAll("&lt;", "<")
            .replaceAll("&gt;", ">")
            .replaceAll("&#40;", "(")
            .replaceAll("&#41;", ")")
            .replaceAll("&nbsp;", " ")
            .replaceAll("&#58;", ":")
            .replaceAll("&#46;", ".")
            .replaceAll("&#58;", ":")
            .replaceAll("&#46;", ".");
    }
}