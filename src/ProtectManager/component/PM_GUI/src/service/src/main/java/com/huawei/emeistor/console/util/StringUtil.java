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
package com.huawei.emeistor.console.util;

import java.lang.reflect.Field;

/**
 * String 工具类
 *
 * @author mwx776342
 * @version V100R001C00
 * @since 2022-03-07
 */
public class StringUtil {
    private static final int DEFAULT_TIMES = 3;

    /**
     * 清理字符串：三次写0
     *
     * @param string 字符串
     */
    public static final void tripleWriteZero(String string) {
        if (string != null) {
            try {
                // 每次拿效率不高，考虑到用的不多，暂时这样
                Field valueField = String.class.getDeclaredField("value");
                valueField.setAccessible(true);

                // 获取字符串中的值
                Object valueObject = valueField.get(string);

                // valueObject为null，无法通过下面的判断
                if (valueObject instanceof char[]) {
                    char[] valueChars = (char[]) valueObject;
                    tripleWriteZeroToChars(valueChars);
                }
            } catch (NoSuchFieldException | IllegalAccessException exception) {
                return;
            }
        }
    }

    /**
     * 清理字符数组：三次写0
     *
     * @param chars 字符数组
     */
    public static void tripleWriteZeroToChars(char[] chars) {
        // 字符数组不为空，且字符数组数量大于0
        if (chars != null && chars.length > 0) {
            for (int times = 0; times < DEFAULT_TIMES; times++) {
                for (int index = 0; index < chars.length; index++) {
                    chars[index] = 0;
                }
            }
        }
    }
}
