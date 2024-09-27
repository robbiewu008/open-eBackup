/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.backup;

import openbackup.system.base.util.EnumUtil;

/**
 * 功能描述
 *
 * @author l00422407
 * @since 2020-12-04
 */
public enum BackupTypeConstants {
    /**
     * 1：全量备份
     */
    FULL(1, "full"),
    /**
     * 2：差异备份
     */
    CUMULATIVE_INCREMENT(3, "cumulative_increment"),
    /**
     * 3：增量备份
     */
    DIFFERENCE_INCREMENT(2, "difference_increment"),
    /**
     * 4：事务日志备份
     */
    LOG(4, "log"),
    /**
     * 5：永久增量备份
     */
    PERMANENT_INCREMENT(5, "permanent_increment"),
    /**
     * 6：快照备份
     */
    NATIVE_SNAPSHOT(6, "snapshot");

    /**
     * dme返回的备份类型
     */
    private final int abBackupType;

    /**
     * 备份类型对应的String（数据库中存储的是字符串）
     */
    private final String backupType;

    /**
     * DME对应的全量备份
     */
    public static final String DME_BACKUP_FULL = "fullBackup";

    /**
     * DME对应的差异备份
     */
    public static final String DME_BACKUP_DIFF = "diffBackup";

    /**
     * DME对应的增量备份
     */
    public static final String DME_BACKUP_INCREMENT = "incrementBackup";

    /**
     * DME对应的日志备份
     */
    public static final String DME_BACKUP_LOG = "logBackup";

    /**
     * DME对应的永久增量备份
     */
    public static final String DME_BACKUP_FOREVER_INCREMENT = "foreverIncrementBackup";

    BackupTypeConstants(int abBackupType, String backupType) {
        this.abBackupType = abBackupType;
        this.backupType = backupType;
    }

    public int getAbBackupType() {
        return abBackupType;
    }

    public String getBackupType() {
        return backupType;
    }

    /**
     * 根据abBackupType获取枚举值
     *
     * @param backupType int类型
     * @return BackupTypeConstants
     */
    public static BackupTypeConstants getBackupTypeByAbBackupType(int backupType) {
        return EnumUtil.get(BackupTypeConstants.class, BackupTypeConstants::getAbBackupType, backupType);
    }

    /**
     * 爱数返回的int类型转换为数据库存档的String类型
     *
     * @param abBackupType 爱数返回的int类型
     * @return String backupType
     */
    public static String convert2BackupType(int abBackupType) {
        switch (abBackupType) {
            case 1:
                return FULL.backupType;
            case 2:
                return DIFFERENCE_INCREMENT.backupType;
            case 3:
                return CUMULATIVE_INCREMENT.backupType;
            case 4:
                return LOG.backupType;
            case 5:
                return PERMANENT_INCREMENT.backupType;
            case 6:
                return NATIVE_SNAPSHOT.backupType;
            default:
                return "";
        }
    }

    /**
     * dme返回的int类型转换为数据库存档的String类型 oracle vmware都适用
     *
     * @param dmeBackupType dme返回的int类型
     * @return String backupType
     */
    public static String convert2dmeBackupType(int dmeBackupType) {
        switch (dmeBackupType) {
            case 0:
                return FULL.backupType;
            case 1:
                return DIFFERENCE_INCREMENT.backupType;
            case 2:
                return CUMULATIVE_INCREMENT.backupType;
            default:
                return "";
        }
    }

    /**
     * 数据库存档的String类型转换为爱数返回的int类型,属于枚举类BackupTypeConstants
     * NATIVE_SNAPSHOT；FULL；CUMULATIVE_INCREMENT；DIFFERENCE_INCREMENT；LOG；PERMANENT_INCREMENT
     *
     * @param backupType 数据库存档的String类型
     * @return int abBackupType
     */
    public static int convert2AbBackType(String backupType) {
        switch (backupType) {
            case "full":
                return FULL.abBackupType;
            case "cumulative_increment":
                return CUMULATIVE_INCREMENT.abBackupType;
            case "difference_increment":
                return DIFFERENCE_INCREMENT.abBackupType;
            case "log":
                return LOG.abBackupType;
            case "permanent_increment":
                return PERMANENT_INCREMENT.abBackupType;
            case "snapshot":
                return NATIVE_SNAPSHOT.abBackupType;
            default:
                return -1;
        }
    }

    /**
     * 转换DME的备份方式
     *
     * @param backupType 数据库存档的String类型
     * @return String DME的备份方式
     */
    public static String convert2DmeBackType(String backupType) {
        switch (backupType) {
            case "cumulative_increment": // 差异备份
                return "diffBackup";
            case "difference_increment": // 增量备份
                return "incrementBackup";
            case "log": // 日志备份
                return "logBackup";
            case "permanent_increment": // 永久增量备份
                return "foreverIncrementBackup";
            default:
                return "fullBackup";
        }
    }

    /**
     * DME的备份方式转化回来
     *
     * @param dmeBackupType dme的备份方式
     * @return pm定义的备份参数
     */
    public static String dmeBackTypeConvertBack(String dmeBackupType) {
        switch (dmeBackupType) {
            case DME_BACKUP_DIFF: // 差异备份
                return CUMULATIVE_INCREMENT.getBackupType();
            case DME_BACKUP_INCREMENT: // 增量备份
                return DIFFERENCE_INCREMENT.getBackupType();
            case DME_BACKUP_LOG: // 日志备份
                return LOG.getBackupType();
            case DME_BACKUP_FOREVER_INCREMENT: // 永久增量备份
                return PERMANENT_INCREMENT.getBackupType();
            default:
                return FULL.getBackupType();
        }
    }
}
