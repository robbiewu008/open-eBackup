/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package openbackup.openstack.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.openstack.protection.access.common.OpenstackCommonService;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.provider.OpenstackDomainResourceProvider;
import openbackup.openstack.protection.access.provider.OpenstackResourceScanProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;

/**
 * 功能描述: test OpenstackDomainResourceProvider
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-03-19
 */
public class OpenstackDomainResourceProviderTest {
    private static OpenstackDomainResourceProvider openstackDomainResourceProvider;
    private static final ResourceService resourceService = PowerMockito.mock(ResourceService.class);
    private static final OpenstackResourceScanProvider openstackResourceScanProvider =
        PowerMockito.mock(OpenstackResourceScanProvider.class);
    private static final OpenstackCommonService openstackQuotaService =
            PowerMockito.mock(OpenstackCommonService.class);

    @BeforeClass
    public static void init() {
        openstackDomainResourceProvider =
            new OpenstackDomainResourceProvider(resourceService, openstackResourceScanProvider, openstackQuotaService);
    }

    /**
     * 用例名称：验证Domain扫描成功。<br/>
     * 前置条件：参数正确。<br/>
     * check点：无异常，返回扫描结果。<br/>
     */
    @Test
    public void test_scan_success() {
        ProtectedResource projectResource = mockDomainResource();
        List<ProtectedResource> scannedRes = new ArrayList<>();
        ProtectedResource scannedResource = mockDomainResource();
        scannedResource.setSubType(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
        scannedRes.add(scannedResource);
        Mockito.when(openstackResourceScanProvider.scanByAgent(any(), any())).thenReturn(scannedRes);
        List<ProtectedResource> resources = openstackDomainResourceProvider.scan(projectResource);
        Assert.assertEquals(2, resources.size());
    }

    /**
     * 用例名称：验证更新Domain成功。<br/>
     * 前置条件：参数正确。<br/>
     * check点：成功修改参数
     */
    @Test
    public void test_before_update_success() {
        ProtectedResource domainResource = mockDomainResource();
        Mockito.when(resourceService.getResourceById(anyBoolean(), anyString()))
            .thenReturn(Optional.of(mockDomainResource()));
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        openstackDomainResourceProvider.beforeUpdate(domainResource);
        Assert.assertEquals(ResourceConstants.SOURCE_TYPE_REGISTER, domainResource.getSourceType());
    }

    /**
     * 用例场景：OpenStack项目资源插件类型判断正确 <br/>
     * 前置条件：流程正常 <br/>
     * 检查点：返回结果为True
     */
    @Test
    public void test_applicable_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.OPENSTACK_DOMAIN.getType());
        openstackDomainResourceProvider.beforeCreate(resource);
        boolean isOpenstack = openstackDomainResourceProvider.applicable(resource);
        Assert.assertTrue(isOpenstack);
    }

    private ProtectedResource mockDomainResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("domain_mock_uuid");
        protectedResource.setSubType(ResourceSubTypeEnum.OPENSTACK_DOMAIN.getType());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenstackConstant.VISIBLE, "1");
        protectedResource.setExtendInfo(extendInfo);
        Authentication auth = new Authentication();
        auth.setExtendInfo(extendInfo);
        protectedResource.setAuth(auth);
        return protectedResource;
    }

}