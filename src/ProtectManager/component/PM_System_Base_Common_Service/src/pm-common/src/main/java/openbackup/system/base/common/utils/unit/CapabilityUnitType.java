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
 * 容量单位
 *
 */
public enum CapabilityUnitType implements IUnitType {
    /**
     * bit
     */
    BIT(1L),

    /**
     * byte
     */
    BYTE(8L),

    /**
     * KB
     */
    KB(1024L * 8L),

    /**
     * MB
     */
    MB((1024L * 1024L) * 8L),

    /**
     * GB
     */
    GB((1024L * 1024L * 1024L) * 8L),

    /**
     * TB
     */
    TB((1024L * 1024L * 1024L * 1024L) * 8L),

    /**
     * PB
     */
    PB((1024L * 1024L * 1024L * 1024L * 1024L) * 8L);

    private final long scale;

    private final boolean isGroupUsed = Boolean.TRUE;

    CapabilityUnitType(long scale) {
        this.scale = scale;
    }

    /**
     * getUnit
     *
     * @return long
     */
    public long getUnit() {
        return this.scale;
    }

    /**
     * isGroupingUsed
     *
     * @return boolean
     */
    @Override
    public boolean isGroupingUsed() {
        return isGroupUsed;
    }
}
