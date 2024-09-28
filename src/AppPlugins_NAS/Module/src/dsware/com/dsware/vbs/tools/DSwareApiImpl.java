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

import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.AbstractMap;
import java.util.Collections;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * DSwareApiImpl
 *
 */
public class DSwareApiImpl implements DSwareApi {
    private static Method createVolumeMethod;

    private static Method deleteVolumeMethod;

    private static Method deleteVolumeWithTypeMethod;

    private static Method queryVolumeMethod;

    private static Method queryAllVolumeMethod;

    private static Method createSnapshotMethod;

    private static Method deleteSnapshotMethod;

    private static Method querySnapshotMethod;

    private static Method queryAllSnapshotMethod;

    private static Method duplicateSnapshotMethod;

    private static Method createVolumeFromSnapMethod;

    private static Method createVolumesFromSnapMethod;

    private static Method querySnapOfVolumeMethod;

    private static Method queryVolumeOfSnapMethod;

    private static Method queryPoolInfoMethod;

    private static Method createFullVolumeFromSnapMethod;

    private static Method queryAllPoolInfoMethod;

    private static Method createBitmapVolumeMethod;

    private static Method queryBitmapVolumeMethod;

    private static Method queryAllBitmapVolumeMethod;

    private static Method expandVolumeMethod;

    private static Constructor dswareApiImplConstructor;

    private final Object apiImplObj;

    /**
     * DSwareApiImpl
     *
     * @param dswareFloatIp String
     */
    public DSwareApiImpl(String dswareFloatIp) {
        try {
            if (dswareApiImplConstructor == null) {
                dswareApiImplConstructor = RuntimeLoad.getDswareClass(this.getClass().getSimpleName())
                    .getDeclaredConstructor(String.class);
            }
            apiImplObj = dswareApiImplConstructor.newInstance(dswareFloatIp);
        } catch (InstantiationException | IllegalAccessException | InvocationTargetException
                 | NoSuchMethodException e) {
            throw new DSwareException(e);
        }
    }

