/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.provider;

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;

import java.util.List;
import java.util.Map;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbClusterConnectionCheckerTest {

    @Mock
    private ProtectedEnvironmentRetrievalsService mockEnvironmentRetrievalsService;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private GoldenDbService mockGoldenDbService;

    private GoldenDbClusterConnectionChecker goldenDbClusterConnectionCheckerUnderTest;

    @Before
    public void setUp() {
        goldenDbClusterConnectionCheckerUnderTest = new GoldenDbClusterConnectionChecker(
            mockEnvironmentRetrievalsService, mockAgentUnifiedService, mockGoldenDbService);
    }

    @Test
    public void testApplicable() {
        // Setup
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());

        // Run the test
        boolean result = goldenDbClusterConnectionCheckerUnderTest.applicable(resource);

        // Verify the results
        assertTrue(result);
    }

    @Test
    public void testCollectConnectableResources() {
        when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(getEnvironment());

        // Run the test
        final Map<ProtectedResource, List<ProtectedEnvironment>> result =
            goldenDbClusterConnectionCheckerUnderTest.collectConnectableResources(getEnvironment());
        Assert.assertEquals(result.size(),1);
    }

    private ProtectedEnvironment getEnvironment(){
        String json ="{\"name\":\"goldentest666222\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"extendInfo\":{\"linkStatus\":\"0\",\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"nodeType\\\":\\\"managerNode\\\",\\\"parentUuid\\\":\\\"7017bd24-1a4d-42fc-aaf4-3046eab88704\\\",\\\"osUser\\\":\\\"zxmanager\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"7017bd24-1a4d-42fc-aaf4-3046eab88704\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }
}
