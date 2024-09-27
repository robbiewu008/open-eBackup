/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.Mockito.when;

import openbackup.access.framework.resource.validator.JsonSchemaValidator;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.service.TidbService;

import com.google.common.collect.ImmutableMap;

import org.apache.commons.io.FileUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.test.util.ReflectionTestUtils;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * TidbClusterProviderTest
 *
 * @author w00426202
 * @since 2023-07-15
 */
@RunWith(PowerMockRunner.class)
public class TidbClusterProviderTest {
    @Mock
    private TidbService tidbService;

    @Mock
    private ResourceService resourceService;

    @Mock
    private JsonSchemaValidator jsonSchemaValidator;

    @Mock
    private ProviderManager mockProviderManager;

    @Mock
    private PluginConfigManager mockPluginConfigManager;

    @Mock
    private ResourceService mockResourceService;

    private TidbClusterProvider tidbClusterProvider;

    @Before
    public void setUp() {
        tidbClusterProvider = new TidbClusterProvider(mockProviderManager, mockPluginConfigManager, tidbService,
            resourceService, jsonSchemaValidator);
    }

    /**
     * 用例场景：测试类型判断
     * 前置条件：
     * 检查点：返回false
     */
    @Test
    public void test_applicable() {
        Assert.assertTrue(tidbClusterProvider.applicable(ResourceSubTypeEnum.TIDB_CLUSTER.getType()));
    }

