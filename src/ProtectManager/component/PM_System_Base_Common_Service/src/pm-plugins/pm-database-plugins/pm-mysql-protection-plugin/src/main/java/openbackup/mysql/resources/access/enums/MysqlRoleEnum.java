/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.mysql.resources.access.enums;

/**
 * mysql节点role枚举
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/10
 */
public enum MysqlRoleEnum {
    /**
     * MySQL主实例、集群实例
     */
    MASTER("1"),

    /**
     * MySQL备实例，普通实例
     */
    SLAVE("2");

    /**
     * role
     */
    private final String role;

    /**
     * 构造方法
     *
     * @param role role
     */
    MysqlRoleEnum(String role) {
        this.role = role;
    }

    /**
     * get role
     *
     * @return role
     */
    public String getRole() {
        return role;
    }
}
