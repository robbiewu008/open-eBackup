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
package openbackup.system.base.common.utils.unit;

/**
 * 容量单位：Byte
 *
 */
public class ByteCapUnitType extends AbstractCapUnitType {
    /**
     * 获取KBCapUnitType值
     *
     * @return KBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // KB
        return new KBCapUnitType();
    }

    /**
     * 获取BiggerConversion值
     *
     * @return UNIT_KILO
     */
    @Override
    protected long getBiggerConversion() {
        return UNIT_KILO;
    }

    /**
     * 获取SmallerCapUnitType。默认为null
     *
     * @return AbstractCapUnitType
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        // Bit
        // 最小单位裁定为Byte
        return null;
    }

    /**
     * 获取小的转换
     *
     * @return Long.MIN_VALUE
     */
    @Override
    protected long getSmallerConversion() {
        return Long.MIN_VALUE;
    }

    /**
     * 获取单位名称
     *
     * @return Byte
     */
    @Override
    public String getUnitName() {
        return "Byte";
    }
}