    @Test(expected = LegoCheckedException.class)
    public void test_check() throws IOException {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(TidbConstants.CLUSTER_NAME, "cluster1");
        protectedResource.setExtendInfo(extendInfo1);

        ProtectedEnvironment masterReqFromJsonFile = getMasterReqFromJsonFile();

        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setExtendInfo(ImmutableMap.of(ResourceConstants.AGENT_IP_LIST,
            "[{\"id\":\"8.40.146.172:2379\",\"role\":\"pd\",\"host\":\"8.40.146.172\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.172\",\"hostManagerResourceUuid\":\"12300eb1-2be9-44db-b050-a4af8a85eb95\"},{\"id\":\"8.40.146.173:2379\",\"role\":\"pd\",\"host\":\"8.40.146.173\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.173\",\"hostManagerResourceUuid\":\"a50086f2-0418-42bf-8378-eacd20a66b0c\"},{\"id\":\"8.40.146.179:2379\",\"role\":\"pd\",\"host\":\"8.40.146.179\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.179\",\"hostManagerResourceUuid\":\"ff4dd354-c0cb-4580-8ffb-e27e82cc980c\"},{\"id\":\"8.40.146.170:4000\",\"role\":\"tidb\",\"host\":\"8.40.146.170\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.170\",\"hostManagerResourceUuid\":\"ca297641-c6a1-4578-b400-83e9a71f4d0b\"},{\"id\":\"8.40.146.171:4000\",\"role\":\"tidb\",\"host\":\"8.40.146.171\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.171\",\"hostManagerResourceUuid\":\"4a5bd0db-0197-4411-a765-4da8afe097b7\"},{\"id\":\"8.40.146.175:20160\",\"role\":\"tikv\",\"host\":\"8.40.146.175\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.175\",\"hostManagerResourceUuid\":\"7e7ad259-d00c-4707-a349-851f1cef4188\"},{\"id\":\"8.40.146.176:20160\",\"role\":\"tikv\",\"host\":\"8.40.146.176\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.176\",\"hostManagerResourceUuid\":\"5d6e4be0-b92e-4167-9915-b81e47a4edc1\"},{\"id\":\"8.40.146.178:20160\",\"role\":\"tikv\",\"host\":\"8.40.146.178\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.178\",\"hostManagerResourceUuid\":\"ffd4b69b-420b-447a-9639-0522bf4b7431\"}]"));
        when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource1));

        when(tidbService.getClusterList(any())).thenReturn(protectedResourceList);

        //doNothing().when(tidbService.checkClusterInfo(any(), anyString(), any()));

        tidbClusterProvider.register(masterReqFromJsonFile);

    }

    /**
     * 用例场景：创建集群，传入的agentID不存在
     * 前置条件：无
     * 检查点: 抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void test_check_exception_if_agent_not_exists() throws IOException {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(TidbConstants.CLUSTER_NAME, "cluster1");
        protectedResource.setExtendInfo(extendInfo1);

        ProtectedEnvironment masterReqFromJsonFile = getMasterReqFromJsonFile();

        when(resourceService.getResourceById(any())).thenReturn(Optional.ofNullable(null));

        when(tidbService.getClusterList(any())).thenReturn(protectedResourceList);

        tidbClusterProvider.register(masterReqFromJsonFile);
    }

    /**
     * 用例场景：创建集群，集群节点与agent不匹配
     * 前置条件：无
     * 检查点: 抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void test_check_exception_if_agent_not_match() throws IOException {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(TidbConstants.CLUSTER_NAME, "cluster1");
        protectedResource.setExtendInfo(extendInfo1);

        ProtectedEnvironment masterReqFromJsonFile = getMasterReqFromJsonFile();

        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setExtendInfo(ImmutableMap.of(ResourceConstants.AGENT_IP_LIST, ""));
        when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource1));

        when(tidbService.getClusterList(any())).thenReturn(protectedResourceList);
        tidbClusterProvider.register(masterReqFromJsonFile);
    }

    /**
     * 用例场景：创建集群，传入的集群列表clusterInfoList参数有误
     * 前置条件：无
     * 检查点: 抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void test_check_exception_if_clusterInfoList_error() throws IOException {
        List<ProtectedResource> protectedResourceList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put(TidbConstants.CLUSTER_NAME, "cluster1");
        protectedResource.setExtendInfo(extendInfo1);

        ProtectedEnvironment masterReqFromJsonFile = getMasterReqFromJsonFile();
        masterReqFromJsonFile.getExtendInfo().remove(TidbConstants.CLUSTER_INFO_LIST);

        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setExtendInfo(ImmutableMap.of(ResourceConstants.AGENT_IP_LIST,
            "[{\"id\":\"8.40.146.172:2379\",\"role\":\"pd\",\"host\":\"8.40.146.172\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.172\",\"hostManagerResourceUuid\":\"12300eb1-2be9-44db-b050-a4af8a85eb95\"},{\"id\":\"8.40.146.173:2379\",\"role\":\"pd\",\"host\":\"8.40.146.173\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.173\",\"hostManagerResourceUuid\":\"a50086f2-0418-42bf-8378-eacd20a66b0c\"},{\"id\":\"8.40.146.179:2379\",\"role\":\"pd\",\"host\":\"8.40.146.179\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.179\",\"hostManagerResourceUuid\":\"ff4dd354-c0cb-4580-8ffb-e27e82cc980c\"},{\"id\":\"8.40.146.170:4000\",\"role\":\"tidb\",\"host\":\"8.40.146.170\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.170\",\"hostManagerResourceUuid\":\"ca297641-c6a1-4578-b400-83e9a71f4d0b\"},{\"id\":\"8.40.146.171:4000\",\"role\":\"tidb\",\"host\":\"8.40.146.171\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.171\",\"hostManagerResourceUuid\":\"4a5bd0db-0197-4411-a765-4da8afe097b7\"},{\"id\":\"8.40.146.175:20160\",\"role\":\"tikv\",\"host\":\"8.40.146.175\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.175\",\"hostManagerResourceUuid\":\"7e7ad259-d00c-4707-a349-851f1cef4188\"},{\"id\":\"8.40.146.176:20160\",\"role\":\"tikv\",\"host\":\"8.40.146.176\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.176\",\"hostManagerResourceUuid\":\"5d6e4be0-b92e-4167-9915-b81e47a4edc1\"},{\"id\":\"8.40.146.178:20160\",\"role\":\"tikv\",\"host\":\"8.40.146.178\",\"status\":\"up\",\"parent\":null,\"hostManagerIp\":\"192.168.146.178\",\"hostManagerResourceUuid\":\"ffd4b69b-420b-447a-9639-0522bf4b7431\"}]"));
        when(resourceService.getResourceById(any())).thenReturn(Optional.of(protectedResource1));

        when(tidbService.getClusterList(any())).thenReturn(protectedResourceList);
        tidbClusterProvider.register(masterReqFromJsonFile);
    }

    @Test(expected = NullPointerException.class)
    public void test_getSubResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        when(tidbService.getResourceByCondition(anyMap())).thenReturn(null);
        ReflectionTestUtils.invokeMethod(tidbClusterProvider, "getSubResource", protectedResource);
    }

    @Test(expected = NullPointerException.class)
    public void test_updateResourceStatus() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        List<ProtectedResource> listPro = new ArrayList<>();
        listPro.add(protectedResource);
        PowerMockito.when(tidbClusterProvider, "getSubResource", protectedResource).thenReturn(protectedResource);
        ReflectionTestUtils.invokeMethod(tidbClusterProvider, "getSubResource", protectedResource);
    }

    @Test(expected = NullPointerException.class)
    public void test_checkTableHealth() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        List<ProtectedResource> listPro = new ArrayList<>();
        listPro.add(protectedResource);
        PowerMockito.when(tidbClusterProvider, "getSubResource", protectedResource).thenReturn(protectedResource);

        ProtectedResource clusterResource = new ProtectedResource();
        ProtectedResource agentResource = new ProtectedResource();
        ProtectedResource databaseResource = new ProtectedResource();
        ReflectionTestUtils.invokeMethod(tidbClusterProvider, "checkTableHealth", clusterResource, agentResource,
            databaseResource);
    }

    @Test(expected = NullPointerException.class)
    public void test_checkDbTable() throws Exception {
        ProtectedResource protectedResource = new ProtectedResource();
        List<ProtectedResource> listPro = new ArrayList<>();
        listPro.add(protectedResource);
        PowerMockito.when(tidbClusterProvider, "getSubResource", protectedResource).thenReturn(protectedResource);

        ProtectedResource clusterResource = new ProtectedResource();
        ProtectedResource agentResource = new ProtectedResource();
        ReflectionTestUtils.invokeMethod(tidbClusterProvider, "checkDbTable", clusterResource, agentResource);
    }

    /**
     * getMasterReqFromJsonFile
     *
     * @return ProtectedEnvironment
     * @throws Exception Exception
     */
    public ProtectedEnvironment getMasterReqFromJsonFile() throws IOException {
        URL resource = Thread.currentThread().getContextClassLoader().getResource("Cluster-req.json");
        File file = new File(Objects.requireNonNull(resource).getPath());
        String masterReqStr = FileUtils.readFileToString(file, Charset.defaultCharset());
        ProtectedEnvironment ProtectedEnvironment = JsonUtil.read(masterReqStr, ProtectedEnvironment.class);
        return ProtectedEnvironment;
    }
}
