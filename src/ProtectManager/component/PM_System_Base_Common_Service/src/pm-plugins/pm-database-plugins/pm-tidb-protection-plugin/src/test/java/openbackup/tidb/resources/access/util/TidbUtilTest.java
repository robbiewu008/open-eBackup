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
package openbackup.tidb.resources.access.util;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;

import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * tidb 工具 测试类
 *
 */
public class TidbUtilTest {
    /**
     * 用例场景：创建/更新集群，进行环境参数检验
     * 前置条件：无
     * 检查点：校验成功不报错
     */
    @Test
    public void test_check_tidb_success() {
        TidbUtil.checkTidbReqParam(getEnvironment());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建/更新集群，进行ClusterInfoList参数检验
     * 前置条件：无
     * 检查点：校验成功不报错
     */
    public void test_check_ClusterInfoList_success() {
        ProtectedEnvironment environment = getEnvironment();
        environment.getExtendInfo()
            .put(TidbConstants.CLUSTER_INFO_LIST,
                "[{\"id\":\"8.40.98.242:2379\",\"role\":\"pd\",\"host\":\"8.40.98.242\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.242\",\"hostManagerResourceUuid\":\"0590c88b-63dd-4fbe-9f9f-b67573365eba\"},{\"id\":\"8.40.98.243:2379\",\"role\":\"pd\",\"host\":\"8.40.98.243\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.243\",\"hostManagerResourceUuid\":\"6583e1a2-7822-48b5-8bf5-e4a65ba49ceb\"},{\"id\":\"8.40.98.244:2379\",\"role\":\"pd\",\"host\":\"8.40.98.244\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.244\",\"hostManagerResourceUuid\":\"0bfe900a-804a-4d16-b8a6-f290248ad84b\"},{\"id\":\"8.40.98.240:4000\",\"role\":\"tidb\",\"host\":\"8.40.98.240\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.240\",\"hostManagerResourceUuid\":\"4ea57c6b-99d4-4e06-8e83-2a570b3cb979\"},{\"id\":\"8.40.98.241:4000\",\"role\":\"tidb\",\"host\":\"8.40.98.241\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.241\",\"hostManagerResourceUuid\":\"e82dff58-b8e0-46f3-b947-235fc1540910\"},{\"id\":\"8.40.98.248:9000\",\"role\":\"tiflash\",\"host\":\"8.40.98.248\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.248\",\"hostManagerResourceUuid\":\"256ce5bd-bf90-4a6d-8073-7a9bbb01b9d1\"},{\"id\":\"8.40.98.245:20160\",\"role\":\"tikv\",\"host\":\"8.40.98.245\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.245\",\"hostManagerResourceUuid\":\"262db6f6-4006-4b7d-a11b-b0d13c7214b2\"},{\"id\":\"8.40.98.246:20160\",\"role\":\"tikv\",\"host\":\"8.40.98.246\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.246\",\"hostManagerResourceUuid\":\"388a277d-f6a9-4c5f-bb1a-88a71f869a3c\"},{\"id\":\"8.40.98.247:20160\",\"role\":\"tikv\",\"host\":\"8.40.98.247\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.247\",\"hostManagerResourceUuid\":\"dddf9a21-4ebb-44d3-9fe3-99a70ad3adf6\"}]");
        TidbUtil.checkClusterInfoList(environment);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：clusterInfoList里面的参数有为null的
     * 前置条件：无
     * 检查点：校验失败
     */
    @Test
    public void test_check_tidb_clusterInfoList_error() {
        ProtectedEnvironment environment = getEnvironment();
        environment.getExtendInfo()
            .put(TidbConstants.CLUSTER_INFO_LIST,
                "[{\"id\":\"8.40.98.242:2379\",\"role\":\"pd\",\"host\":\"8.40.98.242\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.242\",\"hostManagerResourceUuid\":\"0590c88b-63dd-4fbe-9f9f-b67573365eba\"},{\"id\":\"8.40.98.243:2379\",\"role\":\"pd\",\"host\":\"8.40.98.243\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.243\",\"hostManagerResourceUuid\":\"6583e1a2-7822-48b5-8bf5-e4a65ba49ceb\"},{\"id\":\"8.40.98.244:2379\",\"role\":\"pd\",\"host\":\"8.40.98.244\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.244\",\"hostManagerResourceUuid\":\"0bfe900a-804a-4d16-b8a6-f290248ad84b\"},{\"id\":\"8.40.98.240:4000\",\"role\":\"tidb\",\"host\":\"8.40.98.240\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.240\",\"hostManagerResourceUuid\":\"4ea57c6b-99d4-4e06-8e83-2a570b3cb979\"},{\"id\":\"8.40.98.241:4000\",\"role\":\"tidb\",\"host\":\"8.40.98.241\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.241\",\"hostManagerResourceUuid\":\"e82dff58-b8e0-46f3-b947-235fc1540910\"},{\"id\":\"8.40.98.248:9000\",\"role\":\"tiflash\",\"host\":\"8.40.98.248\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.248\",\"hostManagerResourceUuid\":\"256ce5bd-bf90-4a6d-8073-7a9bbb01b9d1\"},{\"id\":\"8.40.98.245:20160\",\"role\":\"tikv\",\"host\":\"8.40.98.245\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.245\",\"hostManagerResourceUuid\":\"262db6f6-4006-4b7d-a11b-b0d13c7214b2\"},{\"id\":\"8.40.98.246:20160\",\"role\":\"tikv\",\"host\":\"8.40.98.246\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.246\",\"hostManagerResourceUuid\":\"388a277d-f6a9-4c5f-bb1a-88a71f869a3c\"},{\"id\":\"8.40.98.247:20160\",\"role\":\"tikv\",\"host\":\"8.40.98.247\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.98.247\",\"hostManagerResourceUuid\":null}]");
        Assert.assertThrows(LegoCheckedException.class, () -> TidbUtil.checkClusterInfoList(environment));
    }

    /**
     * 用例场景：clusterInfoList为空
     * 前置条件：无
     * 检查点：校验失败
     */
    @Test
    public void test_check_tidb_clusterInfoList_empty() {
        ProtectedEnvironment environment = getEnvironment();
        environment.getExtendInfo().put(TidbConstants.CLUSTER_INFO_LIST, "");
        Assert.assertThrows(LegoCheckedException.class, () -> TidbUtil.checkClusterInfoList(environment));
    }

    private ProtectedEnvironment getEnvironment() {
        String json
            = "{\"uuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"name\":\"testzyl\",\"type\":\"Database\",\"subType\":\"TDSQL-cluster\",\"createdTime\":\"2023-05-31 20:10:22.084\",\"rootUuid\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"sourceType\":\"register\",\"protectionStatus\":0,\"extendInfo\":{\"linkStatus\":\"1\",\"clusterInfo\":\"{\\\"ossNodes\\\":[{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\",\\\"port\\\":\\\"8080\\\"},{\\\"nodeType\\\":\\\"ossNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\",\\\"port\\\":\\\"8080\\\"}],\\\"schedulerNodes\\\":[{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\\\",\\\"ip\\\":\\\"8.40.147.38\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\\\",\\\"ip\\\":\\\"8.40.147.39\\\"},{\\\"nodeType\\\":\\\"schedulerNode\\\",\\\"parentUuid\\\":\\\"16f74c9f-915c-4af6-91f6-40c643f13fd5\\\",\\\"ip\\\":\\\"8.40.147.40\\\"}]}\"},\"endpoint\":\"192.168.147.38,192.168.147.39,192.168.147.40,192.168.147.38,192.168.147.39,192.168.147.40\",\"port\":0,\"auth\":{\"authType\":2,\"authKey\":\"DES\",\"authPwd\":\"DES\",\"extendInfo\":{}},\"dependencies\":{\"agents\":[{\"uuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"name\":\"tdsql-h63\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"16f74c9f-915c-4af6-91f6-40c643f13fd5\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.40,8.40.147.40,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_96e73f8ccba74641bc75d44c16b7d97e\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_5ce5b61e6fed4c618a6131ad28ef2e48\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.40\",\"port\":59531,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"name\":\"host-8-40-147-32\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"7ed7f76c-7ad2-4cc5-af1b-2ec17dc6b6a7\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.38,8.40.147.38,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_8503624548bf45d68368c26cf12027dc\":\"0fc6cb490c73476bb90aa69e40f3c931\",\"$citations_agents_435d0f7267f14c40879d86094149ed51\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\"},\"endpoint\":\"192.168.147.38\",\"port\":59530,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false},{\"uuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"name\":\"host-8-40-147-33\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"createdTime\":\"2023-05-16 10:11:11.0\",\"rootUuid\":\"c1099cbe-9fa2-42be-95a9-b629c29d8b48\",\"version\":\"1.5.RC1.007\",\"protectionStatus\":0,\"extendInfo\":{\"agentIpList\":\"192.168.147.39,8.40.147.39,fe80::9da9:f138:d9c4:719c,fe80::67fd:320b:becb:54dd,fe80::34ad:dc0b:f1c6:96bf,fe80::d5a4:d578:83c7:3ea2,fe80::3a07:c531:2a9b:d4d8,fe80::fd61:f1f9:bbe0:2647\",\"$citations_agents_dea1dc850efa4568bc13d815ba0be3d7\":\"96590445-0df7-31f4-806b-9fb9e4ed548d\",\"scenario\":\"0\",\"src_deduption\":\"true\",\"$citations_agents_156c0a940f72494bbfb802a7db1e34d2\":\"0fc6cb490c73476bb90aa69e40f3c931\"},\"endpoint\":\"192.168.147.39\",\"port\":59522,\"linkStatus\":\"1\",\"username\":\"\",\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}]},\"linkStatus\":\"0\",\"scanInterval\":3600,\"cluster\":false}";
        ProtectedEnvironment read = JsonUtil.read(json, ProtectedEnvironment.class);
        return read;
    }

    /**
     * 资源类型特定值转换。
     */
    @Test
    public void test_wrapExtendInfo2Add() {
        ProtectedResource src = new ProtectedResource();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(TidbConstants.TIUP_PATH, "/temp/tidb");
        src.setExtendInfo(extendInfo);

        ProtectedResource target = new ProtectedResource();
        Map<String, String> targetExtendInfo = new HashMap<>();
        target.setExtendInfo(targetExtendInfo);
        TidbUtil.wrapExtendInfo2Add(target, src);
        Assert.assertEquals("/temp/tidb", target.getExtendInfo().get(TidbConstants.TIUP_PATH));
    }

    /**
     * 用例场景：查找可用的tiup节点的uuid，集群信息不正确
     * 前置条件：无
     * 检查点：校验失败
     */
    @Test
    public void test_setTiupUuid_fail_if_cluster_id_is_wrong() {
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        DefaultProtectAgentSelector defaultSelector = Mockito.mock(DefaultProtectAgentSelector.class);
        TidbService tidbService = Mockito.mock(TidbService.class);
        Optional<ProtectedResource> resourceOptional = Optional.empty();
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(resourceOptional);
        Assert.assertThrows(LegoCheckedException.class,
            () -> TidbUtil.setTiupUuid(new HashMap<>(), "clusterUuid", resourceService, defaultSelector, tidbService));
    }

    /**
     * 用例场景：查找可用的tiup节点的uuid，找到可用的，校验正确性
     * 前置条件：无
     * 检查点：校验成功
     */
    @Test
    public void test_setTiupUuid_success() {
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        DefaultProtectAgentSelector defaultSelector = Mockito.mock(DefaultProtectAgentSelector.class);
        TidbService tidbService = Mockito.mock(TidbService.class);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("55555");
        protectedEnvironment.setName("qqqqqq");
        protectedEnvironment.setVersion("3.2.0");
        protectedEnvironment.setExtendInfoByKey("version", "3.2.0");
        protectedEnvironment.setExtendInfoByKey("status", "status");
        protectedEnvironment.setExtendInfoByKey("projectId", "projectId");
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(DatabaseConstants.AGENTS, resources);
        protectedEnvironment.setDependencies(resourceMap);
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedEnvironment));
        String id1 = "b88e84f0-a7d0-472d-a336-7898436ffe43";
        String id2 = "b88e84f0-a7d0-472d-a336-7898436ffe45";
        List<Endpoint> endpointList = Arrays.asList(new Endpoint(id1, "192.169.1.13", 9099),
            new Endpoint(id2, "192.169.1.12", 9099));
        PowerMockito.when(defaultSelector.selectByAgentParameter(any(), any())).thenReturn(endpointList);
        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(new ProtectedResource());
        PowerMockito.doNothing().when(tidbService).checkHealth(any(), any(), anyString(), anyString());
        Map<String, String> protectObjectExtendInfo = new HashMap<>();
        TidbUtil.setTiupUuid(protectObjectExtendInfo, "clusterUuid", resourceService, defaultSelector, tidbService);
        Assert.assertTrue(
            StringUtils.equals(MapUtils.getString(protectObjectExtendInfo, TidbConstants.TIUP_UUID), id1) || StringUtils
                .equals(MapUtils.getString(protectObjectExtendInfo, TidbConstants.TIUP_UUID), id2));
    }

