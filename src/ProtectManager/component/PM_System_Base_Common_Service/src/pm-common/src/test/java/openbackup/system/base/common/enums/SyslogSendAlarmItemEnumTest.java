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

import openbackup.system.base.common.enums.SyslogSendAlarmItemEnum;

import org.junit.Assert;
import org.junit.jupiter.api.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.stream.IntStream;

/**
 * {@link SyslogSendAlarmItemEnum} 测试类
 *
 */
@RunWith(PowerMockRunner.class)
class SyslogSendAlarmItemEnumTest {
    /**
     * 用例名称：通过告警类型枚举类型获取int值
     * 前置条件：无
     * 检查点：返回值符合预期
     */
    @Test
    void getByValue() {
        Assert.assertEquals(1, SyslogSendAlarmItemEnum.ALARM.getValue());
    }

    /**
     * 用例名称：通过告警类型数组算的int值的和获取告警类型枚举类型
     * 前置条件：无
     * 检查点：返回值符合预期
     */
    @Test
    void parseItemsFromValue() {
        int[] testAlarmTypeArray =
            new int[]{SyslogSendAlarmItemEnum.EVENT.getValue(), SyslogSendAlarmItemEnum.ALARM.getValue()};
        int sum = IntStream.of(testAlarmTypeArray).sum();
        int[] res = SyslogSendAlarmItemEnum.parseItemsFromValue(sum);
        Assert.assertArrayEquals(testAlarmTypeArray, res);
    }

    /**
     * 用例名称：通过int值获取告警类型枚举类型
     * 前置条件：无
     * 检查点：返回值符合预期
     */
    @Test
    void getValue() {
        SyslogSendAlarmItemEnum test = SyslogSendAlarmItemEnum.getByValue(1);
        Assert.assertEquals(SyslogSendAlarmItemEnum.ALARM, test);
    }
}