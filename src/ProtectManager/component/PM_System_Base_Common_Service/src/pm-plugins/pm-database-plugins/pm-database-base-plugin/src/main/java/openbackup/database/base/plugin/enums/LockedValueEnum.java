/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.database.base.plugin.enums;

/**
 * 启用禁用枚举类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-14
 */
public enum LockedValueEnum {
    /**
     * 置灰
     */
    OPTIONAL("true"),

    /**
     * 不置灰
     */
    NO_OPTIONAL("false");

    private final String locked;

    /**
     * 构造方法
     *
     * @param locked 节点类型
     */
    LockedValueEnum(String locked) {
        this.locked = locked;
    }

    /**
     * 获取具体值方法
     *
     * @return 具体值
     */
    public String getLocked() {
        return locked;
    }
}
