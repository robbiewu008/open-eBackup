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
package openbackup.openstack.adapter.exception;

/**
 * OpenStack北向接口异常类
 *
 */
public class OpenStackException extends RuntimeException {
    private final long errorCode;

    public OpenStackException(long errorCode) {
        this.errorCode = errorCode;
    }

    /**
     * 构造方法
     *
     * @param errorCode 错误码
     * @param message 错误信息
     */
    public OpenStackException(long errorCode, String message) {
        super(message);
        this.errorCode = errorCode;
    }

    public long getErrorCode() {
        return errorCode;
    }
}
