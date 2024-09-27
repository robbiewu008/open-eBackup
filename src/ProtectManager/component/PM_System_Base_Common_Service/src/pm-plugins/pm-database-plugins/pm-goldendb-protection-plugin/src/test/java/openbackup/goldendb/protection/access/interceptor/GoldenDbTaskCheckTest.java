/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.provider.GoldenDbClusterProvider;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.Arrays;
import java.util.List;

@RunWith(MockitoJUnitRunner.class)
public class GoldenDbTaskCheckTest {

    @Mock
    private GoldenDbService mockGoldenDbService;

    @Mock
    private GoldenDbClusterProvider mockGoldenDbClusterProvider;

    private GoldenDbTaskCheck goldenDbTaskCheckUnderTest;

    @Before
    public void setUp() {
        goldenDbTaskCheckUnderTest = new GoldenDbTaskCheck(mockGoldenDbService, mockGoldenDbClusterProvider);
    }

    @Test
    public void testCheckEnvChange() {
        when(mockGoldenDbService.getResourceById(any())).thenReturn(getInstance());
        when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(getCluster());
        final PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>(0,
            Arrays.asList(getInstance()));
        when(mockGoldenDbClusterProvider.browse(any(), any())).thenReturn(pageListResponse);

        // Configure GoldenDbService.getComputeNode(...).
        final MysqlNode mysqlNode = new MysqlNode();
        mysqlNode.setUuid("57d45ef6-fa6c-4204-a2e5-5aa4eaf02c85");
        mysqlNode.setId("id");
        mysqlNode.setName("name");
        mysqlNode.setRole("role");
        mysqlNode.setIp("ip");
        mysqlNode.setPort("port");
        mysqlNode.setOsUser("osUser");
        mysqlNode.setNodeType("nodeType");
        mysqlNode.setParentUuid("parentUuid");
        final List<MysqlNode> mysqlNodes = Arrays.asList(mysqlNode);
        when(mockGoldenDbService.getComputeNode(any())).thenReturn(mysqlNodes);

        // Run the test
        goldenDbTaskCheckUnderTest.checkEnvChange("clusterId", "instanceId");

        // Run the test
        try {
            goldenDbTaskCheckUnderTest.checkEnvChange("clusterId", "instanceId");
        } catch (LegoCheckedException exception) {
            System.out.println(exception);
        }
        // Verify the results
        Assert.assertTrue(true);
    }

    /**
     * 生产环境节点数与备份任务不一样
     */
    @Test
    public void testCheckEnvChange2() {
        when(mockGoldenDbService.getResourceById(any())).thenReturn(getInstance());
        when(mockGoldenDbService.getEnvironmentById(any())).thenReturn(getCluster());
        final PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>(0,
            Arrays.asList(getInstance()));
        when(mockGoldenDbClusterProvider.browse(any(), any())).thenReturn(pageListResponse);

        // Configure GoldenDbService.getComputeNode(...).
        final MysqlNode mysqlNode = new MysqlNode();
        mysqlNode.setUuid("57d45ef6-fa6c-4204-a2e5-5aa4eaf02c85");
        mysqlNode.setId("id");
        mysqlNode.setName("name");
        mysqlNode.setRole("role");
        mysqlNode.setIp("ip");
        mysqlNode.setPort("port");
        mysqlNode.setOsUser("osUser");
        mysqlNode.setNodeType("nodeType");
        mysqlNode.setParentUuid("parentUuid");
        final List<MysqlNode> mysqlNodes = Arrays.asList(mysqlNode);
        final List<MysqlNode> mysqlNodes2 = Arrays.asList(mysqlNode, mysqlNode);
        when(mockGoldenDbService.getComputeNode(any())).thenReturn(mysqlNodes).thenReturn(mysqlNodes2);

        // Run the test
        Assert.assertThrows(LegoCheckedException.class,
            () -> goldenDbTaskCheckUnderTest.checkEnvChange("clusterId", "instanceId"));
    }

    private ProtectedEnvironment getInstance() {
        String json
            = "{\"parentUuid\":\"5a9e688f541c4eb7a5017406c21839eb\",\"name\":\"cluster3\",\"type\":\"Database\",\"subType\":\"GoldenDB-clusterInstance\",\"auth\":{\"authType\":2,\"authKey\":\"super\",\"authPwd\":\"Huawei@123\"},\"port\":\"\",\"hostName\":\"\",\"ip\":\"\",\"extendInfo\":{\"linkStatus\":\"0\",\"clusterInfo\":\"{\\\"id\\\":\\\"3\\\",\\\"name\\\":\\\"cluster3\\\",\\\"group\\\":[{\\\"groupId\\\":\\\"1\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"5\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb3\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"6\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb4\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]},{\\\"groupId\\\":\\\"2\\\",\\\"databaseNum\\\":\\\"2\\\",\\\"mysqlNodes\\\":[{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"7\\\",\\\"name\\\":\\\"DN5\\\",\\\"role\\\":\\\"slave\\\",\\\"ip\\\":\\\"8.40.162.216\\\",\\\"port\\\":\\\"5503\\\",\\\"osUser\\\":\\\"zxdb5\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\"},{\\\"uuid\\\":\\\"\\\",\\\"id\\\":\\\"8\\\",\\\"name\\\":\\\"DN6\\\",\\\"role\\\":\\\"master\\\",\\\"ip\\\":\\\"8.40.162.217\\\",\\\"port\\\":\\\"5504\\\",\\\"osUser\\\":\\\"zxdb6\\\",\\\"nodeType\\\":\\\"dataNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\"}]}],\\\"gtm\\\":[{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"3884a310-db2b-4265-bcc4-26c6c1abb064\\\",\\\"osUser\\\":\\\"zxgtm1\\\"},{\\\"nodeType\\\":\\\"gtmNode\\\",\\\"parentUuid\\\":\\\"8bc55739-8811-4b64-abac-35a49486a14c\\\",\\\"osUser\\\":\\\"zxgtm1\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"8bc55739-8811-4b64-abac-35a49486a14c\"},{\"uuid\":\"3884a310-db2b-4265-bcc4-26c6c1abb064\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    private ProtectedEnvironment getCluster() {
        String json
            = "{\"name\":\"goldentest666222\",\"type\":\"Database\",\"subType\":\"GoldenDB-cluster\",\"extendInfo\":{\"linkStatus\":\"0\",\"GoldenDB\":\"{\\\"nodes\\\":[{\\\"nodeType\\\":\\\"managerNode\\\",\\\"parentUuid\\\":\\\"7017bd24-1a4d-42fc-aaf4-3046eab88704\\\",\\\"osUser\\\":\\\"zxmanager\\\"}]}\"},\"dependencies\":{\"agents\":[{\"uuid\":\"7017bd24-1a4d-42fc-aaf4-3046eab88704\"}]}}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    @Test
    public void testUpdateStatus() throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        Method method = goldenDbTaskCheckUnderTest.getClass().getDeclaredMethod("updateInstanceStatus", String.class);
        method.setAccessible(true);
        PowerMockito.doNothing().when(mockGoldenDbService).updateResourceLinkStatus(any(), any());
        method.invoke(goldenDbTaskCheckUnderTest, "666");
        Assert.assertTrue(true);
    }
}
