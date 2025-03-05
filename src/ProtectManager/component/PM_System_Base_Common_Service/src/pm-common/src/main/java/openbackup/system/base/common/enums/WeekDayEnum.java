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

import openbackup.system.base.common.utils.VerifyUtil;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 周枚举
 *
 */
public enum WeekDayEnum {
    /**
     * 周日
     */
    SUN("sun", "1"),

    /**
     * 周一
     */
    MON("mon", "2"),

    /**
     * 周二
     */
    TUE("tue", "3"),

    /**
     * 周三
     */
    WED("wed", "4"),

    /**
     * 周四
     */
    THU("thu", "5"),

    /**
     * 周五
     */
    FRI("fri", "6"),

    /**
     * 周六
     */
    SAT("sat", "7");

    private final String name;

    private final String index;

    WeekDayEnum(String name, String index) {
        this.name = name;
        this.index = index;
    }

    public String getName() {
        return name;
    }

    public String getIndex() {
        return index;
    }

    /**
     * 转换周日期字符串
     * “sun,mon,tue,wed"
     * convert to
     * "1,2,3,4"
     *
     * @param nameString 周字符串
     * @return index 序号字符串
     */
    public static List<String> convertName2Index(List<String> nameString) {
        if (VerifyUtil.isEmpty(nameString)) {
            return nameString;
        }
        Map<String, String> nameIndexMap = Stream.of(WeekDayEnum.values())
            .collect(Collectors.toMap(WeekDayEnum::getName, WeekDayEnum::getIndex));
        List<String> index = new ArrayList<>();
        for (String name : nameString) {
            index.add(nameIndexMap.get(name));
        }
        return index;
    }

    /**
     * 判断是否存在该日期
     *
     * @param name 日期名称
     * @return 是否
     */
    public static boolean contains(String name) {
        return Arrays.stream(WeekDayEnum.values()).map(WeekDayEnum::getName).collect(Collectors.toSet()).contains(name);
    }
}
