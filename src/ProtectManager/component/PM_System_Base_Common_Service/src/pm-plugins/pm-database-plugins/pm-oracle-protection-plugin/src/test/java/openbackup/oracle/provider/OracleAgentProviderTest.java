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
package openbackup.oracle.provider;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.List;

import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.BDDMockito.given;

public class OracleAgentProviderTest {

    private final OracleBaseService oracleBaseService = Mockito.mock(OracleBaseService.class);

    private final OracleAgentProvider oracleAgentProvider = new OracleAgentProvider(oracleBaseService);

    /**
     * 用例场景：oracle集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(protectedResource).build();
        protectedResource.setSubType(ResourceSubTypeEnum.ORACLE_CLUSTER.getType());
        Assert.assertTrue(oracleAgentProvider.applicable(agentSelectParam));
    }
    /**
     * 用例名称：orcal单机根据资源获取agent列表时，正确返回<br/>
     * 前置条件：无<br/>
     * check点：agent数量都符合期望<br/>
     */
    @Test
    public void should_return_one_when_select_ORACLE_resource() {
        // given
        given(oracleBaseService.getAgentEndpoint(any())).willReturn(new Endpoint("1", "9.9.9.9", 9999));
        AgentSelectParam agentSelectParam = AgentSelectParam.builder().resource(mockOracleResource()).build();
        // when
        List<Endpoint> returnEndPoints = oracleAgentProvider.getSelectedAgents(agentSelectParam);
        // then
        then(returnEndPoints).hasSize(1);
        Assert.assertEquals(returnEndPoints.size(),1);
    }

    private ProtectedResource mockOracleResource(){
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid("1234567");
        resource.setParentUuid("7654321");
        resource.setName("hwdb");
        resource.setSubType(ResourceSubTypeEnum.ORACLE.getType());
        return resource;
    }

}