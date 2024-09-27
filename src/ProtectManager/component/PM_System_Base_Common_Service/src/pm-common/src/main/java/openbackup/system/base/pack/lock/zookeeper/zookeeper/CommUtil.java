/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.pack.lock.zookeeper.zookeeper;

import openbackup.system.base.common.utils.ExceptionUtil;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.Closeable;
import java.io.IOException;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-05-30
 */
public final class CommUtil {
    private static final Logger LOGGER = LoggerFactory.getLogger(CommUtil.class);

    private CommUtil() {}

    /**
     * 静默关闭流
     *
     * @param closeable 可关闭对象
     */
    public static void closeQuietly(final Closeable closeable) {
        try {
            if (closeable != null) {
                closeable.close();
            }
        } catch (final IOException ioe) {
            LOGGER.error("close quietly error.", ExceptionUtil.getErrorMessage(ioe));
        }
    }
}
