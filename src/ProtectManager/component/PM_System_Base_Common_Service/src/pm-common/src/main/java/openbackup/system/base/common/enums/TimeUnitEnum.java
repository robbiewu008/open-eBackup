package openbackup.system.base.common.enums;

import lombok.Getter;

import java.util.Arrays;

/**
 * 周期类型时间枚举定义
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.0.0]
 * @since: 2020/10/9
 **/
@Getter
public enum TimeUnitEnum {
    MINUTES("m"),
    HOURS("h"),
    DAYS("d"),
    WEEKS("w"),
    MONTHS("MO"),
    YEARS("y");

    private final String unit;

    TimeUnitEnum(String unit) {
        this.unit = unit;
    }

    /**
     * 根据时间单位缩写获取对应枚举类
     *
     * @param unit 时间单位缩写
     * @return TimeUnitEnum
     */
    public static TimeUnitEnum getByUnit(String unit) {
        return Arrays.stream(TimeUnitEnum.values())
            .filter(timeUnit -> timeUnit.unit.equals(unit))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }
}
