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

import java.io.IOException;
import java.net.URL;
import java.net.URLClassLoader;
import java.util.HashMap;
import java.util.Map;

/**
 * RuntimeLoad
 *
 */
public class RuntimeLoad {
    private static final String DSWARE_JSR_PATH = "DSWARE_JSR_PATH";

    private static final Map<String, Class<?>> classNameToClassObjMap = new HashMap<>();

    private static Class<?> dswareApiImplClass;

    private static Class<?> dswarePoolInfoClass;

    private static Class<?> dswareSnapInfoClass;

    private static Class<?> dswareVolumeInfoClass;

    private static Class<?> dswareBackupInfoClass;

    private static Class<?> dswareBitmapVolumeInfoClass;

    private static Class<?> dswareExceptionClass;

    private static Class<?> dswareErrorCodeClass;

    static {
        String jarPath = System.getenv(DSWARE_JSR_PATH);
        if (jarPath == null || jarPath.isEmpty()) {
            jarPath = "file:dsware-api.jar";
        } else {
            jarPath = "file:" + jarPath;
        }
        try (URLClassLoader classLoader = new URLClassLoader(new URL[] {new URL(jarPath)})) {
            loadClass(classLoader);
            DSwareApiImpl.loadApiImplMethod(dswareApiImplClass);
            DSwareBackupInfo.loadBackupInfoMethod(dswareBackupInfoClass);
            DSwareBitmapVolumeInfo.loadBitmapVolumeInfoMethod(dswareBitmapVolumeInfoClass);
            DSwareException.loadExceptionMethod(dswareExceptionClass, dswareErrorCodeClass);
            DSwarePoolInfo.loadPoolInfoMethod(dswarePoolInfoClass);
            DSwareSnapInfo.loadSnapInfoMethod(dswareSnapInfoClass);
            DSwareVolumeInfo.loadVolumeInfoMethod(dswareVolumeInfoClass);
        } catch (ClassNotFoundException | IOException | NoSuchMethodException e) {
            e.printStackTrace();
        }
    }

    /**
     * getDswareClass
     *
     * @param classSimpleName String
     * @return Class<?>
     */
    public static Class<?> getDswareClass(String classSimpleName) {
        return classNameToClassObjMap.get(classSimpleName);
    }

    /**
     * isNotInstance
     *
     * @param wantObject Object
     * @param targetObject Object
     * @return boolean
     */
    public static boolean isNotInstance(Object wantObject, Object targetObject) {
        return !getDswareClass(wantObject.getClass().getSimpleName()).isInstance(targetObject);
    }

    private static void loadClass(URLClassLoader classLoader) throws ClassNotFoundException {
        dswareApiImplClass = classLoader.loadClass("com.dsware.om.client.service.impl.DSwareApiImpl");
        dswarePoolInfoClass = classLoader.loadClass("com.dsware.om.client.bean.DSwarePoolInfo");
        dswareSnapInfoClass = classLoader.loadClass("com.dsware.om.client.bean.DSwareSnapInfo");
        dswareVolumeInfoClass = classLoader.loadClass("com.dsware.om.client.bean.DSwareVolumeInfo");
        dswareBackupInfoClass = classLoader.loadClass("com.dsware.om.client.bean.DSwareBackupInfo");
        dswareBitmapVolumeInfoClass = classLoader.loadClass("com.dsware.om.client.bean.DSwareBitmapVolumeInfo");
        dswareExceptionClass = classLoader.loadClass("com.dsware.om.client.exception.DSwareException");
        dswareErrorCodeClass = classLoader.loadClass("com.dsware.om.client.exception.DSwareErrorCode");
        classNameToClassObjMap.put(dswareApiImplClass.getSimpleName(), dswareApiImplClass);
        classNameToClassObjMap.put(dswarePoolInfoClass.getSimpleName(), dswarePoolInfoClass);
        classNameToClassObjMap.put(dswareSnapInfoClass.getSimpleName(), dswareSnapInfoClass);
        classNameToClassObjMap.put(dswareVolumeInfoClass.getSimpleName(), dswareVolumeInfoClass);
        classNameToClassObjMap.put(dswareBackupInfoClass.getSimpleName(), dswareBackupInfoClass);
        classNameToClassObjMap.put(dswareBitmapVolumeInfoClass.getSimpleName(), dswareBitmapVolumeInfoClass);
        classNameToClassObjMap.put(dswareExceptionClass.getSimpleName(), dswareExceptionClass);
    }
}
