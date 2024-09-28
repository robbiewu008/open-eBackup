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
package openbackup.database.base.plugin.util;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.GeneralDbConstant;

import org.junit.Assert;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

/**
 * GeneralDbUtil测试类
 *
 */
public class GeneralDbUtilTest {
    /**
     * 用例场景：将配置文件的字符串转化为对象
     * 前置条件：无
     * 检查点：能将appconf字符串转化为对象
     */
    @Test
    public void app_conf_transform_success() {
        String appConfStr = TestConfHelper.getHanaConf();
        AppConf appConf = GeneralDbUtil.getAppConf(appConfStr).orElse(null);
        Assert.assertNotNull(appConf.getResource().getClusterCheckResultThreshold());
        Assert.assertNotNull(appConf.getBackup().getSupports().get(0).getBackupType());

        Assert.assertNull(GeneralDbUtil.getAppConf("").orElse(null));
    }

    /**
     * 用例场景：检查列表中是否包含字符串
     * 前置条件：无
     * 检查点：包含: true; 不包含: false
     */
    @Test
    public void check_list_contain_elem_success() {
        List<String> weeks = Arrays.asList("Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
            "Sunday");
        Assert.assertTrue(GeneralDbUtil.isListContainsElemWithoutCase(weeks, "tuesday"));
        Assert.assertTrue(GeneralDbUtil.isListContainsElemWithoutCase(weeks, "Thursday"));
        Assert.assertFalse(GeneralDbUtil.isListContainsElemWithoutCase(weeks, "Frid"));

        Assert.assertFalse(GeneralDbUtil.isListContainsElemWithoutCase(new ArrayList<>(),"a"));
        List<String> months = new ArrayList<>();
        months.add(null);
        Assert.assertTrue(GeneralDbUtil.isListContainsElemWithoutCase(months,null));
    }

    /**
     * 用例场景：检查从资源中获取host
     * 前置条件：无
     * 检查点：集群，获取数量大于1；单机，获取数量等于1
     */
    @Test
    public void check_get_host_success() {
        List<ProtectedEnvironment> hosts1 = GeneralDbUtil.getHosts(TestConfHelper.mockInstance(false));
        Assert.assertEquals(hosts1.size(), 2);
        List<ProtectedEnvironment> hosts2 = GeneralDbUtil.getHosts(TestConfHelper.mockInstance(true));
        Assert.assertEquals(hosts2.size(), 1);
    }

    /**
     * 用例场景：将ProtectedResource转化为AppEnv
     * 前置条件：无
     * 检查点：将ProtectedResource转化为AppEnv, 属性正确. 补充host的信息
     */
    @Test
    public void resource_transfer_to_app_env_success() {
        List<ProtectedEnvironment> hosts = TestConfHelper.mockHost();
        // 数据库
        ProtectedEnvironment singleDb = TestConfHelper.mockDatabase(true);
        AppEnv appEnvDb = GeneralDbUtil.resourceToAppEnv(singleDb, hosts, null);
        Assert.assertEquals(appEnvDb.getExtendInfo().get(GeneralDbConstant.EXTEND_SCRIPT_KEY), TestConfHelper.SAP_HANA);
        Assert.assertEquals(1, appEnvDb.getNodes().size());
        Assert.assertNotNull(appEnvDb.getNodes().get(0).getEndpoint());
        // 单机
        ProtectedEnvironment singleInstance = TestConfHelper.mockInstance(true);
        AppEnv appEnvSingleInstance = GeneralDbUtil.resourceToAppEnv(singleInstance, hosts, null);
        Assert.assertEquals(appEnvSingleInstance.getExtendInfo().get(GeneralDbConstant.EXTEND_SCRIPT_KEY),
            TestConfHelper.SAP_HANA);
        Assert.assertEquals(1, appEnvSingleInstance.getNodes().size());
        Assert.assertEquals(0, appEnvSingleInstance.getNodes().get(0).getNodes().size());
        Assert.assertNotNull(appEnvDb.getNodes().get(0).getEndpoint());
        // 集群
        ProtectedEnvironment clusterInstance = TestConfHelper.mockInstance(false);
        AppEnv appEnvClusterInstance = GeneralDbUtil.resourceToAppEnv(clusterInstance, hosts, null);
        Assert.assertEquals(appEnvClusterInstance.getExtendInfo().get(GeneralDbConstant.EXTEND_SCRIPT_KEY),
            TestConfHelper.SAP_HANA);
        Assert.assertTrue(appEnvClusterInstance.getNodes().size() > 1);
        Assert.assertEquals(1, appEnvClusterInstance.getNodes().get(0).getNodes().size());
        Assert.assertNotNull(appEnvClusterInstance.getNodes().get(0).getNodes().get(0).getEndpoint());
    }

    /**
     * 用例场景：将ProtectedResource转化为AppEnv
     * 前置条件：无
     * 检查点：将ProtectedResource转化为AppEnv, 属性正确: version, extendinfo.
     *        host值能转化回只留uuid
     */
    @Test
    public void app_resource_to_protected_resource_success() {
        ProtectedResource protectedResource = TestConfHelper.mockDatabase(false);
        List<ProtectedEnvironment> hostList = TestConfHelper.mockHost();
        GeneralDbUtil.setProtectResourceFullHostInfo(protectedResource, hostList);
        Assert.assertNotNull(
            protectedResource.getDependencies().get(GeneralDbConstant.DEPENDENCY_HOST_KEY).get(0).getSubType());
        AppResource appResource = new AppResource();
        appResource.setUuid("aaa");
        appResource.setExtendInfo(new HashMap<>());
        appResource.getExtendInfo().put("a", "1");
        appResource.getExtendInfo().put(GeneralDbConstant.EXTEND_VERSION_KEY, "1.2");
        GeneralDbUtil.appResourceToProtectedResource(protectedResource, hostList, appResource);
        Assert.assertNull(
            protectedResource.getDependencies().get(GeneralDbConstant.DEPENDENCY_HOST_KEY).get(0).getSubType());
        Assert.assertEquals(protectedResource.getExtendInfoByKey("a"), "1");
        Assert.assertEquals(protectedResource.getExtendInfoByKey(GeneralDbConstant.EXTEND_VERSION_KEY), "1.2");
    }

    /**
     * 用例场景：判断version是否在区间内
     * 前置条件：无
     * 检查点：version应该满足：minVersion <= version <= maxVersion
     */
    @Test
    public void check_version_match() {
        Assert.assertTrue(GeneralDbUtil.checkVersion(null, null, null));
        Assert.assertFalse(GeneralDbUtil.checkVersion(null, "1", null));
        Assert.assertFalse(GeneralDbUtil.checkVersion(null, null, "2"));
        Assert.assertTrue(GeneralDbUtil.checkVersion("1.0", "1.0", null));
        Assert.assertTrue(GeneralDbUtil.checkVersion("2.0", null, "2.0"));
        Assert.assertTrue(GeneralDbUtil.checkVersion("1.1", "1.0", "2.0"));
        Assert.assertFalse(GeneralDbUtil.checkVersion("1.1", "2.0", "2"));
        Assert.assertFalse(GeneralDbUtil.checkVersion("2.1", null, "2"));
    }
}
