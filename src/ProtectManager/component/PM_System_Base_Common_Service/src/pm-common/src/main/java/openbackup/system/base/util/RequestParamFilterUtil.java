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
package openbackup.system.base.util;

import java.util.Locale;
import java.util.Objects;
import java.util.Optional;

/**
 * 请求参数过滤处理
 *
 */
public class RequestParamFilterUtil {
    /**
     * 通配符处理
     *
     * @param value 入参
     * @param <T> 类型
     * @return 处理后处理
     */
    public static <T> String escape(T value) {
        return Optional.ofNullable(value)
            .map(Objects::toString)
            .map(str -> str.toLowerCase(Locale.ENGLISH))
            .map(str -> str.replaceAll("\\\\", "\\\\"))
            .map(str -> str.replaceAll("%", "\\\\%"))
            .map(str -> str.replaceAll("\\?", "\\\\?"))
            .map(str -> str.replaceAll("_", "\\\\_"))
            .orElse(null);
    }
}
