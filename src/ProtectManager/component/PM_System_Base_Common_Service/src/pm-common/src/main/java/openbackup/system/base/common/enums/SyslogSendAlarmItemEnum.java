/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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
