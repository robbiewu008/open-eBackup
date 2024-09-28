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
package openbackup.gaussdbdws.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;

import openbackup.access.framework.resource.service.provider.UnifiedClusterResourceIntegrityChecker;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import com.huawei.oceanprotect.repository.service.LocalStorageService;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.constants.LocalStorageInfoRes;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import com.google.common.collect.ImmutableMap;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * DWS集群环境测试类
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest({EnvironmentLinkStatusHelper.class})
public class GaussDBDWSClusterEnvironmentProviderTest {
    private GaussDBDWSClusterEnvironmentProvider gaussDBDWSClusterEnvironmentProvider;

    private ProviderManager providerManager;

    private ResourceService resourceService;

    private LocalStorageService localStorageService;

    private UnifiedClusterResourceIntegrityChecker clusterIntegrityChecker;

    private AgentUnifiedService agentUnifiedService;

    private GaussDBBaseService gaussDBBaseService;

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = Mockito.mock(
        ResourceConnectionCheckProvider.class);

    private ProtectedResourceChecker protectedResourceChecker;

    private final ResourceConnectionCheckProvider provider = Mockito.mock(ResourceConnectionCheckProvider.class);

    private TaskRepositoryManager taskRepositoryManager;

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @Before
    public void init() throws IllegalAccessException {
        this.protectedResourceChecker = Mockito.mock(ProtectedResourceChecker.class);
        this.providerManager = Mockito.mock(ProviderManager.class);
        PluginConfigManager pluginConfigManager = Mockito.mock(PluginConfigManager.class);
        this.resourceService = Mockito.mock(ResourceService.class);
        this.agentUnifiedService = Mockito.mock(AgentUnifiedService.class);
        this.clusterIntegrityChecker = Mockito.mock(UnifiedClusterResourceIntegrityChecker.class);
        this.taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);
        this.localStorageService = Mockito.mock(LocalStorageService.class);
        this.gaussDBBaseService = new GaussDBBaseService(resourceService, protectedResourceChecker, providerManager,
            resourceConnectionCheckProvider, taskRepositoryManager);
        this.gaussDBDWSClusterEnvironmentProvider = new GaussDBDWSClusterEnvironmentProvider(this.providerManager,
            pluginConfigManager, resourceService, clusterIntegrityChecker, agentUnifiedService);
        MemberModifier.field(GaussDBDWSClusterEnvironmentProvider.class, "localStorageService")
            .set(gaussDBDWSClusterEnvironmentProvider, localStorageService);
        LocalStorageInfoRes localStorageInfoRes = new LocalStorageInfoRes();
        localStorageInfoRes.setEsn("xxxxxxxxxxxxxxxxx");
        PowerMockito.when(localStorageService.getStorageInfo()).thenReturn(localStorageInfoRes);
        MemberModifier.field(GaussDBDWSClusterEnvironmentProvider.class, "gaussDBBaseService")
            .set(gaussDBDWSClusterEnvironmentProvider, gaussDBBaseService);
        PowerMockito.when(providerManager.findProviderOrDefault(any(), any(), any())).thenReturn(provider);
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any())).thenReturn(
            LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 用例场景：GaussDB(DWS)集群环境检查类provider过滤
     * 前置条件：无
     * 检查点：类过滤成功或失败
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(gaussDBDWSClusterEnvironmentProvider.applicable(ResourceSubTypeEnum.GAUSSDB_DWS.getType()));
    }

    /**
     * 用例场景：检查扫描返回的数据库对象是否是我们需要的
     * 前置条件：无
     * 检查点: 检查成功
     */
    @Test
    public void check_scan_success() {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111111");
        Optional<ProtectedResource> resourceOptional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("11111111")).thenReturn(resourceOptional);
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_DATABASES, "database1,database2,database3");
        NodeInfo nodeInfo = new NodeInfo();
        List<NodeInfo> nodeInfos = new ArrayList<>();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DwsConstant.EXTEND_INFO_KEY_STATE, DwsConstant.EXTEND_INFO_NORMAL_VALUE_STATE);
        nodeInfo.setExtendInfo(extendInfo);
        nodeInfos.add(nodeInfo);
        appEnvResponse.setNodes(nodeInfos);
        checkResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(checkResult);
        Assert.assertEquals(3, gaussDBDWSClusterEnvironmentProvider.scan(protectedEnvironment).size());
    }

    /**
     * 用例场景：检查扫描返回的数据库对象 查询不到集群资源
     * 前置条件：无
     * 检查点: 报错
     */
    @Test
    public void check_scan_fail() {
        expectedException.expect(LegoCheckedException.class);
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        gaussDBDWSClusterEnvironmentProvider.scan(protectedEnvironment);
    }

    /**
     * 用例场景：检查扫描返回的数据库对象 查询节点信息异常
     * 前置条件：无
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_nodes_is_not_normal_when_scan() {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111111");
        Optional<ProtectedResource> resourceOptional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("11111111")).thenReturn(resourceOptional);
        CheckResult<AppEnvResponse> checkResult = new CheckResult<>();
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_DATABASES, "database1,database2,database3");
        appEnvResponse.setNodes(new ArrayList<>());
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(new HashMap<>());
        nodeInfo.getExtendInfo()
            .put(DwsConstant.EXTEND_INFO_KEY_STATE, DwsConstant.EXTEND_INFO_UNAVAILABLE_VALUE_STATE);
        appEnvResponse.getNodes().add(nodeInfo);
        checkResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(checkResult);
        Assert.assertThrows("dws cluster qqqqqq not online", LegoCheckedException.class,
            () -> gaussDBDWSClusterEnvironmentProvider.scan(protectedEnvironment));
    }

    private ProtectedEnvironment getProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("555555");
        protectedEnvironment.setName("qqqqqq");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DwsConstant.EXTEND_INFO_KEY_ENV_FILE, "envFile");
        protectedEnvironment.setExtendInfo(extendInfo);
        Map<String, List<ProtectedResource>> resourceMap = new HashMap<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("11111111");
        resources.add(protectedResource);
        resourceMap.put(DwsConstant.DWS_CLUSTER_AGENT, resources);
        protectedEnvironment.setDependencies(resourceMap);
        Authentication authentication = new Authentication();
        authentication.setAuthKey("omm");
        protectedEnvironment.setAuth(authentication);
        return protectedEnvironment;
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：查询祖册dws资源大于八个
     * 检查点: 报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_environment_more_eight_when_check() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        List<ProtectedResource> list = new ArrayList<>();
        for (int i = 0; i < 10; i++) {
            ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
            protectedEnvironment.setUuid("555555");
            list.add(protectedEnvironment);
        }
        pageListResponse.setTotalCount(10);
        pageListResponse.setRecords(list);
        mockDwsCluster(pageListResponse);
        Assert.assertThrows("dws value: 10 check is empty", LegoCheckedException.class,
            () -> gaussDBDWSClusterEnvironmentProvider.register(getProtectedEnvironment()));
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：查询注册dws资源 存在相同的uuid
     * 检查点: 报错
     */
    @Test
    public void check_dws_resource_exist_fail() {
        expectedException.expect(LegoCheckedException.class);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid(UUID.nameUUIDFromBytes(
            (getProtectedEnvironment().getName() + getProtectedEnvironment().getSubType()).getBytes(
                Charset.defaultCharset())).toString());
        list.add(protectedEnvironment);
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);
        mockDwsCluster(pageListResponse);
        gaussDBDWSClusterEnvironmentProvider.register(getProtectedEnvironment());
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：查询注册dws资源主机返回的centralCoordinator 对象不存在
     * 检查点: 失败
     */
    @Test
    public void check_central_coordinator_fail() {
        expectedException.expect(LegoCheckedException.class);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("555555");
        list.add(protectedEnvironment);
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);
        mockDwsCluster(pageListResponse);
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111111");
        Optional<ProtectedResource> resourceOptional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("11111111")).thenReturn(resourceOptional);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(new CheckResult<>());
        gaussDBDWSClusterEnvironmentProvider.register(getProtectedEnvironment());
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：无
     * 检查点: 成功
     */
    @Test
    public void check_dws_environment_success() {
        checkSuccess();
        gaussDBDWSClusterEnvironmentProvider.register(getProtectedEnvironment());
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：版本信息是否在有hostAgent 情况下 为 8.0.0
     * 检查点: 失败
     */
    @Test
    public void check_dws_version_fail() {
        expectedException.expect(LegoCheckedException.class);
        checkSuccess();
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("22222222");
        resources.add(protectedResource);
        protectedEnvironment.getDependencies().put(DwsConstant.HOST_AGENT, resources);
        gaussDBDWSClusterEnvironmentProvider.register(protectedEnvironment);
    }

    private void checkSuccess() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        List<ProtectedResource> list = new ArrayList<>();
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setUuid("555555");
        protectedEnvironment.setName("qqqqqq");
        list.add(protectedEnvironment);
        pageListResponse.setTotalCount(1);
        pageListResponse.setRecords(list);
        mockDwsCluster(pageListResponse);
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid("11111111");
        resource.setLinkStatus("1");
        resource.setEndpoint("8.40.102.115");
        Optional<ProtectedResource> resourceOptional = Optional.of(resource);
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner("11111111")).thenReturn(resourceOptional);
        CheckResult<AppEnvResponse> appEnvResponseCheckResult = new CheckResult<>();
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        appEnvResponse.setSubType(ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        List<NodeInfo> nodes = new ArrayList<>();
        NodeInfo nodeInfo = new NodeInfo();
        nodeInfo.setExtendInfo(new HashMap<>());
        nodeInfo.getExtendInfo().put(DwsConstant.COORDINATOR_IP, "8.40.102.115");
        nodeInfo.getExtendInfo().put(DwsConstant.EXTEND_INFO_KEY_VERSION, DwsConstant.HOST_NO_ADD_VERSION);
        nodes.add(nodeInfo);
        appEnvResponse.setNodes(nodes);
        appEnvResponse.setExtendInfo(new HashMap<>());
        appEnvResponse.getExtendInfo().put("cn_nodes", "8.40.102.115");
        appEnvResponseCheckResult.setData(appEnvResponse);
        PowerMockito.when(clusterIntegrityChecker.generateCheckResult(any())).thenReturn(appEnvResponseCheckResult);
    }

    private void mockDwsCluster(PageListResponse<ProtectedResource> pageListResponse) {
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", ResourceTypeEnum.DATABASE.getType());
        filter.put("subType", ResourceSubTypeEnum.GAUSSDB_DWS.getType());
        PowerMockito.when(resourceService.query(0, LegoNumberConstant.TWENTY, filter)).thenReturn(pageListResponse);
    }

    /**
     * 用例场景：健康检查reportJobByLabelContinue
     * 前置条件：无
     * 检查点: 检查成功,不报错
     */
    @Test
    public void health_check_success() {
        checkSuccess();
        gaussDBDWSClusterEnvironmentProvider.healthCheckWithResultStatus(getProtectedEnvironment());
    }

    /**
     * 用例场景：检查资源查询结果
     * 前置条件：无
     * 检查点: 检查成功,不报错
     */
    @Test
    public void test_browse_protected_environment_success() {
        checkSuccess();
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setPageSize(100);
        conditions.setPageNo(0);
        conditions.setParentId("database1");
        conditions.setEnvId("uuid");
        conditions.setResourceType("DWS-schema");
        PowerMockito.when(agentUnifiedService.getDetailPageListNoRetry(any(), any(), any(), any(), false))
            .thenReturn(MockProviderParameter.getProtectedResourcePageListResponse());
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, LegoNumberConstant.ONE,
            ImmutableMap.of(DatabaseConstants.ROOT_UUID, getProtectedEnvironment().getUuid(), "subType",
                conditions.getResourceType())))
            .thenReturn(MockProviderParameter.getProtectedResourcePageListResponse());
        PowerMockito.when(resourceService.query(LegoNumberConstant.ZERO, DwsConstant.QUERY_QUANTITY_SPECIFICATIONS,
            ImmutableMap.of(DatabaseConstants.ROOT_UUID, getProtectedEnvironment().getUuid(), "subType",
                conditions.getResourceType())))
            .thenReturn(MockProviderParameter.getProtectedResourcePageListResponse());
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus("1");
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner(any())).thenReturn(Optional.of(protectedEnvironment));
        gaussDBDWSClusterEnvironmentProvider.browse(getProtectedEnvironment(), conditions);
    }

    /**
     * 用例场景：检测输入的name为空情况下报错
     * 前置条件：无
     * 检查点: 检查成功,不报错
     */
    @Test
    public void should_throw_LegoCheckedException_if_name_not_exist_when_check() {
        ProtectedEnvironment protectedEnvironment = getProtectedEnvironment();
        protectedEnvironment.setName(null);
        Assert.assertThrows("environment name is empty",LegoCheckedException.class,
            () -> gaussDBDWSClusterEnvironmentProvider.register(protectedEnvironment));
    }
}
