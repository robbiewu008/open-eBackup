/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 */

package openbackup.system.base.sdk.operationlog;

import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 用户模块记录操作日志
 *
 * @author jwx701567
 * @since 2021-04-23
 */
public interface ManualSendOperationLog {
    /**
     * 手动记录操作日志
     *
     * @param sourceType source name
     * @param moName mo name
     */
    default void sendOperationLog(String sourceType, String moName) {
        this.sendOperationLog(sourceType, moName, new String[]{}, true);
    }

    /**
     * 手动记录操作日志
     *
     * @param sourceType source name
     * @param moName mo name
     * @param params params
     */
    default void sendOperationLog(String sourceType, String moName, String[] params) {
        this.sendOperationLog(sourceType, moName, params, true);
    }

    /**
     * 手动记录操作日志
     *
     * @param sourceType source type
     * @param moName mo name
     * @param params params
     * @param isSuccess is success
     */
    void sendOperationLog(String sourceType, String moName, String[] params, boolean isSuccess);

    /**
     * 手动记录失败的操作日志
     *
     * @param sourceType source type
     * @param moName mo name
     * @param params params
     * @param exception exception
     */
    void sendOperationFailedLog(String sourceType, String moName, String[] params, LegoCheckedException exception);
}
