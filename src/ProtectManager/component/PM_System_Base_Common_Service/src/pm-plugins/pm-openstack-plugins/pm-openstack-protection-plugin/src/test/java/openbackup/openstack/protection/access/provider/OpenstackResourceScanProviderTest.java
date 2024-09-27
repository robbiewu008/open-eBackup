/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.constant.OpenstackDomainVisibleEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述: test OpenstackResourceScanProvider
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-26
 */
public class OpenstackResourceScanProviderTest {
    private static OpenstackResourceScanProvider openstackResourceScanProvider;
    private static final AgentUnifiedService agentService = PowerMockito.mock(AgentUnifiedService.class);
    private static final ProtectedEnvironmentRetrievalsService envRetrievalsService =
        PowerMockito.mock(ProtectedEnvironmentRetrievalsService.class);
    private static final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private static final EnvironmentCheckProvider environmentCheckProvider =
        PowerMockito.mock(EnvironmentCheckProvider.class);
    private static final ResourceConnectionCheckProvider resourceConnectionCheckProvider =
        PowerMockito.mock(ResourceConnectionCheckProvider.class);

    @BeforeClass
    public static void init() {
        openstackResourceScanProvider = new OpenstackResourceScanProvider(agentService, resourceService,
            envRetrievalsService, environmentCheckProvider, resourceConnectionCheckProvider);
    }

    /**
     * 用例场景：OpenStack扫描环境成功 <br/>
     * 前置条件：OpenStack扫描参数正确 <br/>
     * 检查点：无异常信息，扫描结果非空
     */
    @Test
    public void test_scan_success() {
        ProtectedEnvironment environment = MockFactory.mockEnvironment();
        environment.setUserId("user_01");
        String serviceId = "service_id01";
        environment.getExtendInfo().put(OpenstackConstant.SERVICE_ID_KEY, serviceId);
        ProtectedEnvironment agent = MockFactory.mockAgentResource();
        Map<ProtectedResource, List<ProtectedEnvironment>> agentMap = new HashMap<>();
        agentMap.put(null, Collections.singletonList(agent));
        PowerMockito.when(envRetrievalsService.collectConnectableResources(anyString())).thenReturn(agentMap);
        PowerMockito.when(resourceService.getResourceById(agent.getUuid())).thenReturn(Optional.of(agent));
        PageListResponse<ProtectedResource> domainResource = mockDomainResource();
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(domainResource);
        PageListResponse<ProtectedResource> projectResponse = mockScanProjectResponse();
        PageListResponse<ProtectedResource> hostResponse = mockHostResponse();
        PageListResponse<ProtectedResource> hostDiskResponse = mockHostDiskResponse();
        domainResource.getRecords().get(0).setUuid("default");
        PowerMockito.when(agentService.getDetailPageList(any(), any(), any(), any()))
            .thenReturn(domainResource)
            .thenReturn(projectResponse)
            .thenReturn(hostResponse)
            .thenReturn(hostDiskResponse)
            .thenReturn(mockEmptyResult());

        List<ProtectedResource> scanResources = openstackResourceScanProvider.scan(environment);
        Assert.assertNotNull(scanResources);

        ProtectedResource domainRes = scanResources.stream()
            .filter(project -> ResourceSubTypeEnum.OPENSTACK_DOMAIN.equalsSubType(project.getSubType()))
            .findFirst()
            .orElse(new ProtectedResource());
        Assert.assertEquals(serviceId, domainRes.getUuid());
        Assert.assertEquals(1, Long.parseLong(domainRes.getExtendInfo()
            .get(OpenstackConstant.PROJECT_COUNT)));

        ProtectedResource projectRes = scanResources.stream()
            .filter(project -> ResourceSubTypeEnum.OPENSTACK_PROJECT.equalsSubType(project.getSubType()))
            .findFirst()
            .orElse(new ProtectedResource());
        Assert.assertEquals(1, Long.parseLong(projectRes.getExtendInfo()
            .get(OpenstackConstant.CLOUD_SERVER_COUNT)));
        Assert.assertEquals(OpenstackConstant.DEFAULT_DOMAIN_ID, projectRes.getExtendInfo()
            .get(OpenstackConstant.DOMAIN_ID_KEY));
        Assert.assertEquals(serviceId, projectRes.getParentUuid());

        ProtectedResource hostRes = scanResources.stream()
            .filter(host -> ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(host.getSubType()))
            .findFirst()
            .orElse(new ProtectedResource());
        Assert.assertNotNull(hostRes.getExtendInfo().get(OpenstackConstant.VOLUME_INFO_KEY));
    }

