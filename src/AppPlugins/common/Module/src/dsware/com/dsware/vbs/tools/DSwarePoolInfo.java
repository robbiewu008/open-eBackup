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
package com.dsware.vbs.tools;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * DSwarePoolInfo
 *
 */
public class DSwarePoolInfo {
    private static Method getPoolIdMethod;

    private static Method getTotalCapacityMethod;

    private static Method getUsedCapacityMethod;

    private static Method getAllocCapacityMethod;

    private int poolId;

    private long totalCapacity;

    private long usedCapacity;

    private long allocCapacity;

    /**
     * DSwarePoolInfo
     *
     * @param object Object
     */
    public DSwarePoolInfo(Object object) {
        if (RuntimeLoad.isNotInstance(this, object)) {
            return;
        }
        try {
            this.setPoolId((int) getPoolIdMethod.invoke(object));
            this.setTotalCapacity((long) getTotalCapacityMethod.invoke(object));
            this.setUsedCapacity((long) getUsedCapacityMethod.invoke(object));
            this.setAllocCapacity((long) getAllocCapacityMethod.invoke(object));
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new DSwareException(e);
        }
    }

    /**
     * loadPoolInfoMethod
     *
     * @param dswarePoolInfoClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadPoolInfoMethod(Class<?> dswarePoolInfoClass) throws NoSuchMethodException {
        DSwarePoolInfo.getPoolIdMethod = dswarePoolInfoClass.getMethod("getPoolId");
        DSwarePoolInfo.getTotalCapacityMethod = dswarePoolInfoClass.getMethod("getTotalCapacity");
        DSwarePoolInfo.getUsedCapacityMethod = dswarePoolInfoClass.getMethod("getUsedCapcity");
        DSwarePoolInfo.getAllocCapacityMethod = dswarePoolInfoClass.getMethod("getAllocCapacity");
    }

    public int getPoolId() {
        return this.poolId;
    }

    public void setPoolId(int poolId) {
        this.poolId = poolId;
    }

    public long getTotalCapacity() {
        return this.totalCapacity;
    }

    public void setTotalCapacity(long totalCapacity) {
        this.totalCapacity = totalCapacity;
    }

    public long getUsedCapcity() {
        return this.usedCapacity;
    }

    public void setUsedCapacity(long usedCapacity) {
        this.usedCapacity = usedCapacity;
    }

    public long getAllocCapacity() {
        return this.allocCapacity;
    }

    public void setAllocCapacity(long allocCapacity) {
        this.allocCapacity = allocCapacity;
    }
}
