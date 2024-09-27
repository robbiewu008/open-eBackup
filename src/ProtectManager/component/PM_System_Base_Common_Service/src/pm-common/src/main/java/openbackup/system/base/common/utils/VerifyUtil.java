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

import java.util.Collection;
import java.util.Map;

/**
 * 本类用于对常见的集合、String、Map、一维数组、二维数组等进行校验
 *
 * @author w90002860
 * @version V100R001C00
 * @since 2019-11-01
 */
public final class VerifyUtil {
    /**
     * 默认构造函数
     */
    private VerifyUtil() {
    }

    /**
     * 判断Collection子类元素是否为空集合
     *
     * @param collection 集合对象
     * @return boolean [true：空，false：非空]
     * @author lKF20890
     */
    public static boolean isEmpty(Collection<?> collection) {
        return (collection == null) || (collection.isEmpty());
    }

    /**
     * 判断对象是否为空
     *
     * @param obj 对象
     * @return boolean [true：空，false：非空]
     * @author w90002860
     */
    public static boolean isEmpty(Object obj) {
        return (obj == null) || (obj instanceof String && ((String) obj).isEmpty());
    }

    /**
     * 判断Map对象是否为空
     *
     * @param map Map对象
     * @return boolean [true：空，false：非空]
     * @author w90002860
     */
    public static boolean isEmpty(Map<?, ?> map) {
        return (map == null) || (map.isEmpty());
    }

    /**
     * 判断String对象是否为null、空字符串、空格串
     *
     * @param string 字符串
     * @return boolean [true：空，false：非空]
     * @author w90002860
     */
    public static boolean isEmpty(String string) {
        return (string == null) || (string.trim().isEmpty());
    }

    /**
     * 判断字符串是否为null、空字符串，不trim
     *
     * @param string 字符串
     * @return boolean [true：空，false：非空]
     */
    public static boolean isNone(String string) {
        return string == null || string.isEmpty();
    }

    /**
     * 判断Object数组是否为空
     *
     * @param objects 数组
     * @return boolean [返回类型说明]
     * @author z90005513
     */
    public static boolean isEmpty(Object[] objects) {
        return objects == null || objects.length == 0;
    }

    /**
     * 判断一个数组内的对象是否存在为空,如果存在空返回true；全都不为空，返回false
     *
     * @param objects 对象
     * @return boolean [true：空，false：非空]
     * @author w90002860
     */
    public static boolean isAnyEmpty(Object[] objects) {
        for (Object object : objects) {
            if (isEmpty(object)) {
                return true;
            }
        }
        return false;
    }
}
