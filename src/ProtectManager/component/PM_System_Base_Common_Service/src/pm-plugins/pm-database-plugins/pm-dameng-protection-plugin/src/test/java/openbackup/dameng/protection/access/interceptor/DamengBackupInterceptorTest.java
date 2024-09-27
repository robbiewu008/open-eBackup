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
package openbackup.dameng.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.any;

import openbackup.dameng.protection.access.DamengTestDataUtil;
import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.provider.DamengAgentProvider;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * dameng备份任务拦截器测试类
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-18
 */
public class DamengBackupInterceptorTest {
    private final DamengService damengService = PowerMockito.mock(DamengService.class);
    private final DamengAgentProvider agentProvider = PowerMockito.mock(DamengAgentProvider.class);

    private final DamengBackupInterceptor damengBackupInterceptor = new DamengBackupInterceptor(damengService,
        agentProvider);

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(damengBackupInterceptor.applicable(ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType()));
        Assert.assertTrue(damengBackupInterceptor.applicable(ResourceSubTypeEnum.DAMENG_CLUSTER.getType()));
        Assert.assertFalse(damengBackupInterceptor.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
    }

    /**
     * 用例场景：dameng填充集群agent信息
     * 前置条件：无
     * 检查点：填充agent信息成功
     */
    @Test
    public void supply_agent_success_when_subtype_is_cluster() {
        BackupTask backupTask = DamengTestDataUtil.buildBackupTask("fullBackup",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("127.0.0.1");
        endpoint.setPort(8081);
        PowerMockito.when(agentProvider.getSelectedAgents(any())).thenReturn(Collections.singletonList(endpoint));
        damengBackupInterceptor.supplyAgent(backupTask);
        Endpoint resEndpoint = backupTask.getAgents().get(0);
        Assert.assertEquals(resEndpoint.getIp(), "127.0.0.1");
    }

    /**
     * 用例场景：dameng填充单机node信息
     * 前置条件：无
     * 检查点：填充node信息成功
     */
    @Test
    public void supply_nodes_success_when_subtype_is_single_node() {
        BackupTask backupTask = DamengTestDataUtil.buildBackupTask("fullBackup",
            ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType());
        damengBackupInterceptor.supplyNodes(backupTask);
        Assert.assertEquals(backupTask.getProtectEnv().getNodes().get(0).getEndpoint(), "127.0.0.1");
    }

    /**
     * 用例场景：dameng填充集群node信息
     * 前置条件：无
     * 检查点：填充node信息成功
     */
    @Test
    public void supply_nodes_success_when_subtype_is_cluster() {
        BackupTask backupTask = DamengTestDataUtil.buildBackupTask("fullBackup",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        TaskEnvironment nodes = new TaskEnvironment();
        nodes.setEndpoint("127.0.0.1");
        nodes.setPort(8080);
        PowerMockito.when(damengService.buildTaskNodes(any())).thenReturn(Collections.singletonList(nodes));
        damengBackupInterceptor.supplyNodes(backupTask);
        Assert.assertEquals(backupTask.getProtectEnv().getNodes().get(0).getEndpoint(), "127.0.0.1");
    }

    /**
     * 用例场景：dameng检查连通性
     * 前置条件：无
     * 检查点：检查成功
     */
    @Test
    public void check_connention_success() {
        BackupTask backupTask = DamengTestDataUtil.buildBackupTask("fullBackup",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        damengBackupInterceptor.checkConnention(backupTask);
        Assert.assertEquals(backupTask.getProtectEnv().getNodes().get(0).getEndpoint(), "127.0.0.1");
    }

    /**
     * 用例场景：dameng更新全量备份task信息
     * 前置条件：无
     * 检查点：更新成功
     */
    @Test
    public void supply_backup_task_success_if_job_is_full_backup() {
        BackupTask backupTask = DamengTestDataUtil.buildBackupTask("fullBackup",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        damengBackupInterceptor.supplyBackupTask(backupTask);
        Assert.assertEquals(backupTask.getCopyFormat(), CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
    }

    /**
     * 用例场景：dameng更新日志备份task信息
     * 前置条件：无
     * 检查点：更新成功
     */
    @Test
    public void supply_backup_task_success_if_job_is_log_backup() {
        BackupTask backupTask = DamengTestDataUtil.buildBackupTask("logBackup",
            ResourceSubTypeEnum.DAMENG_CLUSTER.getType());
        damengBackupInterceptor.supplyBackupTask(backupTask);
        Assert.assertEquals(backupTask.getCopyFormat(), CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
    }

    private Map<ProtectedResource, List<ProtectedEnvironment>> buildProtectedResourceMap(
        ProtectedResource protectedResource) {
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap = new HashMap<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setUuid("aaa");
        protectedEnvironment.setPort(8081);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DamengConstant.ROLE, "1");
        protectedEnvironment.setExtendInfo(extendInfo);
        List<ProtectedEnvironment> protectedEnvironmentList = new ArrayList<>();
        protectedEnvironmentList.add(protectedEnvironment);
        protectedResourceMap.put(protectedResource, protectedEnvironmentList);
        return protectedResourceMap;
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(damengBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
