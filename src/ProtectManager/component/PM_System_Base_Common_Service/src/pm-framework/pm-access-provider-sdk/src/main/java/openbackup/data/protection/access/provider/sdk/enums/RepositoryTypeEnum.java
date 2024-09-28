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

import java.util.Arrays;

/**
 * 存储库类型的枚举类
 *
 **/
public enum RepositoryTypeEnum {
    /**
     * 元数据存储库，用户存储备份元数据
     */
    META(0),
    /**
     * 数据存储库，用户存储备份数据
     */
    DATA(1),
    /**
     * 缓存存储库
     */
    CACHE(2),

    /**
     * 日志存储库
     */
    LOG(3);

    private final int type;

    RepositoryTypeEnum(int type) {
        this.type = type;
    }

    public int getType() {
        return type;
    }

    /**
     * 根据存储库型获取存储库类型枚举类
     *
     * @param type 存储库类型
     * @return 存储库类型枚举类 {@code RepositoryTypeEnum}
     */
    public static RepositoryTypeEnum getByType(int type) {
        return Arrays.stream(RepositoryTypeEnum.values())
                .filter(repositoryType -> repositoryType.type == type)
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
