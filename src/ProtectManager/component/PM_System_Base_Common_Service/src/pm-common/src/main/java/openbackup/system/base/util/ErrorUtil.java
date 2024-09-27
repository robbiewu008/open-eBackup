/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
