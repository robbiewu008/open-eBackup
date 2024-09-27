/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.exception;

/**
 * Not Implemented Exception
 *
 * @author l00272247
 * @since 2021-12-10
 */
public class NotImplementedException extends RuntimeException {
    /**
     * constructor
     *
     * @param message message
     */
    public NotImplementedException(String message) {
        super(message);
    }
}
