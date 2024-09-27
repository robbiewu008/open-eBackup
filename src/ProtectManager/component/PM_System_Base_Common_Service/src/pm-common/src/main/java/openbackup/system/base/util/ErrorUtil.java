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

import openbackup.system.base.common.utils.ExceptionUtil;

import java.net.SocketException;
import java.net.SocketTimeoutException;
import java.net.UnknownHostException;
import java.util.stream.Stream;

/**
 * 异常工具类
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-30
 */
public class ErrorUtil {
    /**
     * 判断是否为连接异常
     *
     * @param throwable 异常
     * @return true/false
     */
    public static boolean isRetryableException(Throwable throwable) {
        return Stream.of(UnknownHostException.class, SocketException.class, SocketTimeoutException.class)
            .anyMatch(type -> ExceptionUtil.lookFor(throwable, type) != null);
    }
}
