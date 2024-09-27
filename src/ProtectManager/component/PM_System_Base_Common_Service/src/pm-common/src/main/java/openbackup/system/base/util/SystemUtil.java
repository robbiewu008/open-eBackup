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

import org.springframework.boot.SpringApplication;
import org.springframework.context.ApplicationContext;

/**
 * The SystemUtil
 *
 * @author g30003063
 * @since 2022-02-24
 */
public class SystemUtil {
    private SystemUtil() {
    }

    /**
     * 停止 JVM进程
     *
     * @param applicationContext applicationContext
     */
    public static void stopApplication(ApplicationContext applicationContext) {
        SpringApplication.exit(applicationContext, () -> 1);
        System.exit(1);
    }
}
