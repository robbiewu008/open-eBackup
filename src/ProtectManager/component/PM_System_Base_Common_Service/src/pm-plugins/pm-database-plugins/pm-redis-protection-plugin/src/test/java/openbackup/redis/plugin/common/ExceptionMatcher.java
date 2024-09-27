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
package openbackup.redis.plugin.common;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.hamcrest.Description;
import org.junit.internal.matchers.TypeSafeMatcher;

/**
 * 自定义异常匹配器
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/06/16
 */
public class ExceptionMatcher extends TypeSafeMatcher<LegoCheckedException> {
    private long code;

    private String message;

    public ExceptionMatcher(long code, String message) {
        this.code = code;
        this.message = message;
    }

    @Override
    public boolean matchesSafely(LegoCheckedException item) {
        return item.getErrorCode() == code && item.getMessage().equals(message);
    }

    @Override
    public void describeTo(Description description) {
        description.appendText("expects code ")
            .appendValue(code)
            .appendText(",expects message ")
            .appendValue(message);
    }
}