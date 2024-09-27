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
package openbackup.access.framework.resource.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyString;

import openbackup.access.framework.resource.persistence.model.ProtectedResourceExtendInfoPo;
import openbackup.access.framework.resource.persistence.model.ProtectedResourcePo;
import openbackup.access.framework.resource.service.CyberEngineResourceServiceImpl;
import openbackup.access.framework.resource.service.ProtectedResourceRepository;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.FileSystemInfo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.StorageInfo;
import openbackup.data.protection.access.provider.sdk.resource.TenantInfo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.util.MessageTemplate;

import lombok.NoArgsConstructor;
import org.assertj.core.util.Lists;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.internal.WhiteboxImpl;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * CyberEngineResourceServiceImpl的测试类
 *
 * @author s30031954
 * @since 2022-12-25
 */
@RunWith(PowerMockRunner.class)
@NoArgsConstructor
public class CyberEngineResourceServiceImplTest {
    /**
     * 用例场景：获取所有租户信息
     * 前置条件：设备可查到租户
     * 检查点：查询到租户数量不为0
     */
    @Test
    public void listAllTenantSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        BasePage<ProtectedResourcePo> target = buildProtectedResourcePoBasePage();
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        PageListResponse<TenantInfo> tenantInfoPageListResponse = cyberEngineResourceService.listAllTenants(1, 1);
        Assert.assertEquals(1, tenantInfoPageListResponse.getTotalCount());
    }

    /**
     * 用例场景：获取所有租户信息
     * 前置条件：查不到租户信息
     * 检查点：查询到租户数量为0
     */
    @Test
    public void listNoTenantSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        BasePage<ProtectedResourcePo> target = new BasePage<>();
        target.setTotal(0);
        target.setItems(Lists.newArrayList());
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        PageListResponse<TenantInfo> tenantInfoPageListResponse = cyberEngineResourceService.listAllTenants(1, 1);
        Assert.assertEquals(0, tenantInfoPageListResponse.getTotalCount());
    }

    /**
     * 用例场景：获取设备下的租户信息
     * 前置条件：设备可查到租户
     * 检查点：查询到租户数量不为0
     */
    @Test
    public void listAllTenantsByDeviceIdSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        BasePage<ProtectedResourcePo> target = buildProtectedResourcePoBasePage();
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        PageListResponse<TenantInfo> tenantInfoPageListResponse = cyberEngineResourceService.listAllTenantsByDeviceId(
            "xxx");
        Assert.assertEquals(1, tenantInfoPageListResponse.getTotalCount());
    }

    /**
     * 用例场景：获取设备下的租户信息
     * 前置条件：查不到租户信息
     * 检查点：查询到租户数量为0
     */
    @Test
    public void listNoTenantByDeviceIdSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        BasePage<ProtectedResourcePo> target = new BasePage<>();
        target.setTotal(0);
        target.setItems(Lists.newArrayList());
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        PageListResponse<TenantInfo> tenantInfoPageListResponse = cyberEngineResourceService.listAllTenantsByDeviceId(
            "xxx");
        Assert.assertEquals(0, tenantInfoPageListResponse.getTotalCount());
    }

    /**
     * 用例场景：使用文件系统ID获取租户信息成功
     * 前置条件：设备可查到租户
     * 检查点：查询到租户数量不为0
     */
    @Test
    public void listTenantByResourceIdSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        BasePage<ProtectedResourcePo> target = buildProtectedResourcePoBasePage();
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        PageListResponse<TenantInfo> tenantInfoPageListResponse = cyberEngineResourceService.listTenantByResourceId(1,
            1, "");
        Assert.assertEquals(1, tenantInfoPageListResponse.getTotalCount());
    }

    /**
     * 用例场景：获取设备信息成功
     * 前置条件：可查到设备信息
     * 检查点：查询到指定的设备信息
     */
    @Test
    public void listStorageInfoSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        ArrayList<ProtectedResourcePo> protectedResources = new ArrayList<>();
        ProtectedResourcePo protectedResource = new ProtectedResourcePo();
        protectedResource.setUuid("uuid");
        protectedResource.setName("name");
        protectedResource.setCreatedTime(Timestamp.valueOf("2022-12-24 10:13:12"));
        protectedResource.setAuth("{\"authType\": 1}");
        protectedResource.setRootUuid("deviceId");
        protectedResources.add(protectedResource);
        BasePage<ProtectedResourcePo> target = new BasePage<>();
        target.setItems(protectedResources);
        target.setTotal(1);
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        StorageInfo storageInfo = cyberEngineResourceService.listStorageInfo("deviceId", "tenantId");
        Assert.assertEquals("uuid", storageInfo.getDeviceId());
    }

    /**
     * 用例场景：获取文件系统信息成功
     * 前置条件：设备可查到文件系统
     * 检查点：获取文件系统信息数量不为0。
     */
    @Test
    public void listFileSystemsSuccess() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        BasePage<ProtectedResourcePo> target = buildProtectedResourcePoBasePage();
        PowerMockito.when(repository.query(any())).thenReturn(target);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        PageListResponse<FileSystemInfo> fileSystemInfoPageListResponse = cyberEngineResourceService.listFileSystems(1,
            1, "tenantId", "fileSystemId");
        Assert.assertEquals(1, fileSystemInfoPageListResponse.getTotalCount());
    }

    private BasePage<ProtectedResourcePo> buildProtectedResourcePoBasePage() {
        ArrayList<ProtectedResourcePo> protectedResources = new ArrayList<>();
        ProtectedResourcePo protectedResource = new ProtectedResourcePo();
        protectedResource.setUuid("uuid");
        protectedResource.setName("name");
        ArrayList<ProtectedResourceExtendInfoPo> protectedResourceExtendInfoPos = new ArrayList<>();
        ProtectedResourceExtendInfoPo extendInfoPo = new ProtectedResourceExtendInfoPo();
        extendInfoPo.setUuid("tenantUuid");
        extendInfoPo.setResourceId("deviceId");
        extendInfoPo.setKey("tenantId");
        extendInfoPo.setValue("value");
        protectedResourceExtendInfoPos.add(extendInfoPo);
        protectedResource.setExtendInfoList(protectedResourceExtendInfoPos);
        protectedResource.setCreatedTime(Timestamp.valueOf("2022-12-24 10:13:12"));
        protectedResources.add(protectedResource);
        BasePage<ProtectedResourcePo> target = new BasePage<>();
        target.setItems(protectedResources);
        target.setTotal(1);
        return target;
    }


    /**
     * 用例场景：安全一体机删除受保护环境
     * 前置条件：获取受保护环境id
     * 检查点：删除操作成功
     */
    @Test
    public void delete_10w_filesystem_success() {
        ProtectedResourceRepository repository = PowerMockito.mock(ProtectedResourceRepository.class);
        MessageTemplate<String> messageTemplate = PowerMockito.mock(MessageTemplate.class);
        CyberEngineResourceServiceImpl cyberEngineResourceService = new CyberEngineResourceServiceImpl(repository);
        List<String> deletedList = Collections.singletonList("1");
        Mockito.when(repository.deleteCyberEngineEnvironment(any())).thenReturn(deletedList);
        cyberEngineResourceService.setMessageTemplate(messageTemplate);
        cyberEngineResourceService.deleteEnvironment("test1");
        Assert.assertTrue(true);
    }

    /**
     * 测试场景：查询资源,组装logging所需信息-eventInfo属性
     * 前置条件：传入copyId返回资源信息
     * 检  查  点：资源信息正确
     */
    @Test
    public void get_basic_resource_return_success() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setParentName("System_env");
        protectedResource.setPath("CyberEngine OceanProtect/op28/System_vStore/testj");
        CyberEngineResourceServiceImpl cyberResourceService = Mockito.mock(CyberEngineResourceServiceImpl.class);
        ResourceService resourceService = Mockito.mock(ResourceService.class);
        Mockito.doReturn(Optional.of(protectedResource)).when(resourceService).getBasicResourceById(anyString());
        CopyRestApi copyRestApi = Mockito.mock(CopyRestApi.class);
        Copy copy = new Copy();
        copy.setResourceId("test_resource_id");
        Mockito.doReturn(copy).when(copyRestApi).queryCopyByID(anyString());
        WhiteboxImpl.setInternalState(cyberResourceService, "copyRestApi", copyRestApi);
        WhiteboxImpl.setInternalState(cyberResourceService, "resourceService", resourceService);
        Mockito.doCallRealMethod().when(cyberResourceService).getBasicResourceInfoById(anyString());
        Assert.assertEquals("System_env",
            cyberResourceService.getBasicResourceInfoById("test_copy_id").getTenantName());
    }
}