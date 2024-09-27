/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.resource.enums;

/**
 * 反向注册时前端显示agent操作系统类型和environments表中osType字段值
 *
 * @author z00613137
 * @since 2023-08-18
 */
public enum EnvironmentsOsTypeEnum {
    /**
     * Linux
     */
    LINUX("linux"),

    /**
     * Linux_AIX
     */
    AIX("aix"),

    /**
     * Linux_SOLARIS
     */
    SOLARIS("solaris");

    private final String osType;

    EnvironmentsOsTypeEnum(String osType) {
        this.osType = osType;
    }

    /**
     * 操作系统
     *
     * @return osType
     */
    public String getOsType() {
        return osType;
    }
}