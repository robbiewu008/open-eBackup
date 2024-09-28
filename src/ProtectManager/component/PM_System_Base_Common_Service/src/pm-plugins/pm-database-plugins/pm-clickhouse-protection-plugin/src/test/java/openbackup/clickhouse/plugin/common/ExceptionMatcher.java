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
package openbackup.clickhouse.plugin.common;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.hamcrest.Description;
import org.hamcrest.TypeSafeMatcher;

/**
 * 自定义异常匹配器
 *
 */
public class ExceptionMatcher extends TypeSafeMatcher<LegoCheckedException> {
    private final long errorCode;

    private final String message;

    public ExceptionMatcher(long errorCode, String message) {
        this.errorCode = errorCode;
        this.message = message;
    }

    @Override
    public boolean matchesSafely(LegoCheckedException item) {
        return item.getErrorCode() == errorCode && item.getMessage().equals(message);
    }

    @Override
    public void describeTo(Description description) {
        description.appendText("expects errorCode ")
            .appendValue(errorCode)
            .appendText(",expects message ")
            .appendValue(message);
    }
}
