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
package openbackup.system.base.common.exception;

import org.slf4j.Logger;

/**
 * 资源不存在异常
 *
 */
public class ResourceNotExistException extends LegoCheckedException {
    /**
     * ResourceNotExistException
     *
     * @param message message
     * @param cause cause
     */
    public ResourceNotExistException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * ResourceNotExistException
     *
     * @param message message
     */
    public ResourceNotExistException(String message) {
        super(message);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param cause cause
     */
    public ResourceNotExistException(long errorCode, String[] parameter, Throwable cause) {
        super(errorCode, parameter, cause);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param message message
     */
    public ResourceNotExistException(long errorCode, String[] parameter, String message) {
        super(errorCode, parameter, message);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param message message
     * @param cause cause
     */
    public ResourceNotExistException(long errorCode, String[] parameter, String message, Throwable cause) {
        super(errorCode, parameter, message, cause);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param message message
     * @param logger logger
     * @param parameter parameter
     */
    public ResourceNotExistException(long errorCode, String message, Logger logger, String... parameter) {
        super(errorCode, message, logger, parameter);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param throwable throwable
     */
    public ResourceNotExistException(long errorCode, Throwable throwable) {
        super(errorCode, throwable);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param info info
     */
    public ResourceNotExistException(long errorCode, String info) {
        super(errorCode, info);
    }

    /**
     * ResourceNotExistException
     *
     * @param errorCode errorCode
     * @param info info
     * @param throwable throwable
     */
    public ResourceNotExistException(long errorCode, String info, Throwable throwable) {
        super(errorCode, info, throwable);
    }
}
