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
package com.huawei.oceanprotect.k8s.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import com.huawei.oceanprotect.k8s.protection.access.constant.K8sConstant;
import com.huawei.oceanprotect.k8s.protection.access.service.K8sCommonService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.HashMap;
import java.util.Map;

/**
 * K8sRestoreProvider测试类
 *
 */
public class K8sRestoreProviderTest {
    private final CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
    private final K8sCommonService commonService = Mockito.mock(K8sCommonService.class);
    private final K8sRestoreProvider k8sRestoreProvider = new K8sRestoreProvider(copyRestApi, commonService);

    @Before
    public void init() {
        Copy copy = new Copy();
        copy.setOriginBackupId("9527");
        Mockito.when(copyRestApi.queryCopyByID(Mockito.any())).thenReturn(copy);
    }

    /**
     * 测试场景：恢复拦截成功
     * 前置条件：无
     * 检查点：拦截成功
     */
    @Test
    public void intercept_success() {
        RestoreTask task = new RestoreTask();
        TaskEnvironment targetEnv = new TaskEnvironment();
        targetEnv.setExtendInfo(new HashMap<>());
        task.setTargetEnv(targetEnv);
        TaskResource targetObj = new TaskResource();
        targetObj.setSubType(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType());
        task.setTargetObject(targetObj);

        RestoreTask res = k8sRestoreProvider.initialize(task);
        Assert.assertEquals(res.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS), SpeedStatisticsEnum.UBC.getType());
        Assert.assertEquals(res.getAdvanceParams().get(K8sConstant.ORIGIN_BACKUP_ID), "9527");
    }

    /**
     * 测试场景：指定了修改环境变量时，恢复拦截成功
     * 前置条件：无
     * 检查点：拦截成功
     */
    @Test
    public void intercept_with_change_env_success() {
        RestoreTask task = new RestoreTask();
        TaskEnvironment targetEnv = new TaskEnvironment();
        Map<String, String> extendInfo = new HashMap<>();
        String jsonStr = "[{\"containerName\":\"container\",\"envMap\":{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"},\"workLoadName\":\"Pod-0\",\"workLoadType\":\"Pod\"},{\"containerName\":\"container1\",\"envMap\":{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"},\"workLoadName\":\"Pod-01\",\"workLoadType\":\"Pod1\"}]";
        extendInfo.put("isEnableChangeEnv", "true");
        extendInfo.put("advancedConfigReqList", jsonStr);
        task.setAdvanceParams(extendInfo);
        targetEnv.setExtendInfo(new HashMap<>());
        task.setTargetEnv(targetEnv);
        TaskResource targetObj = new TaskResource();
        targetObj.setSubType(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType());
        task.setTargetObject(targetObj);

        RestoreTask res = k8sRestoreProvider.initialize(task);
        Assert.assertEquals(res.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS), SpeedStatisticsEnum.UBC.getType());
        Assert.assertEquals(res.getAdvanceParams().get(K8sConstant.ORIGIN_BACKUP_ID), "9527");
    }

    /**
     * 测试场景：applicable匹配成功
     * 前置条件：无
     * 检查点：返回True
     */
    @Test
    public void test_applicable() {
        Assert.assertTrue(k8sRestoreProvider.applicable(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType()));
        Assert.assertTrue(k8sRestoreProvider.applicable(ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.getType()));
    }

    /**
     * 测试场景：插件前置检查
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void test_restoreTaskCreationPreCheck() {
        RestoreTask task = new RestoreTask();
        Map<String, String> extendInfo = new HashMap<>();
        String jsonStr = "[{\"containerName\":\"container\",\"envMap\":{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"},\"workLoadName\":\"Pod-0\",\"workLoadType\":\"StatefulSet\"},{\"containerName\":\"container1\",\"envMap\":{\"key1\":\"value1\",\"key2\":\"value2\",\"key3\":\"value3\"},\"workLoadName\":\"Pod-01\",\"workLoadType\":\"StatefulSet\"}]";
        extendInfo.put("isEnableChangeEnv", "true");
        extendInfo.put("advancedConfigReqList", jsonStr);
        task.setAdvanceParams(extendInfo);
        TaskResource targetObj = new TaskResource();
        targetObj.setSubType(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType());
        task.setTargetObject(targetObj);
        k8sRestoreProvider.restoreTaskCreationPreCheck(task);
        Assert.assertTrue(true);
    }
}
