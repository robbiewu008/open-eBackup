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
package openbackup.oceanbase.provider;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.OceanBaseTest;

import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;

/**
 * 功能描述
 *
 */
@RunWith(PowerMockRunner.class)
public class OceanBaseAgentProviderTest extends OceanBaseTest {
    private OceanBaseAgentProvider oceanBaseAgentProvider;

    @Override
    @Before
    public void init() {
        super.init();
        oceanBaseAgentProvider = new OceanBaseAgentProvider(oceanBaseService);
    }

    /**
     * 用例场景：备份场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void backup_select_success() {
        ProtectedResource resource = mockProtectedResource();
        obClient1.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
        mockGetEvnById();

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        List<Endpoint> endpointList = oceanBaseAgentProvider.getSelectedAgents(agentSelectParam);

        Endpoint endpoint1 = new Endpoint();
        endpoint1.setId(obClient2.getUuid());
        endpoint1.setPort(obClient2.getPort());
        endpoint1.setIp(obClient2.getEndpoint());

        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId(obServer1.getUuid());
        endpoint2.setPort(obServer1.getPort());
        endpoint2.setIp(obServer1.getEndpoint());

        Endpoint endpoint3 = new Endpoint();
        endpoint3.setId(obServer2.getUuid());
        endpoint3.setPort(obServer2.getPort());
        endpoint3.setIp(obServer2.getEndpoint());
        assertThat(endpointList).usingRecursiveComparison()
            .isEqualTo(Lists.newArrayList(endpoint1, endpoint2, endpoint3));
    }

    /**
     * 用例场景：回復场景查询agent成功
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void restore_select_success() {
        ProtectedResource resource = mockProtectedResource();
        mockGetEvnById();

        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.RESTORE.getValue())
            .build();

        List<Endpoint> endpointList = oceanBaseAgentProvider.getSelectedAgents(agentSelectParam);

        Endpoint endpoint1 = new Endpoint();
        endpoint1.setId(obClient1.getUuid());
        endpoint1.setPort(obClient1.getPort());
        endpoint1.setIp(obClient1.getEndpoint());

        Endpoint endpoint2 = new Endpoint();
        endpoint2.setId(obServer1.getUuid());
        endpoint2.setPort(obServer1.getPort());
        endpoint2.setIp(obServer1.getEndpoint());

        Endpoint endpoint3 = new Endpoint();
        endpoint3.setId(obServer2.getUuid());
        endpoint3.setPort(obServer2.getPort());
        endpoint3.setIp(obServer2.getEndpoint());

        assertThat(endpointList).usingRecursiveComparison()
            .isEqualTo(Lists.newArrayList(endpoint1, endpoint2, endpoint3));
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType());
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(nodeResource1).build();
        boolean applicable = oceanBaseAgentProvider.applicable(agentSelectParam);
        Assert.assertTrue(applicable);
    }
}
