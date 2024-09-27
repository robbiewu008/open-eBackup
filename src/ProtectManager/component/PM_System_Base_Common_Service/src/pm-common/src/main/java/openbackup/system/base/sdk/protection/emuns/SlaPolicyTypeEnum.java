/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.protection.emuns;

import lombok.Getter;

import java.util.Arrays;

/**
 * SLA 备份策略类型
 *
 * @author y00559272
 * @version [A8000 1.0.0]
 * @since 2021-01-03
 */
@Getter
public enum SlaPolicyTypeEnum {
    /**
     * 备份
     */
    BACKUP("backup"),
    /**
     * 归档
     */
    ARCHIVING("archiving"),
    /**
     * 复制
     */
    REPLICATION("replication");

    /**
     * action名称
     */
    private final String name;

    SlaPolicyTypeEnum(String name) {
        this.name = name;
    }

    /**
     * 根据Type名称获取Type枚举
     *
     * @param name type名称
     * @return SlaPolicyTypeEnum
     */
    public static SlaPolicyTypeEnum getTypeByName(String name) {
        return Arrays.stream(SlaPolicyTypeEnum.values())
                .filter(type -> type.name.equals(name))
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
