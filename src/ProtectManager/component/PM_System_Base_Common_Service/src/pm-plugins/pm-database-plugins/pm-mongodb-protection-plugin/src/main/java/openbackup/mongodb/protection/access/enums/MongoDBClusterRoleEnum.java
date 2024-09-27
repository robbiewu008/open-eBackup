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
package openbackup.mongodb.protection.access.enums;

import java.util.HashMap;
import java.util.Map;

/**
 * MongoDB 数据库角色
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
public enum MongoDBClusterRoleEnum {
    PRIMARY("PRIMARY"),

    SECONDARY("SECONDARY"),

    FAULT("FAULT"),

    ARBITER("ARBITER");

    private static final Map<String, MongoDBClusterRoleEnum> ROLE_ENUM_MAP = new HashMap<>(
        MongoDBClusterRoleEnum.values().length);

    MongoDBClusterRoleEnum(String role) {
        this.role = role;
    }

    static {
        for (MongoDBClusterRoleEnum each : MongoDBClusterRoleEnum.values()) {
            ROLE_ENUM_MAP.put(each.getRole(), each);
        }
    }

    private final String role;

    public String getRole() {
        return role;
    }

    /**
     * 根据属性值返回对应的枚举值
     *
     * @param value 属性值
     * @return 枚举值
     */
    public static MongoDBClusterRoleEnum getValue(String value) {
        MongoDBClusterRoleEnum mongoDBClusterRoleEnum = ROLE_ENUM_MAP.get(value);
        if (mongoDBClusterRoleEnum == null) {
            return MongoDBClusterRoleEnum.FAULT;
        }
        return mongoDBClusterRoleEnum;
    }
}
