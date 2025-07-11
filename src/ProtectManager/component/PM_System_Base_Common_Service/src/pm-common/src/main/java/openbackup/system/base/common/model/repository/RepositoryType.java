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
package openbackup.system.base.common.model.repository;

/**
 * 功能描述
 *
 */
public enum RepositoryType {
    /**
     * 本地存储库类型
     */
    LOCAL(1),
    /**
     * S3存储库类型
     */
    S3(2),
    /**
     * 蓝光存储库类型
     */
    BLUE_RAY(3),
    /**
     * 磁带存储库类型
     */
    TAPE(4),
    /**
     * 外部存储库类型
     */
    EXTERNAL(5);

    private final int type;

    RepositoryType(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    /**
     * 根据传入的值获取对应的枚举值
     *
     * @param value value
     * @return RepositoryType
     */
    public static RepositoryType getValue(int value) {
        for (RepositoryType msgType : RepositoryType.values()) {
            if (value == msgType.type) {
                return msgType;
            }
        }
        return RepositoryType.LOCAL;
    }
}
