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
 * 容量工具类
 *
 * @author cKF17701
 * @version V100R001C00
 * @since 2019-10-25
 */
public class CapUnit {
    private double capacity;

    private String unit;

    /**
     * 构造函数
     *
     * @param capacity capacity
     * @param unit     unit
     */
    public CapUnit(double capacity, String unit) {
        this.capacity = capacity;
        this.unit = unit;
    }

    /**
     * 获取容量
     *
     * @return double
     */
    public double getCapacity() {
        return capacity;
    }

    /**
     * 设置容量
     *
     * @param capacity 容量
     */
    public void setCapacity(double capacity) {
        this.capacity = capacity;
    }

    /**
     * 获得单位
     *
     * @return String unit
     */
    public String getUnit() {
        return unit;
    }

    /**
     * 设置单位
     *
     * @param unit 设置单位
     */
    public void setUnit(String unit) {
        this.unit = unit;
    }
}