    /**
     * 用例场景：查找可用的tiup节点的uuid，找不到可用的
     * 前置条件：无
     * 检查点：校验失败
     */
    @Test
    public void test_setTiupUuid_fail_if_agent_is_offline() {
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        DefaultProtectAgentSelector defaultSelector = Mockito.mock(DefaultProtectAgentSelector.class);
        TidbService tidbService = Mockito.mock(TidbService.class);
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("55555");
        protectedEnvironment.setName("qqqqqq");
        protectedEnvironment.setVersion("3.2.0");
        protectedEnvironment.setExtendInfoByKey("version", "3.2.0");
        protectedEnvironment.setExtendInfoByKey("status", "status");
        protectedEnvironment.setExtendInfoByKey("projectId", "projectId");
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("22222");
        resources.add(protectedResource);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        resourceMap.put(DatabaseConstants.AGENTS, resources);
        protectedEnvironment.setDependencies(resourceMap);
        List<ProtectedResource> list = new ArrayList<>();
        list.add(protectedEnvironment);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);
        PowerMockito.when(resourceService.getResourceById(anyString())).thenReturn(Optional.of(protectedEnvironment));
        String id1 = "b88e84f0-a7d0-472d-a336-7898436ffe43";
        String id2 = "b88e84f0-a7d0-472d-a336-7898436ffe45";
        List<Endpoint> endpointList = Arrays.asList(new Endpoint(id1, "192.169.1.13", 9099),
            new Endpoint(id2, "192.169.1.12", 9099));
        PowerMockito.when(defaultSelector.selectByAgentParameter(any(), any())).thenReturn(endpointList);
        PowerMockito.when(tidbService.getResourceByCondition(anyString())).thenReturn(new ProtectedResource());
        PowerMockito.doThrow(new LegoCheckedException(""))
            .when(tidbService)
            .checkHealth(any(), any(), anyString(), anyString());
        Assert.assertThrows(LegoCheckedException.class,
            () -> TidbUtil.setTiupUuid(new HashMap<>(), "clusterUuid", resourceService, defaultSelector, tidbService));
    }
}