    /**
     * 用例场景：OpenStack扫描插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        boolean isOpenstack = openstackResourceScanProvider.applicable(ResourceSubTypeEnum.OPENSTACK_CONTAINER.getType());
        Assert.assertTrue(isOpenstack);
    }

    /**
     * 用例场景：OpenStack环境检查处理正确 <br/>
     * 前置条件：无 <br/>
     * 检查点：调用environmentCheckProvider的check方法
     */
    @Test
    public void test_check_success() {
        openstackResourceScanProvider.register(MockFactory.mockEnvironment());
        Mockito.verify(environmentCheckProvider, Mockito.times(1)).check(any());
    }


    /**
     * 用例场景：OpenStack环境健康检查处理正确 <br/>
     * 前置条件：无 <br/>
     * 检查点：无异常
     */
    @Test
    public void test_healthCheck_success() {
        openstackResourceScanProvider.validate(MockFactory.mockEnvironment());
        Mockito.verify(resourceConnectionCheckProvider, Mockito.times(1)).checkConnection(any());
    }

    private PageListResponse<ProtectedResource> mockEmptyResult() {
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(new ArrayList<>());
        pageListResponse.setTotalCount(0);
        return pageListResponse;
    }

    private PageListResponse<ProtectedResource> mockDomainResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("service_id01");
        protectedResource.setSubType(ResourceSubTypeEnum.OPENSTACK_DOMAIN.getType());
        protectedResource.setPath("envPath" + File.separator + "domain");
        Authentication authentication = new Authentication();
        authentication.setAuthKey("domain_auth_key");
        authentication.setAuthPwd("domain_auth_pwd");
        protectedResource.setAuth(authentication);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenstackConstant.VISIBLE, OpenstackDomainVisibleEnum.VISIBLE.getCode());
        protectedResource.setExtendInfo(extendInfo);
        PageListResponse<ProtectedResource> domainResResponse = new PageListResponse<>();
        domainResResponse.setTotalCount(1);
        domainResResponse.setRecords(Collections.singletonList(protectedResource));
        return domainResResponse;
    }

    private PageListResponse<ProtectedResource> mockScanProjectResponse() {
        List<ProtectedResource> projectResources = new ArrayList<>();
        ProtectedResource projectRes = new ProtectedResource();
        projectRes.setUuid("d03b7f2f-85ea-397f-8369-9e332fc0cec9");
        projectRes.setParentUuid("default");
        projectRes.setSubType(ResourceSubTypeEnum.OPENSTACK_PROJECT.getType());
        Map<String, String> extend = new HashMap<>();
        projectRes.setExtendInfo(extend);
        projectResources.add(projectRes);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(projectResources);
        pageListResponse.setTotalCount(projectResources.size());
        return pageListResponse;
    }

    private PageListResponse<ProtectedResource> mockHostResponse() {
        List<ProtectedResource> cloudHostResources = new ArrayList<>();
        ProtectedResource cloudHostRes = new ProtectedResource();
        cloudHostRes.setUuid("cloud_uuid_00");
        cloudHostRes.setParentUuid("d03b7f2f-85ea-397f-8369-9e332fc0cec9");
        cloudHostRes.setSubType(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        Map<String, String> extendInfo = new HashMap<>();
        cloudHostRes.setExtendInfo(extendInfo);
        cloudHostResources.add(cloudHostRes);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(cloudHostResources);
        pageListResponse.setTotalCount(cloudHostResources.size());
        return pageListResponse;
    }

    private PageListResponse<ProtectedResource> mockHostDiskResponse() {
        List<ProtectedResource> cloudHostResources = new ArrayList<>();
        ProtectedResource cloudHostDiskRes = new ProtectedResource();
        cloudHostDiskRes.setUuid("cloud_disk_uuid_00");
        cloudHostDiskRes.setParentUuid("cloud_uuid_00");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("id", "disk_id0");
        extendInfo.put("name", "disk_name0");
        extendInfo.put("shareable", "true");
        extendInfo.put("fullClone", "0");
        cloudHostDiskRes.setExtendInfo(extendInfo);

        ProtectedResource cloudHostDiskRes1 = new ProtectedResource();
        cloudHostDiskRes1.setUuid("cloud_disk_uuid_01");
        cloudHostDiskRes1.setParentUuid("cloud_uuid_00");
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put("id", "disk_id1");
        extendInfo1.put("name", "disk_name1");
        extendInfo1.put("shareable", "false");
        extendInfo1.put("fullClone", "");
        cloudHostDiskRes1.setExtendInfo(extendInfo1);

        cloudHostResources.add(cloudHostDiskRes);
        cloudHostResources.add(cloudHostDiskRes1);
        PageListResponse<ProtectedResource> pageListResponse = new PageListResponse<>();
        pageListResponse.setRecords(cloudHostResources);
        pageListResponse.setTotalCount(cloudHostResources.size());
        return pageListResponse;
    }
}