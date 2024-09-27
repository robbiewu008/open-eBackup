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
 * 抽象的自动转换容量单位
 *
 * @author l90003110
 * @version V100R001C00
 * @since 2019-10-25
 */
public abstract class AbstractCapUnitType {
    /**
     * UNIT_KILO
     */
    protected static final long UNIT_KILO = 1024L;

    private double value;

    /**
     * 获取值与单位的连接串
     *
     * @param inputValue   数值
     * @param maxPrecision 支持小数位最大长度
     * @param minPrecision 支持小数位最小长度
     * @return 值与单位连接值
     */
    public String getAdaptedValueWithUnitByPrecision(double inputValue, int maxPrecision, int minPrecision) {
        AbstractCapUnitType adaptedCapUnitType = getAdaptedCapUnitType(inputValue);

        double adaptedValue = adaptedCapUnitType.getValue();

        // 默认2位小数
        String returnValue = MathUtil.parseNumber(adaptedValue, true, maxPrecision, minPrecision);

        return returnValue + " " + adaptedCapUnitType.getUnitName();
    }

    /**
     * 返回单位名称 如GB
     *
     * @return String [返回类型说明]
     * @since 2019-10-25
     */
    public abstract String getUnitName();

    /**
     * 获取后一个较大的单位
     *
     * @return AbstractCapUnitType [返回类型说明]
     */
    protected abstract AbstractCapUnitType getBiggerCapUnitType();

    /**
     * 向较大单位演进条件
     *
     * @return long [返回类型说明]
     */
    protected abstract long getBiggerConversion();

    /**
     * 获取前一个较小的单位
     *
     * @return AbstractCapUnitType [返回类型说明]
     */
    protected abstract AbstractCapUnitType getSmallerCapUnitType();

    /**
     * 向较小单位演进条件
     *
     * @return long [返回类型说明]
     */
    protected abstract long getSmallerConversion();

    /**
     * getValue
     *
     * @return double
     */
    protected double getValue() {
        return value;
    }

    /**
     * 自动获取合适的容量单位
     *
     * @param inputValue inputValue
     * @return AbstractCapUnitType [返回类型说明]
     */
    private AbstractCapUnitType getAdaptedCapUnitType(double inputValue) {
        // 向较大单位演进
        AbstractCapUnitType biggerCapUnit = getBiggerCapUnitType();
        long biggerConversion = getBiggerConversion();
        if (Math.abs(inputValue) >= biggerConversion && biggerCapUnit != null) {
            return biggerCapUnit.getAdaptedCapUnitType(inputValue / (double) biggerConversion);
        }

        // 向较小单位演进
        AbstractCapUnitType smallerCapUnit = getSmallerCapUnitType();
        long smallerConversion = getSmallerConversion();
        if (Math.abs(inputValue) < 1 && smallerCapUnit != null) {
            return smallerCapUnit.getAdaptedCapUnitType(inputValue * (double) smallerConversion);
        }

        this.value = inputValue;
        return this;
    }
}
