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
package openbackup.system.base.common.constants;

import openbackup.system.base.common.exception.ErrorResponse;

import java.util.Arrays;
import java.util.Comparator;
import java.util.List;

/**
 * 功能描述
 *
 */
public enum ForwardErrorPriorityEnum {
    NEED_EMAIL_DYNAMIC_PWD_ERROR(1, 1677752064L),
    EMAIL_DYNAMIC_CODE_ERROR(2, 1677929500L),
    LDAP_ERROR_PASS_UPPER_LIMITS(3, 1677929516L),
    LOGIN_CRED_WRONG(4, 1677929488L),
    NOT_NEED_EMAIL_DYNAMIC_PWD(5, 1677929473L),
    FREQUENT_SENDING_EMAIL_DYNAMIC(6, 1677929517L);

    private final int priority;

    private final long errorCode;

    ForwardErrorPriorityEnum(int priority, long errorCode) {
        this.priority = priority;
        this.errorCode = errorCode;
    }

    /**
     * 用来根据优先级选择要抛出的异常错误码 如果没有则返回空的错误码(new)
     *
     * @param errors 要选择的错误列表
     * @return 优先级最高的错误
     */
    public static ErrorResponse selectCriticalError(List<ErrorResponse> errors) {
        return errors.stream()
            .min(Comparator.comparingInt(error -> getPriorityByCode(error.getErrorCode())))
            .orElse(new ErrorResponse());
    }

    // 映射错误码到优先级（带保护机制）
    private static int getPriorityByCode(String code) {
        try {
            long errorCode = Long.parseLong(code);
            return Arrays.stream(ForwardErrorPriorityEnum.values())
                .filter(error -> error.errorCode == errorCode)
                .findFirst()
                .map(error -> error.priority)
                .orElse(Integer.MAX_VALUE); // 未匹配的错误码赋予最低优先级
        } catch (NumberFormatException ex) {
            return Integer.MAX_VALUE; // 非数字错误码兜底处理
        }
    }
}
