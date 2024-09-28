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
package openbackup.mongodb.protection.access.provider.backup;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.mock.MongoDBMockBean;

import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * MongoDB备份拦截器 测试类
 *
 */
public class MongoDBBackupInterceptorTest {
    private final MongoDBBaseService mongoDBBaseService = Mockito.mock(MongoDBBaseService.class);
    private final ResourceService resourceService = Mockito.mock(ResourceService.class);

    private final MongoDBMockBean mongoDBMockBean = new MongoDBMockBean();

    private final MongoDBBackupInterceptor mongoDBBackupInterceptor = new MongoDBBackupInterceptor(mongoDBBaseService);

    /**
     * 用例场景：MongoDB备份下发provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(mongoDBBackupInterceptor.applicable(ResourceSubTypeEnum.MONGODB_CLUSTER.getType()));
        Assert.assertTrue(mongoDBBackupInterceptor.applicable(ResourceSubTypeEnum.MONGODB_SINGLE.getType()));
    }

    /**
     * 用例场景：MongoDB备份下发provider过滤
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void supply_backup_task_success() {
        BackupTask backupTask = new BackupTask();
        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(new StorageRepository());
        backupTask.setRepositories(repositories);
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setProtectObject(new TaskResource());
        mongoDBBackupInterceptor.supplyBackupTask(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：MongoDB备份下发健康检查成功
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void check_connention_success() {
        BackupTask backupTask = new BackupTask();
        List<StorageRepository> repositories = new ArrayList<>();
        repositories.add(new StorageRepository());
        backupTask.setRepositories(repositories);
        backupTask.setProtectEnv(new TaskEnvironment());
        backupTask.setProtectObject(new TaskResource());
        AppEnvResponse mongoDBAppEnvResponse = mongoDBMockBean.getMongoDBAppEnvResponse();
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.EXIST_NODES,"1");
        mongoDBAppEnvResponse.getExtendInfo().put(MongoDBConstants.AGENT_HOST,"8.40.96.214:28017");
        List<NodeInfo> nodeInfos = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setName("8.40.96.214:28017");
        nodeInfos.add(nodeInfo);
        nodeInfos.add(nodeInfo);
        mongoDBAppEnvResponse.setNodes(nodeInfos);
        given(mongoDBBaseService.getEnvironmentById(any())).willReturn(mongoDBMockBean.getMongoDBProtectedEnvironment());
        List<String> list = new ArrayList<>();
        List<AppEnvResponse> list1 = new ArrayList<>();
        list1.add(mongoDBAppEnvResponse);
        list1.add(mongoDBAppEnvResponse);
        list.add("8.40.96.214:28017");
        given(mongoDBBaseService.getAllIpAndPortList(any())).willReturn(
            list);
        given(mongoDBBaseService.getAppEnvResponses(any(ProtectedResource.class), any(), any(Boolean.class))).willReturn(
            list1);
        mongoDBBackupInterceptor.checkConnention(backupTask);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：MongoDB备份设置节点
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void supply_nodes_success() {
        BackupTask backupTask = new BackupTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setSubType(ResourceSubTypeEnum.MONGODB_SINGLE.getType());
        backupTask.setProtectObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        List<TaskEnvironment> nodes = new ArrayList<>();
        TaskEnvironment taskEnvironment1 = new TaskEnvironment();
        taskEnvironment1.setUuid("aaaaa");
        taskEnvironment1.setExtendInfo(new HashMap<>());
        nodes.add(taskEnvironment1);
        taskEnvironment.setNodes(nodes);
        taskEnvironment.setExtendInfo(new HashMap<>());
        backupTask.setProtectEnv(taskEnvironment);
        mongoDBBackupInterceptor.supplyNodes(backupTask);
        taskResource.setSubType(ResourceSubTypeEnum.MONGODB_CLUSTER.getType());
        List<TaskEnvironment> list = new ArrayList<>();
        list.add(new TaskEnvironment());
        given(mongoDBBaseService.buildBackupTaskNodes(any())).willReturn(list);
        mongoDBBackupInterceptor.supplyNodes(backupTask);
        Assert.assertEquals(taskEnvironment1,backupTask.getProtectEnv().getNodes().get(0));
    }

    /**
     * 用例场景：MongoDB备份设置agent
     * 前置条件：无
     * 检查点：成功
     */
    @Test
    public void supply_agent_success() throws IllegalAccessException {
        BackupTask backupTask = new BackupTask();
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("uuid");
        backupTask.setProtectObject(taskResource);
        backupTask.setAgents(new ArrayList<>());
        MemberModifier.field(MongoDBBackupInterceptor.class, "resourceService")
            .set(mongoDBBackupInterceptor, resourceService);
        DataBaseAgentSelector dataBaseAgentSelector = PowerMockito.mock(DataBaseAgentSelector.class);
        MemberModifier.field(MongoDBBackupInterceptor.class, "dataBaseAgentSelector")
            .set(mongoDBBackupInterceptor, dataBaseAgentSelector);
        mongoDBBackupInterceptor.supplyAgent(backupTask);
        Assert.assertTrue(CollectionUtils.isEmpty(backupTask.getAgents()));
    }

    @Test
    public void isSupportDataAndLogParallelBackup() {
        Assert.assertTrue(mongoDBBackupInterceptor.isSupportDataAndLogParallelBackup(new ProtectedResource()));
    }
}
