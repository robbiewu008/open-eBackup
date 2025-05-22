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
package openbackup.oceanprotect.k8s.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.oceanprotect.k8s.protection.access.provider.K8sBackupProvider;
import openbackup.oceanprotect.k8s.protection.access.service.K8sCommonService;
import openbackup.oceanprotect.k8s.protection.access.util.K8sUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;

/**
 * K8sBackupProvider Test
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/29
 */
public class K8sBackupProviderTest {
    private final K8sCommonService commonService = Mockito.mock(K8sCommonService.class);
    private final ProtectedEnvironmentService environmentService = Mockito.mock(ProtectedEnvironmentService.class);
    private final K8sBackupProvider k8sBackupProvider = new K8sBackupProvider(commonService, environmentService);

    @Before
    public void init() {
        ProtectedEnvironment agent1 = new ProtectedEnvironment();
        agent1.setUuid("1");
        agent1.setPort(222);
    }

    /**
     * 测试场景：applicable匹配成功
     * 前置条件：无
     * 检查点：返回True
     */
    @Test
    public void test_applicable() {
        Assert.assertTrue(k8sBackupProvider.applicable(ResourceSubTypeEnum.KUBERNETES_NAMESPACE_COMMON.getType()));
        Assert.assertTrue(k8sBackupProvider.applicable(ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.getType()));
    }

    /**
     * 测试场景：k8s备份插件拦截成功
     * 前置条件：备份流程正常
     * 检查点：拦截成功，设置值成功
     */
    @Test
    public void test_intercept_success() {
        BackupTask backupTask = new BackupTask();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setExtendInfo(new HashMap<>());
        taskEnvironment.getExtendInfo().put(K8sUtil.getInternalAgentConnectionKey("1"), "true");
        taskEnvironment.getExtendInfo().put(K8sUtil.getInternalAgentConnectionKey("2"), "false");
        backupTask.setProtectEnv(taskEnvironment);

        StorageRepository storageRepository = new StorageRepository();
        backupTask.setRepositories(new ArrayList<>(Collections.singletonList(storageRepository)));

        backupTask.setProtectObject(mockProtectObject());

        k8sBackupProvider.initialize(backupTask);

        Assert.assertEquals(backupTask.getRepositories().size(), 2);
        Assert.assertEquals(backupTask.getAdvanceParams().get(TaskUtil.SPEED_STATISTICS),
                SpeedStatisticsEnum.UBC.getType());
        Assert.assertEquals(backupTask.getProtectObject().getExtendInfo().get("labels"),
                "open=open,close!=close");
    }

    private TaskResource mockProtectObject() {
        HashMap<String, String> map = new HashMap<>();
        TaskResource taskResource = new TaskResource();
        map.put("excludeLabels", "close!=close");
        map.put("labels", "open=open");
        taskResource.setExtendInfo(map);
        taskResource.setSubType(ResourceSubTypeEnum.KUBERNETES_DATASET_COMMON.getType());
        return taskResource;
    }
}
