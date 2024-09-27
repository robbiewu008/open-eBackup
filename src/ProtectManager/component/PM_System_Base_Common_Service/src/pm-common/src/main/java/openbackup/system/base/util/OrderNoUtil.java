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

import openbackup.system.base.common.exception.LegoCheckedException;

import java.security.NoSuchAlgorithmException;
import java.security.SecureRandom;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * 编号获取工具类
 *
 * @author w00607005
 * @since 2023-07-07
 */
public class OrderNoUtil {
    /**
     * 任务流子任务
     */
    private static final String TASK = "TK";

    /**
     * 时间格式化
     */
    private static final String DATE_TIME_PATTERN_SIMPLE = "yyMMddHHmmssSS";

    private static final AtomicInteger NEXT_COUNTER;

    static {
        SecureRandom secureRandom;
        try {
            secureRandom = SecureRandom.getInstanceStrong();
            NEXT_COUNTER = new AtomicInteger(secureRandom.nextInt(100));
        } catch (NoSuchAlgorithmException e) {
            throw LegoCheckedException.cast(e);
        }
    }

    /**
     * 获取任务序列
     *
     * @return taskNo
     */
    public static String getTaskNo() {
        return generateNo(TASK);
    }

    private static String generateNo(String type) {
        String dateTimeStr = LocalDateTime.now().format(DateTimeFormatter.ofPattern(DATE_TIME_PATTERN_SIMPLE));
        return type + dateTimeStr + NEXT_COUNTER.getAndIncrement();
    }
}
