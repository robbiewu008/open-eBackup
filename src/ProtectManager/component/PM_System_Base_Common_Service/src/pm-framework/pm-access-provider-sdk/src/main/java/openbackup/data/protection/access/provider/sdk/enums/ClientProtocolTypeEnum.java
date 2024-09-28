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
package openbackup.data.protection.access.provider.sdk.enums;

/**
 * 客户端协议类型枚举类
 *
 */
public enum ClientProtocolTypeEnum {
    /**
     * IP协议
     */
    IP(0),

    /**
     * DATA_TURBO协议
     */
    DATA_TURBO(1);

    /**
     * 客户端协议类型
     */
    private Integer clientProtocolType;

    ClientProtocolTypeEnum(int clientProtocolType) {
        this.clientProtocolType = clientProtocolType;
    }

    /**
     * 获取枚举值
     *
     * @return int
     */
    public int getClientProtocolType() {
        return clientProtocolType;
    }
}
