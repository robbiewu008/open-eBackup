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
package openbackup.oceanbase;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.common.dto.OBAgentInfo;
import openbackup.oceanbase.common.dto.OBClusterInfo;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import com.google.common.collect.Lists;

import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 */
public class OceanBaseTest {
    protected OceanBaseService oceanBaseService;

    protected ProtectedEnvironment obClient1;

    protected ProtectedEnvironment obClient2;

    protected ProtectedEnvironment obServer1;

    protected ProtectedEnvironment obServer2;

    public void init() {
        oceanBaseService = Mockito.mock(OceanBaseService.class);
        obClient1 = new ProtectedEnvironment();
        obClient1.setEndpoint("192.168.129.12");
        obClient1.setPort(59512);
        obClient1.setUuid("111-111-111");
        obClient1.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        obClient2 = new ProtectedEnvironment();
        obClient2.setEndpoint("192.168.129.13");
        obClient2.setPort(59513);
        obClient2.setUuid("111-111-222");
        obClient2.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        obServer1 = new ProtectedEnvironment();
        obServer1.setEndpoint("192.168.129.22");
        obServer1.setPort(59522);
        obServer1.setUuid("111-222-111");
        obServer1.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        obServer2 = new ProtectedEnvironment();
        obServer2.setEndpoint("192.168.129.23");
        obServer2.setPort(59523);
        obServer2.setUuid("111-222-222");
        obServer2.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    protected void mockGetEvnById() {
        PowerMockito.when(oceanBaseService.getEnvironmentById(obClient1.getUuid())).thenReturn(obClient1);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obClient2.getUuid())).thenReturn(obClient2);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obServer1.getUuid())).thenReturn(obServer1);
        PowerMockito.when(oceanBaseService.getEnvironmentById(obServer2.getUuid())).thenReturn(obServer2);
        ProtectedResource resource = mockProtectedResource();
        PowerMockito.when(oceanBaseService.getResourceById(resource.getUuid())).thenReturn(Optional.of(resource));
        ProtectedEnvironment environment = mockProtectedEnvironment();
        PowerMockito.when(oceanBaseService.getEnvironmentById(environment.getUuid())).thenReturn(environment);
    }

    protected ProtectedResource mockProtectedResource() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"name\":\"modifyOceanBaseClusterName\",\"rootUuid\":\"OBClusterUuid\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"OBClusterUuid\"}";
        ProtectedResource res = JsonUtil.read(param, ProtectedResource.class);
        res.setExtendInfo(mockExtendInfo());
        res.setDependencies(mockDependencies());
        return res;
    }

    protected ProtectedEnvironment mockProtectedEnvironment() {
        String param
            = "{\"auth\":{\"authKey\":\"root\",\"authPwd\":\"123456\",\"authType\":2},\"name\":\"modifyOceanBaseClusterName\",\"rootUuid\":\"OBClusterUuid\",\"subType\":\"OceanBase-cluster\",\"type\":\"Database\",\"uuid\":\"OBClusterUuid\"}";
        ProtectedEnvironment res = JsonUtil.read(param, ProtectedEnvironment.class);
        res.setExtendInfo(mockExtendInfo());
        res.setDependencies(mockDependencies());
        return res;
    }

    protected ProtectedResource mockTenantResource() {
        String param
            = "{\"extendInfo\":{\"clusterInfo\":\"{\\\"tenantInfos\\\":[{\\\"name\\\":\\\"tenant1\\\"},{\\\"name\\\":\\\"tenant3\\\"}]}\"},\"name\":\"租户集名称2\",\"parentUuid\":\"OBClusterUuid\",\"rootUuid\":\"OBClusterUuid\",\"sourceType\":\"register\",\"subType\":\"OceanBase-tenant\",\"type\":\"Database\",\"uuid\":\"OBTenantSetUuid\"}";
        return JsonUtil.read(param, ProtectedResource.class);
    }

    private Map<String, List<ProtectedResource>> mockDependencies() {
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        dependencies.put(OBConstants.CLIENT_AGENTS, Lists.newArrayList(obClient1, obClient2));
        dependencies.put(OBConstants.SERVER_AGENTS, Lists.newArrayList(obServer1, obServer2));
        return dependencies;
    }

    private Map<String, String> mockExtendInfo() {
        OBAgentInfo agent1 = new OBAgentInfo();
        agent1.setParentUuid(obClient1.getUuid());
        agent1.setNodeType("OBClient");
        agent1.setLinkStatus(obClient1.getLinkStatus());

        OBAgentInfo agent2 = new OBAgentInfo();
        agent2.setParentUuid(obClient2.getUuid());
        agent2.setNodeType("OBClient");
        agent2.setLinkStatus(obClient2.getLinkStatus());

        OBAgentInfo agent3 = new OBAgentInfo();
        agent3.setParentUuid(obServer1.getUuid());
        agent3.setNodeType("OBClient");
        agent3.setLinkStatus(obServer1.getLinkStatus());
        agent3.setIp("8.40.129.22");
        agent3.setPort("2881");

        OBAgentInfo agent4 = new OBAgentInfo();
        agent4.setParentUuid(obServer2.getUuid());
        agent4.setNodeType("OBClient");
        agent4.setLinkStatus(obServer2.getLinkStatus());
        agent4.setIp("8.40.129.23");
        agent4.setPort("2881");

        List<OBAgentInfo> obClientAgents = Lists.newArrayList(agent1, agent2);
        List<OBAgentInfo> obServerAgents = Lists.newArrayList(agent3, agent4);

        OBClusterInfo clusterInfo = new OBClusterInfo();
        clusterInfo.setObClientAgents(obClientAgents);
        clusterInfo.setObServerAgents(obServerAgents);
        Map<String, String> map = new HashMap<>();
        map.put(OBConstants.KEY_CLUSTER_INFO, JsonUtil.json(clusterInfo));
        return map;
    }
}
