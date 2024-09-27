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
 * 容量单位：PB
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public class PBCapUnitType extends AbstractCapUnitType {
    /**
     * 获得大的单位
     *
     * @return null
     */
    @Override
    protected AbstractCapUnitType getBiggerCapUnitType() {
        // 目前没有更大的单位了
        return null;
    }

    /**
     * 获得转换进制
     *
     * @return MAX_VALUE
     */
    @Override
    protected long getBiggerConversion() {
        return Long.MAX_VALUE;
    }

    /**
     * 获得小一位的单位
     *
     * @return GBCapUnitType
     */
    @Override
    protected AbstractCapUnitType getSmallerCapUnitType() {
        return new GBCapUnitType();
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
     * @return PB
     */
    @Override
    public String getUnitName() {
        return "PB";
    }
}
