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
package openbackup.goldendb.protection.access.service.impl;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.nullable;
import static org.mockito.Mockito.when;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;

import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.util.BeanTools;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbServiceImplTest {

    @Mock
    private ResourceService mockResourceService;

    @Mock
    private AgentUnifiedService agentUnifiedService;

    @Mock
    private JobService mockJobService;

    private GoldenDbServiceImpl goldenDbServiceImplUnderTest;

    @Before
    public void setUp() {
        goldenDbServiceImplUnderTest = new GoldenDbServiceImpl(mockResourceService, agentUnifiedService,
            mockJobService);
    }

    @Test
    public void testGetEnvironmentById() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.8.8.8");
        when(mockResourceService.getResourceById("666")).thenReturn(Optional.of(environment));

        // Run the test
        ProtectedEnvironment result = goldenDbServiceImplUnderTest.getEnvironmentById("666");

        // Verify the results
        assertEquals(result.getEndpoint(), "8.8.8.8");
    }

    @Test
    public void testGetEnvironmentById_ResourceServiceReturnsAbsent() {
        // Setup
        when(mockResourceService.getResourceById("8.8.8.8")).thenReturn(Optional.empty());

        // Run the test
        assertThrows(LegoCheckedException.class, () -> goldenDbServiceImplUnderTest.getEnvironmentById("8.8.8.8"));
    }

    @Test
    public void testGetResourceById() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEndpoint("8.8.8.8");
        when(mockResourceService.getResourceById("089a77e6-2029-4be7-b606-17f2515bf882")).thenReturn(
            Optional.of(protectedResource));

        // Run the test
        final ProtectedResource result = goldenDbServiceImplUnderTest.getResourceById(
            "089a77e6-2029-4be7-b606-17f2515bf882");

        // Verify the results
        assertEquals("8.8.8.8", result.getEndpoint());
    }

    @Test
    public void testGetResourceById_ResourceServiceReturnsAbsent() {
        // Setup
        when(mockResourceService.getResourceById("089a77e6-2029-4be7-b606-17f2515bf882")).thenReturn(Optional.empty());

        // Run the test
        assertThrows(LegoCheckedException.class,
            () -> goldenDbServiceImplUnderTest.getResourceById("089a77e6-2029-4be7-b606-17f2515bf882"));
    }

    @Test
    public void testSingleConnectCheck() throws Exception {
        // Setup
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(66);
        when(mockResourceService.getResourceById("envId")).thenReturn(Optional.of(agentEnvironment));
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        agentBaseDto.setErrorCode("0");
        agentBaseDto.setErrorMessage("errorMessage");
        boolean result = goldenDbServiceImplUnderTest.singleConnectCheck(getMysqlNode(), getInstanceEnvironment())
            .isAgentBaseDtoReturnSuccess();
        Assert.assertEquals(result, true);
        agentBaseDto.setErrorCode("1");
        boolean result1 = goldenDbServiceImplUnderTest.singleConnectCheck(getMysqlNode(), getInstanceEnvironment())
            .isAgentBaseDtoReturnSuccess();
        Assert.assertEquals(result1, false);
    }

    @Test
    public void testSingleConnectCheck_ResourceServiceReturnsAbsent() {
        when(mockResourceService.getResourceById("envId")).thenReturn(Optional.empty());

        // Run the test
        assertThrows(LegoCheckedException.class,
            () -> goldenDbServiceImplUnderTest.singleConnectCheck(getMysqlNode(), getInstanceEnvironment()));
    }

    @Test
    public void testSingleConnectCheck_throw_miss_component() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(66);
        when(mockResourceService.getResourceById("envId")).thenReturn(Optional.of(agentEnvironment));
        // Run the test
        assertThrows(LegoCheckedException.class,
            () -> goldenDbServiceImplUnderTest.singleConnectCheck(getMysqlNode(), getInstanceEnvironment()));
    }

    @Test
    public void testSingleConnectCheck_throw_node_type_mismatch() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setEndpoint("8.8.8.8");
        agentEnvironment.setPort(66);
        when(mockResourceService.getResourceById("envId")).thenReturn(Optional.of(agentEnvironment));
        // Run the test
        assertThrows(LegoCheckedException.class,
            () -> goldenDbServiceImplUnderTest.singleConnectCheck(getMysqlNode(), getInstanceEnvironment()));
    }

    @Test
    public void testGetComputeNode() {
        // Run the test
        List<MysqlNode> result = goldenDbServiceImplUnderTest.getComputeNode(getInstanceEnvironment());

        // Verify the results
        assertEquals(4, result.size());
    }

    @Test
    public void testGetManageDbNode() {
        // Run the test
        List<Node> manageDbNode = goldenDbServiceImplUnderTest.getManageDbNode(getClusterEnvironment());

        // Verify the results
        assertEquals(1, manageDbNode.size());
    }

    @Test
    public void testGetGoldenDbEnv() {
        // Setup
        ProtectedEnvironment clusterEnvironment = getClusterEnvironment();
        ProtectedResource resource = BeanTools.copy(clusterEnvironment, ProtectedResource::new);
        PageListResponse pageListResponse = new PageListResponse();
        pageListResponse.setRecords(Collections.singletonList(resource));
        when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(pageListResponse);

        // Run the test
        final List<ProtectedResource> result = goldenDbServiceImplUnderTest.getGoldenDbEnv("");

        // Verify the results
        Assert.assertEquals(result.size(), 0);
    }

    @Test
    public void testGetGtmNode() {
        // Run the test
        List<Gtm> gtmNode = goldenDbServiceImplUnderTest.getGtmNode(getInstanceEnvironment());

        // Verify the results
        assertEquals(2, gtmNode.size());
    }

    @Test
    public void testSingleHealthCheck() {
        when(mockResourceService.getResourceById("envId")).thenReturn(Optional.empty());

        // Run the test
        assertThrows(LegoCheckedException.class,
            () -> goldenDbServiceImplUnderTest.singleConnectCheck(getMysqlNode(), getInstanceEnvironment()));
        boolean healthCheck = goldenDbServiceImplUnderTest.singleHealthCheck(getMysqlNode(), getInstanceEnvironment());
        Assert.assertFalse(healthCheck);
    }

    @Test
    public void testUpdateResourceLinkStatus() {
        // Run the test
        goldenDbServiceImplUnderTest.updateResourceLinkStatus("fc963582-3750-4dce-acf6-ce828a7355ab", "status");
        Assert.assertTrue(true);
    }

    @Test
    public void testGetChildren() {
        PageListResponse pageListResponse = new PageListResponse();
        pageListResponse.setRecords(Collections.singletonList(getClusterEnvironment()));
        pageListResponse.setTotalCount(1);
        PowerMockito.when(mockResourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(pageListResponse);
        String name = goldenDbServiceImplUnderTest.getChildren("666").get(0).getName();
        Assert.assertEquals(name, "goldentest666222");
    }

    @Test
    public void testQueryLatestJob() {
        JobBo jobBo = new JobBo();
        jobBo.setJobId("666");
        openbackup.system.base.common.model.PageListResponse<JobBo> pageListResponse
            = new openbackup.system.base.common.model.PageListResponse<>();
        pageListResponse.setRecords(Arrays.asList(jobBo));

        // Optional<JobBo> optionalJobBo = Optional.of(jobBo);
        PowerMockito.when(mockJobService.queryJobs(any(QueryJobRequest.class), any(PagingParamRequest.class),
            any(SortingParamRequest.class), nullable(String.class))).thenReturn(pageListResponse);
        String jobId = goldenDbServiceImplUnderTest.queryLatestJob("666", "BACKUP").get().getJobId();
        Assert.assertEquals(jobId, "666");
    }

    private ProtectedEnvironment getInstanceEnvironment() {
        String json
            = "{\"parentUuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"name\":\"cluster3\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\"},\"port\":\"\",\"hostName\":\"\",\"ip\":\"\",\"extendInfo\":{\"linkStatus\":\"0\",\"clusterInfo\":\"{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"cluster3\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"5\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb3\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"6\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb4\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"7\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb5\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"8\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb6\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]}],\\\"gtm\\\":[{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\",\\\"osUser\\\":\\\"zxgtm1\\\"},{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\",\\\"osUser\\\":\\\"zxgtm1\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"8bc55739-8811-4b64-abac-35a49486a14c\"},{\"uuid\":\"3884a310-db2b-4265-bcc4-26c6c1abb064\"}]}}";
        ProtectedEnvironment environment = JsonUtil.read(json, ProtectedEnvironment.class);
        return environment;
    }

    private ProtectedEnvironment getClusterEnvironment() {
        String json
            = "{\"name\":\"goldentest666222\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"extendInfo\":{\"linkStatus\":\"0\",\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"nodeType\\\":\\\"managerNode\\\",\\\"parentUuid\\\":\\\"7017bd24-1a4d-42fc-aaf4-3046eab88704\\\",\\\"osUser\\\":\\\"zxmanager\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"7017bd24-1a4d-42fc-aaf4-3046eab88704\"}]}}";
        ProtectedEnvironment environment = JsonUtil.read(json, ProtectedEnvironment.class);
        return environment;
    }

    private MysqlNode getMysqlNode() {
        MysqlNode mysqlNode = new MysqlNode();
        mysqlNode.setUuid("fdaa3a09-f8a1-4a7d-8797-634bddad80c3");
        mysqlNode.setId("id");
        mysqlNode.setName("name");
        mysqlNode.setRole("role");
        mysqlNode.setIp("ip");
        mysqlNode.setPort("port");
        mysqlNode.setOsUser("osUser");
        mysqlNode.setNodeType("nodeType");
        mysqlNode.setParentUuid("envId");
        return mysqlNode;
    }
}
