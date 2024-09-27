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
package openbackup.kingbase.protection.access.provider.agent;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.kingbase.protection.access.service.KingBaseService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

/**
 * {@link KingBaseAgentProvider} 参数类
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/27
 */
public class KingBaseAgentProviderTest {
    private final KingBaseService kingBaseService = PowerMockito.mock(KingBaseService.class);

    private KingBaseAgentProvider provider = new KingBaseAgentProvider(kingBaseService);

    /**
     * 用例场景：kingbase资源类型匹配
     * 前置条件：资源类型为kingbase实例和集群实例
     * 检查点：过滤检查返回true，否则返回false
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType())));
        Assert.assertTrue(
            provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.KING_BASE_CLUSTER_INSTANCE.getType())));
        Assert.assertFalse(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.MYSQL.getType())));
    }

    /**
     * 用例场景：kingbase资源的agents获取
     * 前置条件：参数正确，是kingbase资源
     * 检查点：参数设置正确
     */
    @Test
    public void should_return_agents_when_execute_kingbase_select() {
        PowerMockito.when(kingBaseService.getAgentsByInstanceResource(any())).thenReturn(mockAgents());
        List<Endpoint> agents = provider.getSelectedAgents(getAgentSelectParam(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType()));
        Assert.assertEquals(IsmNumberConstant.ONE, agents.size());
    }

    private AgentSelectParam getAgentSelectParam(String subType) {
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(UUIDGenerator.getUUID());
        resource.setSubType(subType);
        return AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(new HashMap<>())
            .build();
    }

    private List<Endpoint> mockAgents() {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(UUIDGenerator.getUUID());
        endpoint.setIp("127.0.0.1");
        endpoint.setPort(59521);
        List<Endpoint> agents = new ArrayList<>();
        agents.add(endpoint);
        return agents;
    }
}