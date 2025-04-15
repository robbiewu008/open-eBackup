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
package openbackup.postgre.protection.access.provider.restore;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.postgre.protection.access.service.PostgreInstanceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Random;
import java.util.UUID;

/**
 * Postgre实例恢复任务下发provider测试类
 *
 */
public class PostgreInstanceRestoreProviderTest {
    private final PostgreInstanceService postgreInstanceService = PowerMockito.mock(PostgreInstanceService.class);

    private final ProtectedEnvironmentService environmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

    private final AgentUnifiedService agentUnifiedService = PowerMockito.mock(AgentUnifiedService.class);

    private final CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private final PostgreInstanceRestoreProvider provider = new PostgreInstanceRestoreProvider(postgreInstanceService,
        environmentService, agentUnifiedService, copyRestApi);

    private RestoreTask mockRestoreTask(String subType, RestoreLocationEnum restoreLocation) {
        RestoreTask task = new RestoreTask();
        task.setCopyId(UUID.randomUUID().toString());
        task.setTargetLocation(restoreLocation);
        task.setRestoreType(RestoreTypeEnum.CR.getType());
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid(UUID.randomUUID().toString());
        taskResource.setSubType(subType);
        taskResource.setExtendInfo(new HashMap<>());
        task.setTargetObject(taskResource);
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        taskEnvironment.setUuid(UUID.randomUUID().toString());
        taskEnvironment.setNodes(new ArrayList<>());
        taskEnvironment.setExtendInfo(new HashMap<>());
        task.setTargetEnv(taskEnvironment);
        return task;
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入Postgre单实例、Postgre集群实例资源类型
     * 检查点：返回true
     */
    @Test
    public void postgre_instance_applicable_success() {
        // postgre单机实例
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType()));
        // postgre集群实例
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType()));
    }

    /**
     * 用例场景：框架调 applicable接口
     * 前置条件：applicable输入非Postgre实例资源类型
     * 检查点：返回false
     */
    @Test
    public void should_return_false_if_not_postgres_instance_when_applicable() {
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.POSTGRESQL.getType()));
        Assert.assertFalse(provider.applicable(ResourceSubTypeEnum.POSTGRE_CLUSTER.getType()));
    }

    private ProtectedEnvironment mockClusterEnvironment() {
        ProtectedEnvironment clusterEnv = new ProtectedEnvironment();
        List<ProtectedResource> nodes = new ArrayList<>();
        List<String> nodeIds = Arrays.asList("9b0c9bad-b7d1-49f1-b1cf-e07cf4f04c9b",
            "dd370464-793a-4248-a3d7-c51b97f185af", "c4739825-3508-4e68-aba2-896191726e7d");
        List<String> nodeIps = Arrays.asList("10.10.10.11", "10.10.10.12", "10.10.10.13");
        for (int i = 0; i < 3; i++) {
            ProtectedEnvironment tmpNode = new ProtectedEnvironment();
            tmpNode.setUuid(nodeIds.get(i));
            tmpNode.setEndpoint(nodeIps.get(i));
            tmpNode.setPort(new Random().nextInt(65535));
            tmpNode.setExtendInfo(new HashMap<>());
            nodes.add(tmpNode);
        }
        Map<String, List<ProtectedResource>> envDependencies = new HashMap<>();
        envDependencies.put(DatabaseConstants.AGENTS, nodes);
        clusterEnv.setDependencies(envDependencies);
        return clusterEnv;
    }

    private ProtectedResource mockClusterInstResource() {
        ProtectedResource clusterInst = new ProtectedResource();
        clusterInst.setVersion("9.4.0");
        clusterInst.setExtendInfoByKey(PostgreConstants.DB_OS_USER_KEY, "postgres");
        List<String> nodeIds = Arrays.asList("9b0c9bad-b7d1-49f1-b1cf-e07cf4f04c9b",
            "dd370464-793a-4248-a3d7-c51b97f185af", "c4739825-3508-4e68-aba2-896191726e7d");
        List<ProtectedResource> subInstances = new ArrayList<>();
        Map<String, List<ProtectedResource>> instDependencies = new HashMap<>();
        for (int i = 0; i < 3; i++) {
            ProtectedEnvironment tmpInst = new ProtectedEnvironment();
            tmpInst.setUuid(UUID.randomUUID().toString());
            tmpInst.setType(ResourceTypeEnum.DATABASE.getType());
            tmpInst.setSubType(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
            Map<String, String> tmpExtendInfo = new HashMap<>();
            tmpExtendInfo.put(DatabaseConstants.HOST_ID, nodeIds.get(i));
            if (i == 0) {
                tmpExtendInfo.put(DatabaseConstants.ROLE, "1");
            } else {
                tmpExtendInfo.put(DatabaseConstants.ROLE, "2");
            }
            tmpInst.setExtendInfo(tmpExtendInfo);
            subInstances.add(tmpInst);
        }
        instDependencies.put(DatabaseConstants.CHILDREN, subInstances);
        clusterInst.setDependencies(instDependencies);
        return clusterInst;
    }

    /**
     * 用例场景：集群实例恢复设置任务参数
     * 前置条件：1）副本类型非云归档副本和磁带归档副本；2）原位置恢复
     * 检查点：设置恢复任务参数正确
     */
    @Test
    public void supply_restore_task_if_single_inst_and_not_cloud_or_tap_archive_copy_success() {
        RestoreTask task = mockRestoreTask(ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType(),
            RestoreLocationEnum.ORIGINAL);
        Copy copy = new Copy();
        copy.setGeneratedBy(CopyGeneratedByEnum.BY_BACKUP.value());
        copy.setResourceProperties("{\"version\":\"9.4.0\", \"extendInfo\": {\"osUsername\": \"postgres\"}}");
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        ProtectedEnvironment clusterEnv = mockClusterEnvironment();
        PowerMockito.when(environmentService.getEnvironmentById(anyString())).thenReturn(clusterEnv);
        ProtectedResource clusterInst = mockClusterInstResource();
        PowerMockito.when(postgreInstanceService.getResourceById(anyString())).thenReturn(clusterInst);
        provider.supplyRestoreTask(task);
        Assert.assertEquals(RestoreModeEnum.LOCAL_RESTORE.getMode(), task.getRestoreMode());
        Assert.assertEquals(RestoreLocationEnum.ORIGINAL.getLocation(),
            task.getAdvanceParams().get(PostgreConstants.TARGET_LOCATION_KEY));
        Assert.assertEquals(DatabaseDeployTypeEnum.AP.getType(),
            task.getTargetEnv().getExtendInfo().get(DatabaseConstants.DEPLOY_TYPE));
        Assert.assertEquals(3, task.getSubObjects().size());
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：源实例版本不合法
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_src_version_is_invalid_when_isVersionMatched() {
        String srcVersion = "9";
        String tgtVersion = "13.7";
        ReflectionTestUtils.invokeMethod(PostgreInstanceRestoreProvider.class, "isVersionMatched", srcVersion,
            tgtVersion);
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：目标实例版本不合法
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_tgt_version_is_invalid_when_isVersionMatched() {
        String srcVersion = "9.4.3";
        String tgtVersion = "12";
        ReflectionTestUtils.invokeMethod(PostgreInstanceRestoreProvider.class, "isVersionMatched", srcVersion,
            tgtVersion);
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：源实例版本和目标实例版本不一致
     * 检查点：返回false
     */
    @Test
    public void should_return_false_if_v9_minor_version_not_equal_when_isVersionMatched() {
        String srcVersion = "13.3";
        String tgtVersion = "13.7";
        Assert.assertFalse(
            ReflectionTestUtils.invokeMethod(PostgreInstanceRestoreProvider.class, "isVersionMatched", srcVersion,
                tgtVersion));

        String srcVersion2 = "9.4.3";
        String tgtVersion2 = "9.6.3";
        Assert.assertFalse(
            ReflectionTestUtils.invokeMethod(PostgreInstanceRestoreProvider.class, "isVersionMatched", srcVersion2,
                tgtVersion2));
    }

    /**
     * 用例场景：检查源实例版本和目标实例版本是否匹配
     * 前置条件：源实例版本和目标实例版本一致
     * 检查点：返回true
     */
    @Test
    public void should_return_true_if_v9_minor_version_not_equal_when_isVersionMatched() {
        String srcVersion = "9.5.5";
        String tgtVersion = "9.5.5";
        Assert.assertTrue(
            ReflectionTestUtils.invokeMethod(PostgreInstanceRestoreProvider.class, "isVersionMatched", srcVersion,
                tgtVersion));

        String srcVersion2 = "11.6";
        String tgtVersion2 = "11.6";
        Assert.assertTrue(
            ReflectionTestUtils.invokeMethod(PostgreInstanceRestoreProvider.class, "isVersionMatched", srcVersion2,
                tgtVersion2));
    }

    private RestoreTask mockRestoreTaskForCheckSupportRestore() {
        RestoreTask task = new RestoreTask();
        task.setCopyId("7598f0a0-7229-42bf-a4c7-26169b2405ac");
        TaskResource taskResource = new TaskResource();
        taskResource.setUuid("babb9ab1-f283-45d5-85fe-8f4718687a02");
        task.setTargetObject(taskResource);
        return task;
    }

    /**
     * 用例场景：检查是否支持恢复
     * 前置条件：源实例和目标实例版本不匹配
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void test_should_throw_LegoCheckedException_if_ver_not_match_when_checkSupportRestore() {
        RestoreTask task = mockRestoreTaskForCheckSupportRestore();
        Copy copy = new Copy();
        copy.setResourceProperties("{\"version\":\"9.4.0\"}}");
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setVersion("13.7");
        PowerMockito.when(postgreInstanceService.getResourceById(anyString())).thenReturn(protectedResource);
        ReflectionTestUtils.invokeMethod(provider, "checkSupportRestore", task);
    }

    /**
     * 用例场景：检查是否支持恢复
     * 前置条件：1）源实例和目标实例版本匹配；2）源实例和目标实例操作系统用户不一致
     * 检查点：抛出LegoCheckedException异常
     */
    @Test(expected = LegoCheckedException.class)
    public void test_should_throw_LegoCheckedException_if_os_user_is_diff_when_checkSupportRestore() {
        RestoreTask task = mockRestoreTaskForCheckSupportRestore();
        Copy copy = new Copy();
        copy.setResourceProperties("{\"version\":\"9.2.0\", \"extendInfo\": {\"osUsername\": \"postgres\"}}");
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setVersion("9.2.7");
        protectedResource.setExtendInfoByKey(PostgreConstants.DB_OS_USER_KEY, "root");
        PowerMockito.when(postgreInstanceService.getResourceById(anyString())).thenReturn(protectedResource);
        ReflectionTestUtils.invokeMethod(provider, "checkSupportRestore", task);
    }

    /**
     * 用例场景：检查是否支持恢复
     * 前置条件：1）源实例和目标实例版本匹配；2）源实例和目标实例操作系统用户一致
     * 检查点：正常执行不抛异常返回null
     */
    @Test
    public void test_check_support_restore_success() {
        RestoreTask task = mockRestoreTaskForCheckSupportRestore();
        Copy copy = new Copy();
        copy.setResourceProperties("{\"version\":\"9.2.7\", \"extendInfo\": {\"osUsername\": \"postgres\"}}");
        PowerMockito.when(copyRestApi.queryCopyByID(anyString())).thenReturn(copy);
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setVersion("9.2.7");
        protectedResource.setExtendInfoByKey(PostgreConstants.DB_OS_USER_KEY, "postgres");
        PowerMockito.when(postgreInstanceService.getResourceById(anyString())).thenReturn(protectedResource);
        Assert.assertNull(ReflectionTestUtils.invokeMethod(provider, "checkSupportRestore", task));
    }
}
