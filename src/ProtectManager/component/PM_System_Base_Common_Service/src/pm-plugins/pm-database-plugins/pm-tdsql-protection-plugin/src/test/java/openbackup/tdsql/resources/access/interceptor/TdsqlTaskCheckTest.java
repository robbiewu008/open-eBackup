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
package openbackup.tdsql.resources.access.interceptor;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.Mockito.when;

import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.provider.TdsqlClusterProvider;
import openbackup.tdsql.resources.access.service.TdsqlService;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.junit.MockitoJUnitRunner;
import org.powermock.reflect.Whitebox;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-07-04
 */
@RunWith(MockitoJUnitRunner.class)
public class TdsqlTaskCheckTest {
    @Mock
    private TdsqlService mockTdsqlService;
    @Mock
    private TdsqlClusterProvider mockTdsqlClusterProvider;
    private TdsqlTaskCheck tdsqlTaskCheckUnderTest;
    @Before
    public void setUp() {
        tdsqlTaskCheckUnderTest = new TdsqlTaskCheck(mockTdsqlService, mockTdsqlClusterProvider);
    }

    /**
     * 用例场景：校验生产环境的变化结果
     * 前置条件：实例已注册,获取环境信息成功，获取资源信息成功
     * 检查点：生产环境无变化
     */
    @Test
    public void test_check_env_change_should_return_instance_id_set_1685328362_6() {
        when(mockTdsqlService.getResourceById(any())).thenReturn(getInstanceEnvironment());
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getClusterEnvironment());
        final PageListResponse<ProtectedResource> pageListResponse =
            new PageListResponse<>(0, Arrays.asList(getInstanceEnvironment()));
        when(mockTdsqlClusterProvider.browse(any(),any())).thenReturn(pageListResponse);

        final DataNode dataNode = getDataNode();
        final List<DataNode> dataNodeList = Arrays.asList(dataNode);
        when(mockTdsqlService.getInstanceDataNodes(any())).thenReturn(dataNodeList);

        // Run the test
        TdsqlInstance result = tdsqlTaskCheckUnderTest.checkEnvChange("clusterId", "instanceId");

        // Verify the results
        Assert.assertEquals(result.getId(), "set_1685328362_6");
    }

    /**
     * 用例场景：校验生产环境是否
     * 前置条件：集群实例为空
     * 检查点：抛出异常
     */
    @Test
    public void test_check_env_change_should_throw_legocheckedexception() {
        when(mockTdsqlService.getResourceById(any())).thenReturn(getInstanceEnvironment());
        when(mockTdsqlService.getEnvironmentById(any())).thenReturn(getClusterEnvironment());
        when(mockTdsqlClusterProvider.browse(any(),any())).thenReturn(null);
        final DataNode dataNode = getDataNode();
        final List<DataNode> dataNodeList = Arrays.asList(dataNode);

        // Verify the results
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(tdsqlTaskCheckUnderTest.checkEnvChange("clusterId", "instanceId")));
    }

    private ProtectedEnvironment getInstanceEnvironment() {
        String json ="{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"test1\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"parentName\":\"\",\"parentUuid\":\"\",\"extendInfo\":{\"clusterInstanceInfo\":\"{\\\"groups\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"dataNodes\\\": [{\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.38\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 1, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.39\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}, {\\\"setId\\\": \\\"set_1685328362_6\\\", \\\"ip\\\": \\\"8.40.147.40\\\", \\\"port\\\": \\\"4002\\\", \\\"isMaster\\\": 0, \\\"defaultsFile\\\": \\\"/data/tdsql_run/4002/mysql-server-8.0.24/etc/my_4002.cnf\\\", \\\"socket\\\": \\\"/data1/tdengine/data/4002/prod/mysql.sock\\\"}]}], \\\"id\\\": \\\"set_1685328362_6\\\", \\\"type\\\": \\\"0\\\"}\"}}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private ProtectedEnvironment getClusterEnvironment() {
        String json = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        return JsonUtil.read(json, ProtectedEnvironment.class);
    }

    private DataNode getDataNode() {
        DataNode dataNode = new DataNode();
        dataNode.setNodeType("nodeType");
        dataNode.setLinkStatus("linkStatus");
        dataNode.setIp("ip");
        dataNode.setPort("port");
        dataNode.setParentUuid("7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7");
        dataNode.setPriority("priority");
        dataNode.setSocket("socket");
        dataNode.setDefaultsFile("defaultsFile");
        dataNode.setIsMaster("isMaster");
        return dataNode;
    }
}
