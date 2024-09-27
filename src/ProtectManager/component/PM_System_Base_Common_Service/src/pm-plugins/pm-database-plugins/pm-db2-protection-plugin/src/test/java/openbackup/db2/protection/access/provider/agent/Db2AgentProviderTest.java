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
package openbackup.db2.protection.access.provider.agent;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.db2.protection.access.service.Db2Service;
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
 * {@link Db2AgentProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/27
 */
public class Db2AgentProviderTest {
    private final Db2Service db2Service = PowerMockito.mock(Db2Service.class);

    private Db2AgentProvider provider = new Db2AgentProvider(db2Service);

    /**
     * 用例场景：DB2资源类型匹配
     * 前置条件：资源类型为DB2数据库和DB2表空间
     * 检查点：过滤通过返回true，否则返回false
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.DB2_DATABASE.getType())));
        Assert.assertTrue(
            provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.DB2_TABLESPACE.getType())));
        Assert.assertFalse(provider.applicable(getAgentSelectParam(ResourceSubTypeEnum.MYSQL.getType())));
    }

    /**
     * 用例场景：DB2资源的agents获取
     * 前置条件：参数正确，是DB2资源
     * 检查点：参数设置正确
     */
    @Test
    public void should_return_agents_when_execute_db2_select() {
        PowerMockito.when(db2Service.getAgentsByInstanceResource(any())).thenReturn(mockAgents());
        List<Endpoint> agents = provider.getSelectedAgents(getAgentSelectParam(ResourceSubTypeEnum.DB2_DATABASE.getType()));
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