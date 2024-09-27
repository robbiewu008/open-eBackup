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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.unit.CapabilityUnitType;
import openbackup.system.base.common.utils.unit.UnitConvert;

import org.apache.commons.lang3.StringUtils;

import java.util.HashMap;
import java.util.Map;
import java.util.regex.Pattern;

/**
 * 任务速度转换器
 *
 * @author w00616953
 * @since 2021-12-10
 */
public final class JobSpeedConverter {
    private static final String INTEGER_REGEX = "^[-\\+]?[\\d]*$";

    private static final String JOB_SPEED_UNIT = "/s";

    private static final String JOB_SPEED_REGEX = "B/s";

    private static final String JOB_SPEED_SPLIT_REGEX = " ";

    private static final Map<String, String> SPEED_UNIT_CONVERT_MAP = new HashMap<>();

    static {
        SPEED_UNIT_CONVERT_MAP.put("KiB/s", "KB/s");
        SPEED_UNIT_CONVERT_MAP.put("MiB/s", "MB/s");
        SPEED_UNIT_CONVERT_MAP.put("GiB/s", "GB/s");
        SPEED_UNIT_CONVERT_MAP.put("TiB/s", "TB/s");
        SPEED_UNIT_CONVERT_MAP.put("PiB/s", "PB/s");
    }

    /**
     * 转换速度格式
     *
     * @param speed 更新任务上报的速度
     * @return pm展示时的速度格式
     */
    public static String convertJobSpeed(String speed) {
        String convertedSpeed = speed;
        // 如果上报的是速度具体数字，需要转化为相应的单位。
        if (Pattern.compile(INTEGER_REGEX).matcher(convertedSpeed).matches()) {
            // if speed less than 0, than set speed is null
            if (Long.parseLong(convertedSpeed) < 0) {
                return StringUtils.EMPTY;
            } else {
                String val =
                        UnitConvert.autoConvertToAdaptedValueAndUnit(
                                Double.parseDouble(convertedSpeed),
                                CapabilityUnitType.KB,
                                IsmNumberConstant.TWO,
                                IsmNumberConstant.ZERO);
                return val + JOB_SPEED_UNIT;
            }
        } else if (convertedSpeed.contains(JOB_SPEED_REGEX)) {
            String[] speedArray = convertedSpeed.split(JOB_SPEED_SPLIT_REGEX);
            if (speedArray.length > 1 && SPEED_UNIT_CONVERT_MAP.containsKey(speedArray[1])) {
                convertedSpeed = speedArray[0] + JOB_SPEED_SPLIT_REGEX + SPEED_UNIT_CONVERT_MAP.get(speedArray[1]);
            }
            return convertedSpeed;
        } else {
            return convertedSpeed;
        }
    }
}
