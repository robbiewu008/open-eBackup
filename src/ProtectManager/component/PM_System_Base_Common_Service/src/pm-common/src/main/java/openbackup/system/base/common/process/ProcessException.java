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
package openbackup.system.base.common.process;

import java.util.Locale;

/**
 * 处理描述
 *
 */
public class ProcessException extends Exception {
    /**
     * 默认构造函数
     *
     * @param message 信息
     */
    public ProcessException(String message) {
        super(message);
    }

    /**
     * 默认构造函数
     *
     * @param message 信息
     * @param parameters 参数
     */
    public ProcessException(String message, Object... parameters) {
        super(String.format(Locale.ENGLISH, message, parameters));
    }

    /**
     * 默认构造函数
     *
     * @param cause 异常
     * @param message 信息
     * @param parameters 参数
     */
    public ProcessException(Throwable cause, String message, Object... parameters) {
        super(String.format(Locale.ENGLISH, message, parameters), cause, false, false);
    }
}