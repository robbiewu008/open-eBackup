/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
