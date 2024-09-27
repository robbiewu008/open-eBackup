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
package openbackup.oracle.livemount;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.livemount.common.constants.LiveMountConstants;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCancelTask;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountCreateTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Collections;
import java.util.HashMap;

/**
 * 功能描述: test OracleLiveMountInterceptorProvider
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-02-15
 */
public class OracleLiveMountInterceptorProviderTest {
    private OracleLiveMountInterceptorProvider liveMountInterceptorProvider;
    private final AgentUnifiedService agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);

    @Before
    public void init() {
        this.liveMountInterceptorProvider = new OracleLiveMountInterceptorProvider(agentUnifiedService,
                oracleBaseService);
    }

    /**
     * 用例场景：即时挂载参数准备 <br/>
     * 前置条件：参数正常 <br/>
     * 检查点: 无异常，参数正确
     */
    @Test
    public void test_init_live_mount_create_task_param_success() {
        Mockito.when(oracleBaseService.getEnvironmentById(anyString()))
            .thenReturn(getProtectedEnvironment());
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("ip");
        endpoint.setPort(111);
        Mockito.when(oracleBaseService.getAgentEndpoint(any()))
            .thenReturn(endpoint);
        Mockito.when(agentUnifiedService.getHost(anyString(), anyInt()))
            .thenReturn(mockHostDto());
        Mockito.doNothing().when(oracleBaseService).setNodesAuth(any(), any());
        LiveMountCreateTask task = mockCreateTask();
        liveMountInterceptorProvider.initialize(task);
        Assert.assertNotNull(task.getAgents());
        Assert.assertEquals(OracleConstants.OTHER_HOST, task.getAdvanceParams().get(OracleConstants.RECOVER_TARGET));
        Assert.assertEquals("test.sh", task.getScripts().getPreScript());
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(), task.getTargetEnv().getExtendInfo()
            .get(DatabaseConstants.DEPLOY_TYPE));
    }

    /**
     * 用例场景：取消即时挂载参数准备 <br/>
     * 前置条件：参数正常 <br/>
     * 检查点: 无异常，参数正确
     */
    @Test
    public void test_init_live_mount_cancel_task_param_success() {
        Mockito.when(oracleBaseService.getEnvironmentById(anyString()))
            .thenReturn(getProtectedEnvironment());
        Endpoint endpoint = new Endpoint();
        endpoint.setIp("ip");
        endpoint.setPort(111);
        Mockito.when(oracleBaseService.getAgentEndpoint(any()))
            .thenReturn(endpoint);
        Mockito.when(agentUnifiedService.getHost(anyString(), any()))
            .thenReturn(mockHostDto());
        Mockito.doNothing().when(oracleBaseService).setNodesAuth(any(), any());
        LiveMountCancelTask liveMountCancelTask = mockCancelTask();
        liveMountInterceptorProvider.finalize(liveMountCancelTask);
        Assert.assertNotNull(liveMountCancelTask.getAgents());
    }

    /**
     * 测试场景：applicable匹配成功 <br/>
     * 前置条件：传入资源信息subtype类型为Oracle <br/>
     * 检查点：返回True
     */
    @Test
    public void test_applicable_success() {
        boolean applicable = liveMountInterceptorProvider.applicable(ResourceSubTypeEnum.ORACLE.getType());
        Assert.assertTrue(applicable);
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("111111");
        protectedEnvironment.setEndpoint("127.0.0.1");
        protectedEnvironment.setPort(3390);
        protectedEnvironment.setExtendInfo(new HashMap<>());
        return protectedEnvironment;
    }

    private LiveMountCreateTask mockCreateTask() {
        LiveMountCreateTask liveMountCreateTask = new LiveMountCreateTask();
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        liveMountCreateTask.setTargetEnv(BeanTools.copy(protectedEnvironment, TaskEnvironment::new));
        HashMap<String, Object> advanceParams = new HashMap<>();
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setType(1);
        advanceParams.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO,
            JSONObject.writeValueAsString(Collections.singletonList(liveMountFileSystemShareInfo)));
        advanceParams.put(OracleConstants.PRE_SCRIPT, "test.sh");
        liveMountCreateTask.setAdvanceParams(advanceParams);
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parent_uuid");
        liveMountCreateTask.setTargetObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        liveMountCreateTask.setRepositories(Collections.singletonList(storageRepository));
        return liveMountCreateTask;
    }

    private LiveMountCancelTask mockCancelTask() {
        LiveMountCancelTask liveMountCancelTask = new LiveMountCancelTask();
        HashMap<String, Object> advanceParams = new HashMap<>();
        LiveMountFileSystemShareInfo liveMountFileSystemShareInfo = new LiveMountFileSystemShareInfo();
        liveMountFileSystemShareInfo.setType(1);
        advanceParams.put(LiveMountConstants.FILE_SYSTEM_SHARE_INFO,
            JSONObject.writeValueAsString(Collections.singletonList(liveMountFileSystemShareInfo)));
        liveMountCancelTask.setAdvanceParams(advanceParams);
        TaskResource taskResource = new TaskResource();
        taskResource.setParentUuid("parent_uuid");
        taskResource.setRootUuid("root_uuid");
        liveMountCancelTask.setTargetObject(taskResource);
        StorageRepository storageRepository = new StorageRepository();
        storageRepository.setType(RepositoryTypeEnum.DATA.getType());
        liveMountCancelTask.setRepositories(Collections.singletonList(storageRepository));
        return liveMountCancelTask;
    }

    private HostDto mockHostDto() {
        HostDto hostDto = new HostDto();
        hostDto.setExtendInfo("");
        return hostDto;
    }
}