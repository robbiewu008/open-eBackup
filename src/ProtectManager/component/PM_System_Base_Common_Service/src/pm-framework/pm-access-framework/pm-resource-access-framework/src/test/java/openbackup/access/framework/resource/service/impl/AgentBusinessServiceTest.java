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
package openbackup.access.framework.resource.service.impl;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

import openbackup.access.framework.resource.service.impl.AgentBusinessServiceImpl;
import openbackup.data.access.client.sdk.api.framework.dee.DeeLiveMountRestApi;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.service.DeployTypeService;

/**
 * AgentBusinessService 测试类
 *
 */
@SpringBootTest(classes = {AgentBusinessServiceImpl.class})
@RunWith(SpringRunner.class)
public class AgentBusinessServiceTest {
    @Autowired
    private AgentBusinessService agentBusinessService;

    @MockBean
    private ResourceService resourceService;
    @MockBean
    private SessionService sessionService;
    @MockBean
    private JobService jobService;
    @MockBean
    private CopyRestApi copyRestApi;
    @MockBean
    private AgentUnifiedService agentUnifiedService;
    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private DeeLiveMountRestApi deeLiveMountRestApi;

    /**
     * 用例名称：下发任务状态
     * 前置条件：agent正常运行
     * 检查点：下发任务状态成功
     */
    @Test
    public void task_status_should_deliver_success() {
        Mockito.when(copyRestApi.queryCopyByID(Mockito.any())).thenReturn(new Copy());
        Mockito.when(jobService.queryJob(Mockito.any())).thenReturn(new JobBo());

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setExtendInfoByKey("script", "hana");
        Mockito.when(resourceService.getBasicResourceById(Mockito.any())).thenReturn(Optional.of(protectedResource));

        Mockito.when(resourceService.queryHostByEndpoint(Mockito.any())).thenReturn(new ProtectedEnvironment());

        DeliverTaskReq deliverTaskReq = new DeliverTaskReq();
        deliverTaskReq.setTaskId("taskId");
        deliverTaskReq.setStatus("SUCCESS");
        List<DeliverTaskReq.AgentDto> agents = new ArrayList<>();
        agents.add(new DeliverTaskReq.AgentDto());
        agents.add(new DeliverTaskReq.AgentDto());
        deliverTaskReq.setAgents(agents);
        agentBusinessService.deliverTaskStatus(deliverTaskReq);

        Mockito.verify(agentUnifiedService, Mockito.times(2))
                .deliverTaskStatus(Mockito.any(), Mockito.any(), Mockito.any(), Mockito.any());
    }
}
