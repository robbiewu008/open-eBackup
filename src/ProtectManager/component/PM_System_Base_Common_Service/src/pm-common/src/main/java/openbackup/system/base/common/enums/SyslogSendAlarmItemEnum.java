package openbackup.system.base.common.enums;

import lombok.Getter;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;
import java.util.List;

/**
 * Syslog上报告警类型枚举类
 *
 * @author l00617376
 * @version OceanCyber 300 1.2.0
 * @since 2024-05-31
 */
@Getter
public enum SyslogSendAlarmItemEnum {
    /**
     * 不发送
     */
    NONE(0),

    /**
     * 发送告警
     */
    ALARM(1),

    /**
     * 发送告警恢复
     */
    RECOVERY(2),

    /**
     * 发送事件
     */
    EVENT(4);


    private final int value;

    SyslogSendAlarmItemEnum(int value) {
        this.value = value;
    }

    /**
     * 根据value获取SyslogSendAlarmItem
     *
     * @param value value
     * @return SyslogSendAlarmItem
     */
    public static SyslogSendAlarmItemEnum getByValue(int value) {
        for (SyslogSendAlarmItemEnum syslogSendAlarmItemEnum : SyslogSendAlarmItemEnum.values()) {
            if (syslogSendAlarmItemEnum.getValue() == value) {
                return syslogSendAlarmItemEnum;
            }
        }
        return NONE;
    }

    /**
     * parseItemsFromValue
     *
     * @param itemValue itemValue
     * @return List<SyslogSendAlarmItem>
     */
    public static int[] parseItemsFromValue(int itemValue) {
        List<Integer> result = new ArrayList<>();
        int itemValueTmp = itemValue;
        SyslogSendAlarmItemEnum[] syslogSendAlarmItemEnums = SyslogSendAlarmItemEnum.values();
        Arrays.sort(syslogSendAlarmItemEnums, Collections.reverseOrder(
            Comparator.comparingLong(SyslogSendAlarmItemEnum::getValue)));

        for (SyslogSendAlarmItemEnum syslogSendAlarmItemEnum : syslogSendAlarmItemEnums) {
            int syslogSendAlarmItemValue = syslogSendAlarmItemEnum.getValue();
            if (itemValueTmp >= syslogSendAlarmItemValue) {
                result.add(syslogSendAlarmItemEnum.getValue());
                itemValueTmp = itemValueTmp - syslogSendAlarmItemValue;
                if (itemValueTmp == 0) {
                    break;
                }
            }
        }
        return result.stream().mapToInt(Integer::intValue).toArray();
    }
}
