/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.enums;

import java.util.Locale;

/**
 * SDK中使用的BackupType枚举类
 *
 * @author y00559272
 * @since 2021-10-11
 */
public enum BackupTypeEnum {
    FULL(1, 0),
    /**
     * 累积增量备份--对应差异备份
     */
    CUMULATIVE_INCREMENT(3, 2),

    /**
     * 差异增量备份--对应增量备份
     */
    DIFFERENCE_INCREMENT(2, 1),
    LOG(4, 3),

    /**
     * 永久增量备份
     */
    PERMANENT_INCREMENT(5, 6),

    /**
     * 快照备份
     */
    SNAPSHOT(7, 8);

    /**
     * 备份类型
     */
    private int abbreviation;

    /**
     * DMC备份类型
     */
    private int abbreviationDmc;

    BackupTypeEnum(int abbreviation, int abbreviationDmc) {
        this.abbreviation = abbreviation;
        this.abbreviationDmc = abbreviationDmc;
    }

    /**
     * 获取枚举值
     *
     * @return int
     */
    public int getAbbreviation() {
        return abbreviation;
    }

    /**
     * 获取枚举值 Dmc
     *
     * @return int
     */
    public int getAbbreviationDmc() {
        return abbreviationDmc;
    }

    /**
     * name的小写
     *
     * @return name的小写
     */
    public String lower() {
        return name().toLowerCase(Locale.ROOT);
    }
}
