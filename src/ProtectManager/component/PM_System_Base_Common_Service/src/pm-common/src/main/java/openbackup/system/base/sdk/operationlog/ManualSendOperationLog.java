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
package openbackup.system.base.sdk.operationlog;

import openbackup.system.base.common.exception.LegoCheckedException;

/**
 * 用户模块记录操作日志
 *
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
