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
 * DSwareBitmapVolumeInfo
 *
 * @since 2024-08-27
 */
public class DSwareBitmapVolumeInfo {
    private static Method getVolNameMethod;

    private static Method getStatusMethod;

    private static Method getVolSizeMethod;

    private static Method getSnapSizeMethod;

    private static Method getSnapNameFromMethod;

    private static Method getSnapNameToMethod;

    private static Method getBlockSizeMethod;

    private static Method getPoolIdMethod;

    private static Method getCreateTimeMethod;

    private String volName;

    private int status;

    private int volSize;

    private int snapSize;

    private String snapNameFrom;

    private String snapNameTo;

    private int blockSize;

    private int poolId;

    private long createTime;

    /**
     * DSwareBitmapVolumeInfo
     *
     * @param object Object
     */
    public DSwareBitmapVolumeInfo(Object object) {
        if (RuntimeLoad.isNotInstance(this, object)) {
            return;
        }
        try {
            this.setVolName(String.valueOf(getVolNameMethod.invoke(object)));
            this.setStatus((int) getStatusMethod.invoke(object));
            this.setVolSize((int) getVolSizeMethod.invoke(object));
            this.setSnapSize((int) getSnapSizeMethod.invoke(object));
            this.setSnapNameFrom(String.valueOf(getSnapNameFromMethod.invoke(object)));
            this.setSnapNameTo(String.valueOf(getSnapNameToMethod.invoke(object)));
            this.setBlockSize((int) getBlockSizeMethod.invoke(object));
            this.setPoolId((int) getPoolIdMethod.invoke(object));
            this.setCreateTime((long) getCreateTimeMethod.invoke(object));
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new DSwareException(e);
        }
    }

    /**
     * loadBitmapVolumeInfoMethod
     *
     * @param dswareBitmapVolumeInfoClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadBitmapVolumeInfoMethod(Class<?> dswareBitmapVolumeInfoClass) throws NoSuchMethodException {
        getVolNameMethod = dswareBitmapVolumeInfoClass.getMethod("getVolName");
        getStatusMethod = dswareBitmapVolumeInfoClass.getMethod("getStatus");
        getVolSizeMethod = dswareBitmapVolumeInfoClass.getMethod("getVolSize");
        getSnapSizeMethod = dswareBitmapVolumeInfoClass.getMethod("getSnapSize");
        getSnapNameFromMethod = dswareBitmapVolumeInfoClass.getMethod("getSnapNameFrom");
        getSnapNameToMethod = dswareBitmapVolumeInfoClass.getMethod("getSnapNameTo");
        getBlockSizeMethod = dswareBitmapVolumeInfoClass.getMethod("getBlockSize");
        getPoolIdMethod = dswareBitmapVolumeInfoClass.getMethod("getPoolId");
        getCreateTimeMethod = dswareBitmapVolumeInfoClass.getMethod("getCreateTime");
    }

    public String getVolName() {
        return this.volName;
    }

    public final void setVolName(String volName) {
        this.volName = volName;
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

    public int getSnapSize() {
        return this.snapSize;
    }

    public void setSnapSize(final int snapSize) {
        this.snapSize = snapSize;
    }

    public String getSnapNameFrom() {
        return this.snapNameFrom;
    }

    public final void setSnapNameFrom(final String snapNameFrom) {
        this.snapNameFrom = snapNameFrom;
    }

    public String getSnapNameTo() {
        return this.snapNameTo;
    }

    public final void setSnapNameTo(final String snapNameTo) {
        this.snapNameTo = snapNameTo;
    }

    public int getBlockSize() {
        return this.blockSize;
    }

    public void setBlockSize(final int blockSize) {
        this.blockSize = blockSize;
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

    public void setCreateTime(long createTime) {
        this.createTime = createTime;
    }
}
