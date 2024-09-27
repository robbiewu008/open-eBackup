/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.utils;

import openbackup.system.base.common.exception.EmeiStorDefaultExceptionHandler;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.Closeable;
import java.util.concurrent.TimeUnit;

/**
 * 工具类
 *
 * @author l90002863
 * @version [OceanStor ReplicationDirector V100R005C00, 2015-7-8]
 * @since 2019-11-01
 */
public final class CommonUtil {
    private static final Logger logger = LoggerFactory.getLogger(CommonUtil.class);

    /**
     * 最大允许的线程暂停时间：1天
     */
    private static final long MAX_SLEEP_TIME = 86400000L;

    private CommonUtil() {
    }

    /**
     * 关闭Closeable对象
     *
     * @param closed 可关闭的对象
     */
    public static void close(Closeable closed) {
        try {
            if (closed != null) {
                closed.close();
            }
        } catch (Exception ex) {
            logger.error("Exception while Closeable closed. error : %s", ExceptionUtil.getErrorMessage(ex));
        }
    }

    /**
     * 暂停线程执行
     *
     * @param millis 暂停时间，毫秒
     */
    public static void sleep(long millis) {
        if (millis <= 0 || millis > MAX_SLEEP_TIME) {
            logger.error("Invalid parameter. Time: {}", millis);
            throw new EmeiStorDefaultExceptionHandler("Invalid parameter. Time: " + millis);
        }

        try {
            TimeUnit.MILLISECONDS.sleep(millis);
        } catch (Exception e) {
            logger.error("Pausing thread failed. Millis: {}", millis, ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 线程休眠，最长修改时间不超过1天
     *
     * @param duration 休眠时长
     * @param unit     时间单位
     */
    public static void sleep(long duration, TimeUnit unit) {
        sleep(unit.toMillis(duration));
    }
}
