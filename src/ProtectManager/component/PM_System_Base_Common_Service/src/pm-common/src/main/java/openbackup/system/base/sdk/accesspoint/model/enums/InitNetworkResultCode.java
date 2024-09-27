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
package openbackup.system.base.sdk.accesspoint.model.enums;

import lombok.ToString;

/**
 * 初始化备份错误码
 *
 * @author swx1010572
 * @since 2021-01-15
 */
@ToString
public enum InitNetworkResultCode {
    /**
     * 成功
     */
    SUCCESS(0),

    /**
     * 失败
     */
    FAILURE(-1);

    /**
     * 编码
     */
    private final int code;

    /**
     * 带参数初始化函数
     *
     * @param theCode 编码
     */
    InitNetworkResultCode(int theCode) {
        code = theCode;
    }

    /**
     * 是否OK
     *
     * @return 是否OK
     */
    public boolean isOkay() {
        return this == SUCCESS;
    }
}
