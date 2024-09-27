/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.exception;

/**
 * Message Retry Exception
 *
 * @author l00272247
 * @since 2021-04-15
 */
public class MessageRetryException extends RuntimeException {
    /**
     * constructor
     *
     * @param cause cause
     */
    public MessageRetryException(Throwable cause) {
        super(cause);
    }
}
