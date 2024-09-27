/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.constant;

/**
 * Openstack 域可见性枚举类
 *
 * @author c30016231
 * @since 2023-03-13
 */
public enum OpenstackDomainVisibleEnum {
    /**
     * 可见
     */
    VISIBLE("1"),
    /**
     * 不可见
     */
    INVISIBLE("0"),
    /**
     * 其他
     */
    OTHER("OTHER");

    private final String code;

    OpenstackDomainVisibleEnum(String code) {
        this.code = code;
    }

    public String getCode() {
        return code;
    }
}
