package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 策略调度周期枚举定义
 *
 * @author h30003246
 * @since 2020-09-22
 */
public enum ScheduledType {
    /**
     * 按时间间隔
     */
    PERIOD_SCHEDULE("period_schedule"),

    /**
     * 按副本生成后执行
     */
    AFTER_BACKUP_DONE("after_backup_done");

    private final String name;

    ScheduledType(String name) {
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
    public static ScheduledType get(String str) {
        return EnumUtil.get(ScheduledType.class, ScheduledType::getName, str);
    }
}
