/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.exception;

/**
 * Data Protection Reject Exception
 *
 * @author l00272247
 * @since 2021-12-11
 */
public class DataProtectionRejectException extends DataProtectionAccessException {
    /**
     * constructor
     *
     * @param errorCode error code
     * @param parameters parameters
     */
    public DataProtectionRejectException(long errorCode, String[] parameters) {
        super(errorCode, parameters);
    }

    /**
     * constructor
     *
     * @param errorCode error code
     * @param parameters parameters
     * @param cause cause
     */
    public DataProtectionRejectException(long errorCode, String[] parameters, Throwable cause) {
        super(errorCode, parameters, cause);
    }

    /**
     * constructor
     *
     * @param errorCode error code
     * @param parameters parameters
     * @param message message
     */
    public DataProtectionRejectException(long errorCode, String[] parameters, String message) {
        super(errorCode, parameters, message);
    }
}
