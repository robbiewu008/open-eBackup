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

import java.util.Map;

/**
 * DSwareApi
 *
 */
public interface DSwareApi {
    /**
     * createVolume
     *
     * @param dsaIps String[]
     * @param volName String
     * @param poolId int
     * @param volSize int
     * @param thinFlag int
     */
    void createVolume(String[] dsaIps, String volName, int poolId, int volSize, int thinFlag);

    /**
     * deleteVolume
     *
     * @param dsaIps String[]
     * @param volName String
     */
    void deleteVolume(String[] dsaIps, String volName);

    /**
     * deleteVolume
     *
     * @param dsaIps String[]
     * @param volName String
     * @param deleteType int
     */
    void deleteVolume(String[] dsaIps, String volName, int deleteType);

    /**
     * queryVolume
     *
     * @param dsaIps String[]
     * @param volName String
     * @return DSwareVolumeInfo
     */
    DSwareVolumeInfo queryVolume(String[] dsaIps, String volName);

    /**
     * queryAllVolume
     *
     * @param dsaIps String[]
     * @param poolId int
     * @return Map<String, DSwareVolumeInfo>
     */
    Map<String, DSwareVolumeInfo> queryAllVolume(String[] dsaIps, int poolId);

    /**
     * createSnapshot
     *
     * @param dsaIps String[]
     * @param snapName String
     * @param volName String
     * @param smartFlag int
     */
    void createSnapshot(String[] dsaIps, String snapName, String volName, int smartFlag);

    /**
     * deleteSnapshot
     *
     * @param dsaIps String[]
     * @param snapName String
     */
    void deleteSnapshot(String[] dsaIps, String snapName);

    /**
     * querySnapshot
     *
     * @param dsaIps String[]
     * @param snapName String
     * @return DSwareSnapInfo
     */
    DSwareSnapInfo querySnapshot(String[] dsaIps, String snapName);

    /**
     * queryAllSnapshot
     *
     * @param dsaIps String[]
     * @param poolId int
     * @return Map<String, DSwareSnapInfo>
     */
    Map<String, DSwareSnapInfo> queryAllSnapshot(String[] dsaIps, int poolId);

    /**
     * duplicateSnapshot
     *
     * @param dsaIps String[]
     * @param snapNameSrc String
     * @param snapNameDst String
     * @param args int...
     */
    void duplicateSnapshot(String[] dsaIps, String snapNameSrc, String snapNameDst, int... args);

    /**
     * createVolumeFromSnap
     *
     * @param dsaIps String[]
     * @param volName String
     * @param volSize int
     * @param snapNameSrc String
     */
    void createVolumeFromSnap(String[] dsaIps, String volName, int volSize, String snapNameSrc);

    /**
     * createVolumesFromSnap
     *
     * @param dsaIps String[]
     * @param volNames String[]
     * @param volSize int
     * @param snapNameSrc String
     */
    void createVolumesFromSnap(String[] dsaIps, String[] volNames, int volSize, String snapNameSrc);

    /**
     * querySnapOfVolume
     *
     * @param dsaIps String[]
     * @param volName String
     * @return Map<String, DSwareSnapInfo>
     */
    Map<String, DSwareSnapInfo> querySnapOfVolume(String[] dsaIps, String volName);

    /**
     * queryVolumeOfSnap
     *
     * @param dsaIps String[]
     * @param snapName String
     * @return Map<String, DSwareVolumeInfo>
     */
    Map<String, DSwareVolumeInfo> queryVolumeOfSnap(String[] dsaIps, String snapName);

    /**
     * queryPoolInfo
     *
     * @param dsaIps String[]
     * @param poolId int
     * @return DSwarePoolInfo
     */
    DSwarePoolInfo queryPoolInfo(String[] dsaIps, int poolId);

    /**
     * createFullVolumeFromSnap
     *
     * @param dsaIps String[]
     * @param snapNameSrc String
     * @param volName String
     */
    void createFullVolumeFromSnap(String[] dsaIps, String snapNameSrc, String volName);

    /**
     * queryAllPoolInfo
     *
     * @param dsaIps String[]
     * @return Map<Integer, DSwarePoolInfo>
     */
    Map<Integer, DSwarePoolInfo> queryAllPoolInfo(String[] dsaIps);

    /**
     * createBitmapVolume
     *
     * @param dsaIps String[]
     * @param snapNameFrom String
     * @param snapNameTo String
     * @param volNameForBitmap String
     * @return DSwareBackupInfo
     */
    DSwareBackupInfo createBitmapVolume(String[] dsaIps, String snapNameFrom, String snapNameTo,
        String volNameForBitmap);

    /**
     * queryBitmapVolume
     *
     * @param dsaIps String[]
     * @param volName String
     * @return DSwareBitmapVolumeInfo
     */
    DSwareBitmapVolumeInfo queryBitmapVolume(String[] dsaIps, String volName);

    /**
     * queryAllBitmapVolume
     *
     * @param dsaIps String[]
     * @param poolId int
     * @return Map<String, DSwareBitmapVolumeInfo>
     */
    Map<String, DSwareBitmapVolumeInfo> queryAllBitmapVolume(String[] dsaIps, int poolId);

    /**
     * expandVolume
     *
     * @param dsaIps String[]
     * @param volName String
     * @param newVolSize int
     */
    void expandVolume(String[] dsaIps, String volName, int newVolSize);
}
