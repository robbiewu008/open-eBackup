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
package openbackup.kingbase.protection.access.provider.resource;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * {@link KingBaseInstanceProvider} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-25
 */
public class KingBaseInstanceProviderTest {
    private final ProviderManager providerManager = PowerMockito.mock(ProviderManager.class);

    private final ResourceConnectionCheckProvider resourceConnectionCheckProvider = PowerMockito.mock(
        ResourceConnectionCheckProvider.class);

    private final InstanceResourceService instanceResourceService = PowerMockito.mock(InstanceResourceService.class);

    private final KingBaseInstanceProvider provider = new KingBaseInstanceProvider(providerManager,
        instanceResourceService);

    /**
     * 用例场景：KingBases实例适配器
     * 前置条件：输入资源信息
     * 检查点：是否返回true
     */
    @Test
    public void applicable_kingbase_instance_provider_success() {
        ProtectedResource resource = new ProtectedResource();
        resource.setSubType(ResourceSubTypeEnum.KING_BASE_INSTANCE.getType());
        Assert.assertTrue(provider.applicable(resource));
        resource.setSubType(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType());
        Assert.assertFalse(provider.applicable(resource));
    }

    /**
     * 用例场景：创建kingbase实例前检查
     * 前置条件：实例没有被创建
     * 检查点：无异常抛出
     */
    @Test
    public void execute_before_create_success() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(mockContext());
        ProtectedResource resource = mockResource();
        provider.beforeCreate(resource);
        Assert.assertEquals("V008R006C005B0054", resource.getVersion());
        Assert.assertEquals("/data", resource.getExtendInfoByKey(DatabaseConstants.DATA_DIRECTORY));
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    /**
     * 用例场景：修改kingbase实例前检查
     * 前置条件：端口未被修改
     * 检查点：无异常抛出
     */
    @Test
    public void execute_before_update_success() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(mockContext());
        provider.beforeUpdate(mockResource());
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：创建kingbase实例前连通性检查
     * 前置条件：检查结果为空
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_result_is_empty_when_before_create() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = mockContext();
        context.setActionResults(new ArrayList<>());
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(context);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> provider.beforeUpdate(mockResource()));
        Assert.assertEquals("check connection result is empty.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.ERR_PARAM, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：创建kingbase实例前连通性检查
     * 前置条件：检查结果为失败
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_result_is_failed_when_before_create() {
        PowerMockito.when(providerManager.findProvider(any(), any())).thenReturn(resourceConnectionCheckProvider);
        ResourceCheckContext context = mockContext();
        context.getActionResults().get(IsmNumberConstant.ZERO).setCode(IsmNumberConstant.TWO);
        context.getActionResults().get(IsmNumberConstant.ZERO).setBodyErr(CommonErrorCode.WRONG_PASSWORD + "");
        PowerMockito.when(resourceConnectionCheckProvider.tryCheckConnection(any()))
            .thenReturn(context);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> provider.beforeUpdate(mockResource()));
        Assert.assertEquals("check connection failed.", legoCheckedException.getMessage());
        Assert.assertEquals(CommonErrorCode.WRONG_PASSWORD, legoCheckedException.getErrorCode());
    }

    private ProtectedResource mockResource() {
        ProtectedResource resource = new ProtectedResource();
        resource.setParentUuid(UUID.randomUUID().toString());
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(DatabaseConstants.INSTANCE_PORT, "54321");
        resource.setExtendInfo(extendInfo);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint("8.40.97.125");
        resource.setEnvironment(environment);
        return resource;
    }

    private ResourceCheckContext mockContext() {
        Map<String, String> map = new HashMap<>();
        map.put(DatabaseConstants.VERSION, "V008R006C005B0054");
        map.put(DatabaseConstants.DATA_DIRECTORY, "/data");
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(DatabaseConstants.SUCCESS_CODE);
        actionResult.setMessage(JSONObject.fromObject(map).toString());
        List<ActionResult> list = new ArrayList<>();
        list.add(actionResult);
        ResourceCheckContext context = new ResourceCheckContext();
        context.setActionResults(list);
        return context;
    }
}