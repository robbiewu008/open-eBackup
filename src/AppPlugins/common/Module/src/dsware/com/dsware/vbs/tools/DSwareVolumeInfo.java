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
 * DSwareVolumeInfo
 *
 * @since 2024-08-27
 */
public class DSwareVolumeInfo {
    private static Method getVolNameMethod;

    private static Method getFatherNameMethod;

    private static Method getStatusMethod;

    private static Method getVolSizeMethod;

    private static Method getRealSizeMethod;

    private static Method getPoolIdMethod;

    private static Method getCreateTimeMethod;

    private String volName;

    private String fatherName;

    private int status;

    private int volSize;

    private int realSize;

    private int poolId;

    private long createTime;

    /**
     * DSwareVolumeInfo
     *
     * @param object Object
     */
    public DSwareVolumeInfo(Object object) {
        if (RuntimeLoad.isNotInstance(this, object)) {
            return;
        }
        try {
            this.setVolName(String.valueOf(getVolNameMethod.invoke(object)));
            this.setFatherName(String.valueOf(getFatherNameMethod.invoke(object)));
            this.setStatus((int) getStatusMethod.invoke(object));
            this.setVolSize((int) getVolSizeMethod.invoke(object));
            this.setRealSize((int) getRealSizeMethod.invoke(object));
            this.setPoolId((int) getPoolIdMethod.invoke(object));
            this.setCreateTime((long) getCreateTimeMethod.invoke(object));
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new DSwareException(e);
        }
    }

    /**
     * loadVolumeInfoMethod
     *
     * @param dswareVolumeInfoClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadVolumeInfoMethod(Class<?> dswareVolumeInfoClass) throws NoSuchMethodException {
        DSwareVolumeInfo.getVolNameMethod = dswareVolumeInfoClass.getMethod("getVolName");
        DSwareVolumeInfo.getFatherNameMethod = dswareVolumeInfoClass.getMethod("getFatherName");
        DSwareVolumeInfo.getStatusMethod = dswareVolumeInfoClass.getMethod("getStatus");
        DSwareVolumeInfo.getVolSizeMethod = dswareVolumeInfoClass.getMethod("getVolSize");
        DSwareVolumeInfo.getRealSizeMethod = dswareVolumeInfoClass.getMethod("getRealSize");
        DSwareVolumeInfo.getPoolIdMethod = dswareVolumeInfoClass.getMethod("getPoolId");
        DSwareVolumeInfo.getCreateTimeMethod = dswareVolumeInfoClass.getMethod("getCreateTime");
    }

    /**
     * transfer
     *
     * @param volNameToVolInfoMap String
     * @return DSwareVolumeInfo>
     */
    public static Map<String, DSwareVolumeInfo> transfer(Map<String, Object> volNameToVolInfoMap) {
        if (volNameToVolInfoMap == null || volNameToVolInfoMap.isEmpty()) {
            return Collections.emptyMap();
        }
        return volNameToVolInfoMap.values().stream().map(t -> {
            DSwareVolumeInfo volumeInfo = new DSwareVolumeInfo(t);
            return new AbstractMap.SimpleEntry<>(volumeInfo.getVolName(), volumeInfo);
        }).collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
    }

    public String getVolName() {
        return this.volName;
    }

    public final void setVolName(final String volName) {
        this.volName = volName;
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

    public int getVolSize() {
        return this.volSize;
    }

    public void setVolSize(final int volSize) {
        this.volSize = volSize;
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

    public long getCreateTime() {
        return this.createTime;
    }

    public void setCreateTime(final long createTime) {
        this.createTime = createTime;
    }
}
