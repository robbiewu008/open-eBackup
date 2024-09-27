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
 * 容量单位：MB
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public class MBCapUnitType extends AbstractCapUnitType {
    /**
     * 获得大的单位
     *
     * @return GBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // GB
        return new GBCapUnitType();
    }

    /**
     * 获得转换进制
     *
     * @return UNIT_KILO
     */
    @Override
    protected long getBiggerConversion() {
        return UNIT_KILO;
    }

    /**
     * 获得小一位的单位
     *
     * @return KBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        // KB
        return new KBCapUnitType();
    }

    /**
     * 向小转换的进制
     *
     * @return UNIT_KILO
     */
    @Override
    protected long getSmallerConversion() {
        return UNIT_KILO;
    }

    /**
     * 单位名称
     *
     * @return MB
     */
    @Override
    public String getUnitName() {
        return "MB";
    }
}
