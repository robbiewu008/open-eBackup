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
package com.huawei.emeistor.console.kmchelp.kmc;

import com.huawei.kmc.common.ILogger;

import java.text.SimpleDateFormat;
import java.time.Instant;
import java.util.Date;

/**
 * 文 件 名: CryptLogger
 * 包 名: com.huawei.emeistor.kms.kmc.util.kmc
 * 描 述: CryptLogger日志工具
 *
 */
public class CryptLogger extends ILogger {
    private String getCaller() {
        StackTraceElement[] arr = (new Throwable()).getStackTrace();
        /* 打印格式: date | thread | class | msg */
        StringBuilder sb = new StringBuilder(Constant.VOLUME_1024);
        String currentThreadName = Thread.currentThread().getName();
        String className = arr[Constant.STACK_CALLER_INDEX].getClassName();
        String methodName = arr[Constant.STACK_CALLER_INDEX].getMethodName();
        int lineNumber = arr[Constant.STACK_CALLER_INDEX].getLineNumber();

        sb.append(getDate())
            .append(" | ")
            .append(currentThreadName)
            .append(" | ")
            .append(className)
            .append('(')
            .append(methodName)
            .append(':')
            .append(lineNumber)
            .append(')');
        return sb.toString();
    }

    private String getDate() {
        return new SimpleDateFormat("yyyy-MM-dd HH:mm:ss").format(Date.from(Instant.now()));
    }

    @Override
    public void debug(String message) {
        StringBuilder msgBuilder = new StringBuilder(Constant.VOLUME_1024);
        msgBuilder.append(" [debug] ").append(getCaller()).append(message).append(Constant.CR);
        msgBuilder.setLength(0);
    }

    @Override
    public void info(String message) {
        StringBuilder msgBuilder = new StringBuilder(Constant.VOLUME_1024);
        msgBuilder.append(" [info] ").append(getCaller()).append(message).append(Constant.CR);
        msgBuilder.setLength(0);
    }

    @Override
    public void warn(String message) {
        StringBuilder msgBuilder = new StringBuilder(Constant.VOLUME_1024);
        msgBuilder.append(" [warn] ").append(getCaller()).append(message).append(Constant.CR);
        msgBuilder.setLength(0);
    }

    @Override
    public void error(String message) {
        StringBuilder msgBuilder = new StringBuilder(Constant.VOLUME_1024);
        msgBuilder.append(" [error] ").append(getCaller()).append(message).append(Constant.CR);
        msgBuilder.setLength(0);
    }

    @Override
    public void trace(String message) {
        StringBuilder msgBuilder = new StringBuilder(Constant.VOLUME_1024);
        msgBuilder.append(" [trace] ").append(getCaller()).append(message).append(Constant.CR);
        msgBuilder.setLength(0);
    }
}
