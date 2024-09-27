package openbackup.data.access.framework.livemount.common.enums;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 副本选择策略枚举定义
 *
 * @author h30003246
 * @since 2020-09-22
 */
public enum CopyDataSelection {
    /**
     * 最近一小时
     */
    LAST_HOUR("last_hour", "h"),

    /**
     * 最近一天
     */
    LAST_DAY("last_day", "d"),

    /**
     * 最近一周
     */
    LAST_WEEK("last_week", "w"),

    /**
     * 最近一月
     */
    LAST_MONTH("last_month", "MO"),

    /**
     * 总是最新
     */
    LATEST("latest", null);

    private String name;

    private String unit;

    CopyDataSelection(String name, String unit) {
        this.name = name;
        this.unit = unit;
    }

    @JsonValue
    public String getName() {
        return name;
    }

    public String getUnit() {
        return unit;
    }

    /**
     * get copy data selection unit enum
     *
     * @param str str
     * @return CopyDataSelection
     */
    @JsonCreator
    public static CopyDataSelection get(String str) {
        for (CopyDataSelection typeEnum : CopyDataSelection.values()) {
            if (typeEnum.getName().equals(str)) {
                return typeEnum;
            }
        }
        throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
    }
}
