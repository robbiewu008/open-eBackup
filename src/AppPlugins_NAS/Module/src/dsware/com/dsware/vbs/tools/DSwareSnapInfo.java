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
import java.util.AbstractMap;
import java.util.Collections;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * DSwareSnapInfo
 *
 */
public class DSwareSnapInfo {
    private static Method getSnapNameMethod;

    private static Method getFatherNameMethod;

    private static Method getStatusMethod;

    private static Method getSnapSizeMethod;

    private static Method getRealSizeMethod;

    private static Method getPoolIdMethod;

    private static Method getDeleteProrityMethod;

    private static Method getCreateTimeMethod;

    private String snapName;

    private String fatherName;

    private int status;

    private int snapSize;

    private int realSize;

    private int poolId;

    private int deletePrority;

    private long createTime;

    /**
     * DSwareSnapInfo
     *
     * @param object Object
     */
    public DSwareSnapInfo(Object object) {
        if (RuntimeLoad.isNotInstance(this, object)) {
            return;
        }
        try {
            this.setSnapName(String.valueOf(getSnapNameMethod.invoke(object)));
            this.setFatherName(String.valueOf(getFatherNameMethod.invoke(object)));
            this.setStatus((int) getStatusMethod.invoke(object));
            this.setSnapSize((int) getSnapSizeMethod.invoke(object));
            this.setRealSize((int) getRealSizeMethod.invoke(object));
            this.setPoolId((int) getPoolIdMethod.invoke(object));
            this.setDeletePrority((int) getDeleteProrityMethod.invoke(object));
            this.setCreateTime((long) getCreateTimeMethod.invoke(object));
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new DSwareException(e);
        }
    }

    /**
     * loadSnapInfoMethod
     *
     * @param dswareSnapInfoClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadSnapInfoMethod(Class<?> dswareSnapInfoClass) throws NoSuchMethodException {
        DSwareSnapInfo.getSnapNameMethod = dswareSnapInfoClass.getMethod("getSnapName");
        DSwareSnapInfo.getFatherNameMethod = dswareSnapInfoClass.getMethod("getFatherName");
        DSwareSnapInfo.getStatusMethod = dswareSnapInfoClass.getMethod("getStatus");
        DSwareSnapInfo.getSnapSizeMethod = dswareSnapInfoClass.getMethod("getSnapSize");
        DSwareSnapInfo.getRealSizeMethod = dswareSnapInfoClass.getMethod("getRealSize");
        DSwareSnapInfo.getPoolIdMethod = dswareSnapInfoClass.getMethod("getPoolId");
        DSwareSnapInfo.getDeleteProrityMethod = dswareSnapInfoClass.getMethod("getDeletePrority");
        DSwareSnapInfo.getCreateTimeMethod = dswareSnapInfoClass.getMethod("getCreateTime");
    }

    /**
     * transfer
     *
     * @param snapNameToSnapInfoMap String
     * @return DSwareSnapInfo>
     */
    public static Map<String, DSwareSnapInfo> transfer(Map<String, Object> snapNameToSnapInfoMap) {
        if (snapNameToSnapInfoMap == null || snapNameToSnapInfoMap.isEmpty()) {
            return Collections.emptyMap();
        }
        return snapNameToSnapInfoMap.values().stream().map(t -> {
            DSwareSnapInfo snapInfo = new DSwareSnapInfo(t);
            return new AbstractMap.SimpleEntry<>(snapInfo.getSnapName(), snapInfo);
        }).collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
    }

    public String getSnapName() {
        return this.snapName;
    }

    public final void setSnapName(final String snapName) {
        this.snapName = snapName;
    }

    public String getFatherName() {
        return this.fatherName;
    }

    public final void setFatherName(final String fatherName) {
        this.fatherName = fatherName;
    }

    public int getStatus() {
        return this.status;
    }

    public void setStatus(final int status) {
        this.status = status;
    }

    public int getSnapSize() {
        return this.snapSize;
    }

    public void setSnapSize(final int snapSize) {
        this.snapSize = snapSize;
    }

    public int getRealSize() {
        return this.realSize;
    }

    public void setRealSize(final int realSize) {
        this.realSize = realSize;
    }

    public int getPoolId() {
        return this.poolId;
    }

    public void setPoolId(final int poolId) {
        this.poolId = poolId;
    }

    public int getDeletePrority() {
        return this.deletePrority;
    }

    public void setDeletePrority(final int deletePrority) {
        this.deletePrority = deletePrority;
    }

    public long getCreateTime() {
        return this.createTime;
    }

    public void setCreateTime(final long createTime) {
        this.createTime = createTime;
    }
}
