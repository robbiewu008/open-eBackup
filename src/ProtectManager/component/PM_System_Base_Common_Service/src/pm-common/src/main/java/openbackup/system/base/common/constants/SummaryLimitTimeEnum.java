package openbackup.system.base.common.constants;

import com.fasterxml.jackson.annotation.JsonValue;
import com.google.common.collect.ImmutableMap;

import java.util.Map;

/**
 * 演练任务统计期限枚举类
 *
 * @author x30046484
 * @since 2023-11-14
 */
public enum SummaryLimitTimeEnum {
    /**
     * 最近一天
     */
    LATEST_ONE_DAY("1d"),

    /**
     * 最近一周
     */
    LATEST_ONE_WEEK("1w"),

    /**
     * 最近一个月
     */
    LATEST_ONE_MONTH("1m"),

    /**
     * 最近半年
     */
    LATEST_HALF_YEAR("0.5y");

    private final String value;

    /**
     * 统计时限map
     */
    public static final Map<String, SummaryLimitTimeEnum> enumMap =
        ImmutableMap.of("1d", SummaryLimitTimeEnum.LATEST_ONE_DAY, "1w", SummaryLimitTimeEnum.LATEST_ONE_WEEK, "1m",
            SummaryLimitTimeEnum.LATEST_ONE_MONTH, "0.5y", SummaryLimitTimeEnum.LATEST_HALF_YEAR);

    SummaryLimitTimeEnum(String value) {
        this.value = value;
    }

    /**
     * 根据String获取统计时限枚举值对象
     *
     * @param value 枚举值String
     * @return 枚举对象
     */
    public static SummaryLimitTimeEnum getEnumByString(String value) {
        return enumMap.get(value);
    }

    @JsonValue
    public String getValue() {
        return value;
    }
}
