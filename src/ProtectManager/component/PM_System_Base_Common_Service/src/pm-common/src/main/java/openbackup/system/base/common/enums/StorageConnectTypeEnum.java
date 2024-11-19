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
package openbackup.system.base.common.enums;

/**
 * 存储库连接类型
 *
 */
public enum StorageConnectTypeEnum {
    /**
     * 标准模式
     */
    STANDARD(0),

    /**
     * 连接字符串模式
     */
    CONNECT_INFO(1);

    private final int connectType;

    StorageConnectTypeEnum(int connectType) {
        this.connectType = connectType;
    }

    /**
     * 获取connect type
     *
     * @return connect type
     */
    public int getConnectType() {
        return connectType;
    }
}