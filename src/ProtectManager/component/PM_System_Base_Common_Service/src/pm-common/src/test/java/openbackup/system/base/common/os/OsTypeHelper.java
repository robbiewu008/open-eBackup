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
package openbackup.system.base.common.os;

import openbackup.system.base.common.os.OsTypeUtil;
import openbackup.system.base.common.os.enums.OsType;

import lombok.extern.slf4j.Slf4j;

import java.lang.reflect.Field;
import java.lang.reflect.Modifier;

/**
 * 操作系统类型助手
 *
 * @author w00493811
 * @since 2021-08-23
 */
@Slf4j
public class OsTypeHelper {
    /**
     * 修改操作系统类型
     *
     * @param osType OS Type
     */
    public static void modifyOsTypeUtilOsName(OsType osType) {
        modifyOsTypeUtilOsName(osType.name().toLowerCase());
    }

    /**
     * 修改操作系统类型
     *
     * @param osName 操作系统名称
     */
    public static void modifyOsTypeUtilOsName(String osName) {
        try {
            Field field = OsTypeUtil.class.getDeclaredField("OS_NAME");
            field.setAccessible(true);
            int nonFinal = field.getModifiers() & (~Modifier.FINAL);
            Field modifiers = Field.class.getDeclaredField("modifiers");
            modifiers.setAccessible(true);
            modifiers.setInt(field, nonFinal);
            field.set(null, osName);
        } catch (NoSuchFieldException | IllegalAccessException e) {
            log.error(e.getMessage(), e);
        }
    }
}
