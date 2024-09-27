package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * 任务类型枚举
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-04-22
 */
@Getter
public enum NetPlaneTypeEnum {
    /**
     * 备份
     */
    BACKUP("backup"),
    /**
     * 复制
     */
    REPLICATION("replication"),
    /**
     * 归档
     */
    ARCHIVE("archive");

    /**
     * action名称
     */
    private final String name;

    NetPlaneTypeEnum(String name) {
        this.name = name;
    }
}
