/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 */

package openbackup.system.base.common.enums;

import lombok.Getter;

/**
 * Dorado告警枚举
 *
 * @author y30000858
 * @since 2021-01-12
 */
@Getter
public enum DoradoAlarmLevelEnum {
    /**
     * 提示
     */
    INFO(2),
    /**
     * 警告
     */
    WARNING(3),
    /**
     * 重要
     */
    MAJOR(5),
    /**
     * 紧急
     */
    CRITICAL(6);

    private final Integer alarmLevel;

    DoradoAlarmLevelEnum(Integer alarmLevel) {
        this.alarmLevel = alarmLevel;
    }

    /**
     * 获取告警级别String
     *
     * @param value value
     * @return 枚举值
     */
    public static DoradoAlarmLevelEnum getSeverity(int value) {
        for (DoradoAlarmLevelEnum severity : DoradoAlarmLevelEnum.values()) {
            if (value == severity.alarmLevel) {
                return severity;
            }
        }

        return DoradoAlarmLevelEnum.INFO;
    }
}
