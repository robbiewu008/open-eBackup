/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tdsql.resources.access.provider;

import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.argThat;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatcher;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-04
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlInstanceConnectionCheckerTest {
    private static final String ONLINE = LinkStatusEnum.ONLINE.getStatus().toString();

    private static final String OFFLINE = LinkStatusEnum.OFFLINE.getStatus().toString();

    @Mock
    private ProtectedEnvironmentRetrievalsService mockEnvironmentRetrievalsService;

    @Mock
    private AgentUnifiedService mockAgentUnifiedService;

    @Mock
    private TdsqlService mockTdsqlService;

    @Mock
    private ResourceService resourceService;

    @Mock
    private ProviderManager providerManager;

    @Mock
    private UnifiedConnectionCheckProvider unifiedConnectionCheckProvider;

    private TdsqlInstanceConnectionChecker tdsqlInstanceConnectionCheckerUnderTest;

    @Before
    public void setUp() {
        tdsqlInstanceConnectionCheckerUnderTest = new TdsqlInstanceConnectionChecker(mockEnvironmentRetrievalsService,
            mockAgentUnifiedService, mockTdsqlService, resourceService, providerManager);
    }

    /**
     * 用例场景：策略模式策略识别-TDSQL
     * 前置条件：类型参数为TDSQL-clusterInstance
     * 检查点：识别成功
     */
    @Test
    public void test_applicable_success() {
        // Setup
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());

        // Run the test
        boolean result = tdsqlInstanceConnectionCheckerUnderTest.applicable(resource);

        // Verify the results
        assertTrue(result);
    }

    /**
     * 用例场景：获取实例节点及其对应的主机
     * 前置条件：实例已注册成功，获取环境信息成功
     * 检查点：获取实例节点及其对应的主机成功
     */
    @Test
    public void test_collect_collectable_resources_should_have_3_nodes() {
        String clusterInstanceInfo = getEnvironment().getExtendInfo().get("clusterInstanceInfo");
        TdsqlInstance instance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        LinkedList<DataNode> dataNodes = new LinkedList<>();
        instance.getGroups().forEach(group -> {
            dataNodes.addAll(group.getDataNodes());
        });
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodes);
        when(providerManager.findProvider(any(), any())).thenReturn(unifiedConnectionCheckProvider);
        when(unifiedConnectionCheckProvider.tryCheckConnection(any())).thenReturn(new ResourceCheckContext());
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(new ProtectedEnvironment());

        // Run the test
        final Map<ProtectedResource, List<ProtectedEnvironment>> result
            = tdsqlInstanceConnectionCheckerUnderTest.collectConnectableResources(getEnvironment());
        Assert.assertEquals(result.size(), 3);
    }

    /**
     * 用例场景：获取连通性获取检查结果
     * 前置条件：实例已注册
     * 检查点：获取连通性获取检查结果成功
     */
    @Test
    public void test_collect_action_results() {
        ActionResult commonResult = new ActionResult();
        commonResult.setCode(0);
        commonResult.setMessage("success");
        ActionResult pluginResult = new ActionResult();
        pluginResult.setCode(TdsqlConstant.NODE_TYPE_MISMATCH);
        pluginResult.setMessage(
            "{\"errorCode\": 1577209938, \"parameters\": [\"test2,dataNode,96590445-0df7-31f4-806b-9fb9e4ed548d\"], \"errorMessage\": null}");

        ProtectedEnvironment environment1 = new ProtectedEnvironment();
        environment1.setUuid("environment_uuid1");
        ProtectedResource resource1 = new ProtectedResource();
        resource1.setUuid("resource_uuid1");
        resource1.setEnvironment(environment1);

        ProtectedEnvironment environment2 = new ProtectedEnvironment();
        environment2.setUuid("environment_uuid2");
        ProtectedResource resource2 = new ProtectedResource();
        resource2.setUuid("resource_uuid1");
        resource2.setEnvironment(environment2);

        CheckResult<Object> checkResult_1 = new CheckResult<>();
        checkResult_1.setResults(commonResult);
        CheckResult<Object> checkResult_2 = new CheckResult<>();
        checkResult_2.setResults(pluginResult);

        List<CheckResult<Object>> list1 = new ArrayList<>();
        list1.add(checkResult_1);

        List<CheckResult<Object>> list2 = new ArrayList<>();
        list2.add(checkResult_2);

        CheckReport<Object> objectCheckReport1 = new CheckReport<>();
        objectCheckReport1.setResults(list1);
        objectCheckReport1.setResource(resource1);

        CheckReport<Object> objectCheckReport2 = new CheckReport<>();
        objectCheckReport2.setResults(list2);
        objectCheckReport2.setResource(resource2);

        List<CheckReport<Object>> checkReport = new ArrayList<>();
        checkReport.add(objectCheckReport1);
        checkReport.add(objectCheckReport2);

        ProtectedResource resource3 = new ProtectedResource();
        resource3.setUuid("resource_uuid1");
        String instanceInfoStr
            = "{\"id\":\"set_1690526294_5\",\"name\":\"set_1690526294_5\",\"type\":\"0\",\"cluster\":\"a851a36d-1c79-304b-8177-00dea9a2cdee\",\"groups\":[{\"setId\":\"set_1690526294_5\",\"dataNodes\":[{\"ip\":\"8.40.168.191\",\"port\":\"4003\",\"defaultsFile\":\"/data/tdsql_run/4003/mysql-server-8.0.24/etc/my_4003.cnf\",\"socket\":\"/data/4003/prod/mysql.sock\",\"isMaster\":\"1\",\"priority\":\"2\",\"nodeType\":\"dataNode\",\"parentUuid\":\"environment_uuid1\",\"linkStatus\":\"1\"},{\"ip\":\"8.40.168.190\",\"port\":\"4003\",\"defaultsFile\":\"/data/tdsql_run/4003/mysql-server-8.0.24/etc/my_4003.cnf\",\"socket\":\"/data/4003/prod/mysql.sock\",\"isMaster\":\"0\",\"priority\":\"2\",\"nodeType\":\"dataNode\",\"parentUuid\":\"environment_uuid2\",\"linkStatus\":\"1\"}]}]}";
        resource3.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, instanceInfoStr);
        PowerMockito.when(mockTdsqlService.getResourceById(any())).thenReturn(resource3);

        List<ActionResult> results = tdsqlInstanceConnectionCheckerUnderTest.collectActionResults(checkReport,
            new HashMap<>());
        Assert.assertEquals(results.size(), 2);
        Mockito.verify(resourceService).updateSourceDirectly(argThat(checkLinkStatus()));
    }

    private ArgumentMatcher<List<ProtectedResource>> checkLinkStatus() {
        return new ArgumentMatcher<List<ProtectedResource>>() {
            @Override
            public boolean matches(List<ProtectedResource> argument) {
                System.out.println(JsonUtil.json(argument));
                ProtectedResource resource = argument.get(0);

                if (!resource.getUuid().equals("resource_uuid1")) {
                    return false;
                }

                if (!OFFLINE.equals(resource.getExtendInfoByKey(TdsqlConstant.LINKSTATUS))) {
                    return false;
                }

                TdsqlInstance instance = JsonUtil.read(resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO),
                    TdsqlInstance.class);

                List<DataNode> list = instance.getGroups().get(0).getDataNodes();

                if (list.size() != 2) {
                    return false;
                }

                if (!ONLINE.equals(list.get(0).getLinkStatus())) {
                    return false;
                }

                return OFFLINE.equals(list.get(1).getLinkStatus());
            }
        };
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"test1\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"parentName\":\"\",\"parentUuid\":\"\",\"extendInfo\":{\"clusterInstanceInfo\":\"{\\\"groups\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"dataNodes\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.38\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 1, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.39\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.40\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}]}], \\\"id\\\": \\\"set_1685328362_6\\\", \\\"type\\\": \\\"0\\\"}\"}}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }
}