    /**
     * loadApiImplMethod
     *
     * @param dswareApiImplClass Class<?>
     * @throws NoSuchMethodException load dsware jar failed
     */
    public static void loadApiImplMethod(Class<?> dswareApiImplClass) throws NoSuchMethodException {
        createVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_CREATE_VOLUME, String[].class,
            String.class, int.class, int.class, int.class);
        deleteVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_DELETE_VOLUME, String[].class,
            String.class);
        deleteVolumeWithTypeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_DELETE_VOLUME, String[].class,
            String.class, int.class);
        queryVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_VOLUME, String[].class,
            String.class);
        queryAllVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_ALL_VOLUME, String[].class,
            int.class);
        createSnapshotMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_CREATE_SNAPSHOT, String[].class,
            String.class, String.class, int.class);
        deleteSnapshotMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_DELETE_SNAPSHOT, String[].class,
            String.class);
        querySnapshotMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_SNAPSHOT, String[].class,
            String.class);
        queryAllSnapshotMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_ALL_SNAPSHOT,
            String[].class, int.class);
        duplicateSnapshotMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_DUPLICATE_SNAPSHOT,
            String[].class, String.class, String.class, int.class, int.class, int.class);
        createVolumeFromSnapMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_CREATE_VOLUME_FROM_SNAP,
            String[].class, String.class, int.class, String.class);
        createVolumesFromSnapMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_CREATE_VOLUMES_FROM_SNAP,
            String[].class, String[].class, int.class, String.class);
        querySnapOfVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_SNAP_VOLUME,
            String[].class, String.class);
        queryVolumeOfSnapMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_VOLUME_SNAP,
            String[].class, String.class);
        queryPoolInfoMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_POOL_INFO, String[].class,
            int.class);
        createFullVolumeFromSnapMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_CREATE_FULL_VOLUME_INFO,
            String[].class, String.class, String.class);
        queryAllPoolInfoMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_ALL_POOL_INFO,
            String[].class);
        createBitmapVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_CREATE_BITMAP_VOLUME,
            String[].class, String.class, String.class, String.class);
        queryBitmapVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_BITMAP_VOLUME,
            String[].class, String.class);
        queryAllBitmapVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_NAME_QUERY_ALL_BITMAP_VOLUME,
            String[].class, int.class);
        expandVolumeMethod = dswareApiImplClass.getMethod(ToolsContances.CMD_EXPAND_VOLUME, String[].class,
            String.class, int.class);
        dswareApiImplConstructor = dswareApiImplClass.getDeclaredConstructor(String.class);
    }

    private Object invoke(Method method, Object instance, Object... args) {
        try {
            return method.invoke(instance, args);
        } catch (InvocationTargetException | IllegalAccessException e) {
            throw new DSwareException(e);
        }
    }

    @Override
    public void createVolume(String[] dsaIps, String volName, int poolId, int volSize, int thinFlag) {
        invoke(createVolumeMethod, apiImplObj, dsaIps, volName, poolId, volSize, thinFlag);
    }

    @Override
    public void deleteVolume(String[] dsaIps, String volName) {
        invoke(deleteVolumeMethod, apiImplObj, dsaIps, volName);
    }

    @Override
    public void deleteVolume(String[] dsaIps, String volName, int deleteType) {
        invoke(deleteVolumeWithTypeMethod, apiImplObj, dsaIps, volName, deleteType);
    }

    @Override
    public DSwareVolumeInfo queryVolume(String[] dsaIps, String volName) {
        return new DSwareVolumeInfo(invoke(queryVolumeMethod, apiImplObj, dsaIps, volName));
    }

    @Override
    public Map<String, DSwareVolumeInfo> queryAllVolume(String[] dsaIps, int poolId) {
        return DSwareVolumeInfo.transfer(
            (Map<String, Object>) invoke(queryAllVolumeMethod, apiImplObj, dsaIps, poolId));
    }

    @Override
    public void createSnapshot(String[] dsaIps, String snapName, String volName, int smartFlag) {
        invoke(createSnapshotMethod, apiImplObj, dsaIps, snapName, volName, smartFlag);
    }

    @Override
    public void deleteSnapshot(String[] dsaIps, String snapName) {
        invoke(deleteSnapshotMethod, apiImplObj, dsaIps, snapName);
    }

    @Override
    public DSwareSnapInfo querySnapshot(String[] dsaIps, String snapName) {
        return new DSwareSnapInfo(invoke(querySnapshotMethod, apiImplObj, dsaIps, snapName));
    }

    @Override
    public Map<String, DSwareSnapInfo> queryAllSnapshot(String[] dsaIps, int poolId) {
        return DSwareSnapInfo.transfer(
            (Map<String, Object>) invoke(queryAllSnapshotMethod, apiImplObj, dsaIps, poolId));
    }

    @Override
    public void duplicateSnapshot(String[] dsaIps, String snapNameSrc, String snapNameDst, int... args) {
        if (args == null || args.length != 3) {
            throw new DSwareException("duplicateSnapshot args is missing");
        }
        invoke(duplicateSnapshotMethod, apiImplObj, dsaIps, snapNameSrc, snapNameDst, args[0], args[1], args[2]);
    }

    @Override
    public void createVolumeFromSnap(String[] dsaIps, String volName, int volSize, String snapNameSrc) {
        invoke(createVolumeFromSnapMethod, apiImplObj, dsaIps, volName, volSize, snapNameSrc);
    }

    @Override
    public void createVolumesFromSnap(String[] dsaIps, String[] volNames, int volSize, String snapNameSrc) {
        invoke(createVolumesFromSnapMethod, apiImplObj, dsaIps, volNames, volSize, snapNameSrc);
    }

    @Override
    public Map<String, DSwareSnapInfo> querySnapOfVolume(String[] dsaIps, String volName) {
        return DSwareSnapInfo.transfer(
            (Map<String, Object>) invoke(querySnapOfVolumeMethod, apiImplObj, dsaIps, volName));
    }

    @Override
    public Map<String, DSwareVolumeInfo> queryVolumeOfSnap(String[] dsaIps, String snapName) {
        return DSwareVolumeInfo.transfer(
            (Map<String, Object>) invoke(queryVolumeOfSnapMethod, apiImplObj, dsaIps, snapName));
    }

    @Override
    public DSwarePoolInfo queryPoolInfo(String[] dsaIps, int poolId) {
        return new DSwarePoolInfo(invoke(queryPoolInfoMethod, apiImplObj, dsaIps, poolId));
    }

    @Override
    public void createFullVolumeFromSnap(String[] dsaIps, String snapNameSrc, String volName) {
        invoke(createFullVolumeFromSnapMethod, apiImplObj, dsaIps, snapNameSrc, volName);
    }

    @Override
    public Map<Integer, DSwarePoolInfo> queryAllPoolInfo(String[] dsaIps) {
        Map<Integer, Object> poolIdToPoolInfoMap = (Map<Integer, Object>) invoke(queryAllPoolInfoMethod, apiImplObj,
            (Object) dsaIps);
        if (poolIdToPoolInfoMap == null || poolIdToPoolInfoMap.isEmpty()) {
            return Collections.emptyMap();
        }
        return poolIdToPoolInfoMap.values().stream().map(t -> {
            DSwarePoolInfo poolInfo = new DSwarePoolInfo(t);
            return new AbstractMap.SimpleEntry<>(poolInfo.getPoolId(), poolInfo);
        }).collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
    }

    @Override
    public DSwareBackupInfo createBitmapVolume(String[] dsaIps, String snapNameFrom, String snapNameTo,
        String volNameForBitmap) {
        return new DSwareBackupInfo(
            invoke(createBitmapVolumeMethod, apiImplObj, dsaIps, snapNameFrom, snapNameTo, volNameForBitmap));
    }

    @Override
    public DSwareBitmapVolumeInfo queryBitmapVolume(String[] dsaIps, String volName) {
        return new DSwareBitmapVolumeInfo(invoke(queryBitmapVolumeMethod, apiImplObj, dsaIps, volName));
    }

    @Override
    public Map<String, DSwareBitmapVolumeInfo> queryAllBitmapVolume(String[] dsaIps, int poolId) {
        Map<String, Object> volNameToBitmapVolumeInfoMap = (Map<String, Object>) invoke(queryAllBitmapVolumeMethod,
            apiImplObj, dsaIps, poolId);
        if (volNameToBitmapVolumeInfoMap == null || volNameToBitmapVolumeInfoMap.isEmpty()) {
            return Collections.emptyMap();
        }
        return volNameToBitmapVolumeInfoMap.values().stream().map(t -> {
            DSwareBitmapVolumeInfo bitmapVolumeInfo = new DSwareBitmapVolumeInfo(t);
            return new AbstractMap.SimpleEntry<>(bitmapVolumeInfo.getVolName(), bitmapVolumeInfo);
        }).collect(Collectors.toMap(Map.Entry::getKey, Map.Entry::getValue));
    }

    @Override
    public void expandVolume(String[] dsaIps, String volName, int newVolSize) {
        invoke(expandVolumeMethod, apiImplObj, dsaIps, volName, newVolSize);
    }
}
