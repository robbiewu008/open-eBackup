package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 副本保留策略枚举定义
 *
 * @author h30003246
 * @since 2020-09-22
 */
public enum RetentionType {
    /**
     * 永久保留
     */
    PERMANENT("permanent"),

    /**
     * 固定时间
     */
    FIXED_TIME("fixed_time"),

    /**
     * 最后一个
     */
    LATEST_ONE("latest_one");

    private final String name;

    RetentionType(String name) {
        this.name = name;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    /**
     * get retention type enum
     *
     * @param str str
     * @return RetentionUnit
     */
    @JsonCreator
    public static RetentionType get(String str) {
        return EnumUtil.get(RetentionType.class, RetentionType::getName, str);
    }
}
