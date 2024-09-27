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

import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.config.business.initialize.StorageVolumeConfig;
import openbackup.system.base.sdk.accesspoint.model.InitializeParam;
import openbackup.system.base.sdk.accesspoint.model.InitializeParamNodeInfo;

import org.junit.Assert;
import org.junit.Test;

/**
 * 测试初始化参数（路径）
 *
 * @author w00493811
 * @since 2021-02-03
 */
public class InitializeParamNodeInfoTest {
    /**
     * 测试是否全被设置
     */
    @Test
    public void isAllSetReturnTrue() {
        InitializeParamNodeInfo path = new InitializeParamNodeInfo("", null);
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_FILE_SYSTEM, "1");
        Assert.assertTrue(path.isAllSet());
    }

    /**
     * 测试是否全被设置
     */
    @Test
    public void isAllSetReturnFalse() {
        InitializeParamNodeInfo path = new InitializeParamNodeInfo("", null);
        Assert.assertFalse(path.isAllSet());
    }

    /**
     * 测试是否全被设置
     */
    @Test
    public void putValueByTypeReturnTrue() {
        InitializeParamNodeInfo path = new InitializeParamNodeInfo("", null);
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP, "1");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_FILE_SYSTEM, "1");
        Assert.assertTrue(path.isAllSet());
    }

    /**
     * 测试是否全被设置
     */
    @Test
    public void putValueByTypeReturnFalse() {
        InitializeParamNodeInfo path = new InitializeParamNodeInfo("", null);
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP, "");
        Assert.assertFalse(path.isAllSet());
    }

    /**
     * 测试是否全被设置
     */
    @Test
    public void isContainsPathReturnTrue() {
        InitializeParamNodeInfo path = new InitializeParamNodeInfo("", null);
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP,
            "/dpa/dpa_fs_0A_1_01_908281833138771886681651");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_METADATA_BACKUP,
            "/dpa/dpa_fs_0A_3_01_343849162486486369078204");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_COULD_BACKUP,
            "/dpa/dpa_fs_0A_4_01_501380231541797817299300");
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_SELF_BACKUP,
            "/dpa/dpa_fs_0A_5_01_078561124753469323881537");
        Assert.assertTrue(path.isContainsPath("/dpa_fs_0A_3_01_343849162486486369078204/"));
    }

    /**
     * 测试是否全被设置
     */
    @Test
    public void isContainsPathReturnFalse() {
        InitializeParamNodeInfo path = new InitializeParamNodeInfo("", null);
        path.putValueByType(StorageVolumeConfig.VOLUME_TYPE_STANDARD_BACKUP,
            "/dpa/dpa_fs_0A_1_01_908281833138771886681651");
        Assert.assertFalse(path.isContainsPath("/dpa/dpa_fs_0A_3_01_343849162486486368078204"));
    }

    @Test
    public void testJsonObjectToBean() {
        String initValue
            = "{\"nodeNameToNodeParmInfos\":{\"protectengine-a-business-data-0\":{\"podName\":\"protectengine-a-business-data-0\",\"abAddressToNasMap\":{\"172.16.0.133\":\"172.17.128.129\",\"172.17.64.131\":\"172.17.128.129\",\"172.17.128.131\":\"172.17.128.129\",\"192.168.231.154\":\"172.17.128.129\"},\"nasPathForStandardVolume\":\"/dpa_fs_0B_1_01_081104053209075484620516\",\"nasPathForMetaDataVolume\":\"/dpa_fs_0B_3_01_177616148360627050135606\",\"nasPathForSelfBackVolume\":\"/dpa_fs_0B_5_01_076484052375119634718631\",\"nasPathForCloudIdxVolume\":\"/dpa_fs_0B_4_01_699955668992499079028714\",\"allSet\":true},\"protectengine-a-business-control-0\":{\"podName\":\"protectengine-a-business-control-0\",\"abAddressToNasMap\":{\"172.17.64.6\":\"172.17.128.1\",\"172.16.0.8\":\"172.17.128.1\",\"172.17.128.6\":\"172.17.128.1\",\"192.168.231.153\":\"172.17.128.1\"},\"nasPathForStandardVolume\":\"/dpa_fs_0A_1_01_111662463453256876418396\",\"nasPathForMetaDataVolume\":\"/dpa_fs_0A_3_01_031510343598060387096558\",\"nasPathForSelfBackVolume\":\"/dpa_fs_0A_5_01_916303518467352951670228\",\"nasPathForCloudIdxVolume\":\"/dpa_fs_0A_4_01_841534113339098779383408\",\"allSet\":true}},\"mountPathOneNode\":false}";
        InitializeParam initializeParam = JSONObject.toBean(initValue, InitializeParam.class);
    }
}
