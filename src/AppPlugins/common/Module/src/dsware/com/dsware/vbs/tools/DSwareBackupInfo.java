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
 * DSwareBackupInfo
 *
 * @since 2024-08-27
 */
public class DSwareBackupInfo {
    private static Method getSnapSizeMethod;

    private static Method getBitmapSizeMethod;

    private static Method getBlockSizeMethod;

    private int snapSize;

    private int bitmapSize;

    private int blockSize;

    /**
     * DSwareBackupInfo
     *
     * @param object Object
     */
    public DSwareBackupInfo(Object object) {
        if (RuntimeLoad.isNotInstance(this, object)) {
            return;
        }
        try {
            this.setSnapSize((int) getSnapSizeMethod.invoke(object));
            this.setBitmapSize((int) getBitmapSizeMethod.invoke(object));
            this.setBlockSize((int) getBlockSizeMethod.invoke(object));
        } catch (IllegalAccessException | InvocationTargetException e) {
            throw new DSwareException(e);
        }
    }

    /**
     * loadBackupInfoMethod
     *
     * @param dswareBackupInfoClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadBackupInfoMethod(Class<?> dswareBackupInfoClass) throws NoSuchMethodException {
        getSnapSizeMethod = dswareBackupInfoClass.getMethod("getSnapSize");
        getBitmapSizeMethod = dswareBackupInfoClass.getMethod("getBitmapSize");
        getBlockSizeMethod = dswareBackupInfoClass.getMethod("getBlockSize");
    }

    public int getSnapSize() {
        return this.snapSize;
    }

    public void setSnapSize(final int snapSize) {
        this.snapSize = snapSize;
    }

    public int getBitmapSize() {
        return this.bitmapSize;
    }

    public void setBitmapSize(final int bitmapSize) {
        this.bitmapSize = bitmapSize;
    }

    public int getBlockSize() {
        return this.blockSize;
    }

    public void setBlockSize(final int blockSize) {
        this.blockSize = blockSize;
    }
}
