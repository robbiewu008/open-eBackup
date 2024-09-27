/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
