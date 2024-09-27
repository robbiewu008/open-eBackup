/*
 *
 *  * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 *
 */

package openbackup.data.access.framework.core.common.exception;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.slf4j.Logger;

/**
 * 未获取到锁的异常
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-22
 */
public class LockNotObtainedException extends LegoCheckedException {
    public LockNotObtainedException(String message, Throwable cause) {
        super(message, cause);
    }

    public LockNotObtainedException(String message) {
        super(message);
    }

    public LockNotObtainedException(long errorCode, String[] parameter, Throwable cause) {
        super(errorCode, parameter, cause);
    }

    public LockNotObtainedException(long errorCode, String[] parameter, String message) {
        super(errorCode, parameter, message);
    }

    public LockNotObtainedException(long errorCode, String message, Logger logger, String... parameter) {
        super(errorCode, message, logger, parameter);
    }

    public LockNotObtainedException(long errorCode, Throwable throwable) {
        super(errorCode, throwable);
    }

    public LockNotObtainedException(long errorCode, String info) {
        super(errorCode, info);
    }

    public LockNotObtainedException(long errorCode, String info, Throwable throwable) {
        super(errorCode, info, throwable);
    }
}
