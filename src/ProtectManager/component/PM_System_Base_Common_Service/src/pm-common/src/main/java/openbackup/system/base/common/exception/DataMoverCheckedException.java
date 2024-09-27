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

import openbackup.system.base.common.constants.CommonErrorCode;

/**
 * Data Mover Checked Exception
 *
 * @author l00272247
 * @since 2020-07-09
 */
public class DataMoverCheckedException extends LegoCheckedException {
    /**
     * constructor
     */
    public DataMoverCheckedException() {
        this(new String[0]);
    }

    /**
     * constructor
     *
     * @param args args
     */
    public DataMoverCheckedException(String[] args) {
        this(CommonErrorCode.OPERATION_FAILED, args);
    }

    /**
     * constructor
     *
     * @param code code
     */
    public DataMoverCheckedException(long code) {
        this(code, new String[0]);
    }

    /**
     * constructor
     *
     * @param code code
     * @param args args
     */
    public DataMoverCheckedException(long code, String[] args) {
        super(code, args);
    }

    /**
     * constructor
     *
     * @param message message
     * @param code code
     */
    public DataMoverCheckedException(String message, long code) {
        this(message, code, new String[0]);
    }

    /**
     * constructor
     *
     * @param message message
     * @param code code
     * @param args args
     */
    public DataMoverCheckedException(String message, long code, String[] args) {
        super(code, args, message);
    }

    /**
     * constructor
     *
     * @param message message
     * @param cause cause
     * @param code code
     * @param args args
     */
    public DataMoverCheckedException(String message, Throwable cause, long code, String[] args) {
        super(message, cause);
    }

    /**
     * constructor
     *
     * @param cause cause
     * @param code code
     */
    public DataMoverCheckedException(Throwable cause, long code) {
        this(cause, code, new String[0]);
    }

    /**
     * constructor
     *
     * @param cause cause
     * @param code code
     * @param args args
     */
    public DataMoverCheckedException(Throwable cause, long code, String[] args) {
        super(code, args, cause);
    }
}
