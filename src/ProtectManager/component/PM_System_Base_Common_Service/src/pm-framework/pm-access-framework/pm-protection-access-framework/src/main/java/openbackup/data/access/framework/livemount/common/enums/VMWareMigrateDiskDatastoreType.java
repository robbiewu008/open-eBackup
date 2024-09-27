package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * VMWare 迁移磁盘配置dataStore枚举
 *
 * @author h30003246
 * @since 2020-12-31
 */
public enum VMWareMigrateDiskDatastoreType {
    /**
     * 磁盘和虚拟机配置不同的datastore
     */
    DIFFERENT_DATASTORE("different_datastore"),

    /**
     * 磁盘和虚拟机配置相同的datastore
     */
    SAME_DATASTORE("same_datastore");

    private final String name;

    VMWareMigrateDiskDatastoreType(String name) {
        this.name = name;
    }

    @JsonValue

    public String getName() {
        return name;
    }

    /**
     * get Scheduled type enum
     *
     * @param str str
     * @return RetentionUnit
     */
    public static VMWareMigrateDiskDatastoreType get(String str) {
        return EnumUtil.get(VMWareMigrateDiskDatastoreType.class, VMWareMigrateDiskDatastoreType::getName, str);
    }
}
