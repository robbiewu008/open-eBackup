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
package openbackup.data.access.framework.core.common.exception;

import openbackup.system.base.common.exception.LegoCheckedException;

import org.slf4j.Logger;

/**
 * 未获取到锁的异常
 *
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
