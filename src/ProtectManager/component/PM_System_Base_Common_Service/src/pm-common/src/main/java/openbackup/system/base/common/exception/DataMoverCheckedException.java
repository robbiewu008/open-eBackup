/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
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
