/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.sdk.protection.emuns;

import lombok.Getter;

import java.util.Arrays;

/**
 * SLA 备份策略action
 *
 * @author y00559272
 * @version [A8000 1.0.0]
 * @since 2021-01-03
 */
@Getter
public enum SlaPolicyActionEnum {
    /**
     * 全量备份
     */
    FULL("full"),
    /**
     * 日志备份
     */
    LOG("log"),
    /**
     * 累积增量
     */
    CUMULATIVE_INCREMENT("cumulative_increment"),
    /**
     * 差异增量
     */
    DIFFERENCE_INCREMENT("difference_increment"),
    /**
     * 永久增量
     */
    PERMANENT_INCREMENT("permanent_increment");

    /**
     * action名称
     */
    private final String name;

    SlaPolicyActionEnum(String name) {
        this.name = name;
    }

    /**
     * 根据Action名称获取Action枚举
     *
     * @param name action名称
     * @return SlaPolicyActionEnum
     */
    public static SlaPolicyActionEnum getActionByName(String name) {
        return Arrays.stream(SlaPolicyActionEnum.values())
                .filter(action -> action.name.equals(name))
                .findFirst()
                .orElseThrow(IllegalArgumentException::new);
    }
}
