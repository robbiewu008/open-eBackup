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
package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.config.business.initialize.StorageVolumeConfig;
import openbackup.system.base.sdk.accesspoint.model.InitializeParam;
import openbackup.system.base.sdk.accesspoint.model.InitializeParamNodeInfo;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * InitializeParam test
 *
 * @author jwx701567
 * @since 2021-03-16
 */
public class InitializeParamTest {
    @Test
    public void initializeParam() {
        List<InitializeParamNodeInfo> theInitializeParamNodeInfos = new ArrayList<>();
        Map<String, String> abAddressToNasMap = new HashMap<>();
        abAddressToNasMap.put("a", "1");
        abAddressToNasMap.put("b", "1");
        abAddressToNasMap.put("c", "1");
        abAddressToNasMap.put("d", "1");
        theInitializeParamNodeInfos.add(new InitializeParamNodeInfo("Pod1", abAddressToNasMap));

        // 源路径
        Map<Integer, String> volumeNameMap = new HashMap<>();
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_FILE_SYSTEM, "dpa_fs_0_00_098561124753469323881537");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP, "dpa_fs_4_01_908281833138771886681651");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP, "dpa_fs_3_01_343849162486486369078204");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP, "dpa_fs_5_01_501380231541797817299300");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP, "dpa_fs_1_01_078561124753469323881537");
        InitializeParam initializeParam = new InitializeParam(theInitializeParamNodeInfos, volumeNameMap);
        Assert.assertEquals(1, initializeParam.getNodeNameToNodeParmInfos().size());
        System.out.println(initializeParam);
    }

    @Test
    public void initializeParamReset() {
        List<InitializeParamNodeInfo> theInitializeParamNodeInfos = new ArrayList<>();
        Map<String, String> abAddressToNasMap = new HashMap<>();
        abAddressToNasMap.put("a", "1");
        abAddressToNasMap.put("b", "1");
        abAddressToNasMap.put("c", "1");
        abAddressToNasMap.put("d", "1");
        theInitializeParamNodeInfos.add(new InitializeParamNodeInfo("Pod1", abAddressToNasMap));

        // 源路径
        Map<Integer, String> volumeNameMap = new HashMap<>();
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_FILE_SYSTEM, "dpa_fs_0_00_098561124753469323881537");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP, "dpa_fs_4_01_908281833138771886681651");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP, "dpa_fs_3_01_343849162486486369078204");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP, "dpa_fs_5_01_501380231541797817299300");
        volumeNameMap.put(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP, "dpa_fs_1_01_078561124753469323881537");
        List<String> sourcePathList = new LinkedList<>();
        sourcePathList.add("/dpa/dpa_fs_1_01_908281833138771886681651/");
        sourcePathList.add("/dpa/dpa_fs_3_01_343849162486486369078204/");
        sourcePathList.add("/dpa/dpa_fs_4_01_501380231541797817299300  ");
        sourcePathList.add("/dpa/dpa_fs_5_01_078561124753469323881537  ");
        InitializeParam initializeParam = new InitializeParam(theInitializeParamNodeInfos, volumeNameMap);

        Map<String, String> abAddressToNasMap2 = new HashMap<>();
        abAddressToNasMap2.put("e", "1");
        abAddressToNasMap2.put("f", "1");
        abAddressToNasMap2.put("g", "1");
        abAddressToNasMap2.put("h", "1");
        theInitializeParamNodeInfos.add(new InitializeParamNodeInfo("Pod2", abAddressToNasMap2));

        sourcePathList.add("/dpa/dpa_fs_1_01_908281833138771886681651/");
        sourcePathList.add("/dpa/dpa_fs_3_01_343849162486486369078204/");
        sourcePathList.add("/dpa/dpa_fs_4_01_501380231541797817299300  ");
        sourcePathList.add("/dpa/dpa_fs_5_01_078561124753469323881537  ");
        initializeParam.reset(theInitializeParamNodeInfos, sourcePathList);

        Assert.assertEquals(2, initializeParam.getNodeNameToNodeParmInfos().size());
        System.out.println(initializeParam);
    }
}
