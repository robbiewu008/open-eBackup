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
package openbackup.obs.plugin.provider;

import static org.assertj.core.api.Assertions.assertThatNoException;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.obs.plugin.common.constants.EnvironmentConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * ObjectSetProvider Test
 *
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(EnvironmentLinkStatusHelper.class)
public class ObjectSetProviderTest {
    private final ResourceService resourceService = PowerMockito.mock(ResourceService.class);

    private final ObjectSetProvider objectSetProvider = new ObjectSetProvider(resourceService);

    @Before
    public void init() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Test
    public void test_applicable() {
        ProtectedResource object = new ProtectedResource();
        boolean result = objectSetProvider.applicable(object);
        Assert.assertFalse(result);

        object.setSubType(ResourceSubTypeEnum.OBJECT_SET.getType());
        result = objectSetProvider.applicable(object);
        Assert.assertTrue(result);
    }

    @Test
    public void test_before_create_success() {
        ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        Map<String, String> extendInfo = new HashMap<>();
        object.setExtendInfo(extendInfo);
        extendInfo.put("bucketList", "[{\"name\":\"xxx\"},{\"name\":\"fbg\",\"prefix\":[\"e/abcd/123\"]}]");

        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(1);
        resources.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);
        objectSetProvider.beforeCreate(object);
        Assert.assertNotNull(object.getPath());
    }

    @Test
    public void test_before_create_throw_exception_when_exceed_max_count() {
        ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(256);
        resources.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> objectSetProvider.beforeCreate(object));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.RESOURCE_NUM_EXCEED_LIMIT);
    }

    @Test
    public void test_before_create_throw_exception_when_name_illegal() {
        ProtectedResource object = new ProtectedResource();
        object.setName("1_非法名称");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(1);
        resources.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> objectSetProvider.beforeCreate(object));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ILLEGAL_PARAM);
    }

    @Test
    public void test_before_update_success() {
        ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        Map<String, String> extendInfo = new HashMap<>();
        object.setExtendInfo(extendInfo);
        extendInfo.put("bucketList", "[{\"name\":\"xxx\"},{\"name\":\"fbg\",\"prefix\":[\"e/abc\",\"d/123\"]}]");

        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(1);
        resources.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);
        objectSetProvider.beforeUpdate(object);
        Assert.assertNotNull(object.getPath());
    }

    @Test
    public void test_before_create_throw_exception_when_prefix_exceed_256() {
        ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        Map<String, String> extendInfo = new HashMap<>();
        object.setExtendInfo(extendInfo);
        String prefix = buildOverflowPrefix();
        extendInfo.put("bucketList", "[{\"name\":\"xxx\"},{\"name\":\"fbg\",\"prefix\":" + prefix + "}]");

        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(1);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> objectSetProvider.beforeCreate(object));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ERR_PARAM);

    }

    @Test
    public void test_before_create_throw_exception_when_prefix_length_over_1024() {
        ProtectedResource object = new ProtectedResource();
        object.setName("name");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        Map<String, String> extendInfo = new HashMap<>();
        object.setExtendInfo(extendInfo);
        String prefix = buildOverLengthPrefix();
        extendInfo.put("bucketList", "[{\"name\":\"xxx\"},{\"name\":\"fbg\",\"prefix\":[\"" + prefix + "\"]}]");

        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(1);
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> objectSetProvider.beforeCreate(object));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ERR_PARAM);

    }

    @Test
    public void test_name() {
        assertThatNoException().isThrownBy(() -> Whitebox.invokeMethod(objectSetProvider, "checkName", "test"));
        assertThatNoException().isThrownBy(() -> Whitebox.invokeMethod(objectSetProvider, "checkName", "中文_sdad_234"));
        Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(objectSetProvider, "checkName", "123"));
        Assert.assertThrows(LegoCheckedException.class, () -> Whitebox.invokeMethod(objectSetProvider, "checkName",
            "长度64位长度64位长度64位长度64位长度64位长度64位长度64位长度64位长度64位长度64位长度64位长度64位长度64位"));
    }

    @Test
    public void test_before_create_success_when_storage_type_is_empty() {
        ProtectedResource object = new ProtectedResource();
        object.setName("set");
        object.setRootUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        object.setParentUuid("Storage_ObjectStorage_2_a139cfc81cb144ba8b45aad828c813e4");
        ProtectedEnvironment environment = mockEnvironment();
        object.setEnvironment(environment);
        object.setExtendInfo(new HashMap<>());

        PageListResponse<ProtectedResource> resources = new PageListResponse<>();
        resources.setTotalCount(1);
        resources.setRecords(Collections.singletonList(environment));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(resources);

        objectSetProvider.beforeCreate(object);

        Assert.assertEquals(object.getExtendInfo().get(EnvironmentConstant.KEY_STORAGE_TYPE),
            environment.getExtendInfoByKey(EnvironmentConstant.KEY_STORAGE_TYPE));
    }

    private String buildOverLengthPrefix() {
        StringBuilder prefix = new StringBuilder();
        for (int i = 0; i < 1024; i++) {
            prefix.append("a");
        }
        prefix.append("b");
        return prefix.toString();
    }

    private String buildOverflowPrefix() {
        StringBuilder prefix = new StringBuilder("[\"e/abc\",");
        for (int i = 0; i < 256; i++) {
            prefix.append("\"d/123\",");
        }
        prefix.deleteCharAt(prefix.length() - 1);
        prefix.append("]");
        return prefix.toString();
    }

    public static ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("env_id");
        environment.setName("env_name");
        environment.setEndpoint("127.0.0.1");
        Map<String, String> authExtendInfo = new HashMap<>();
        Authentication auth = new Authentication();
        auth.setAuthKey("test_key");
        auth.setAuthPwd("test_pwd");
        auth.setExtendInfo(authExtendInfo);
        environment.setAuth(auth);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("InternalAgentConnection_6278fe77-af59-419e-b373-d5d075f3f80f", "true");
        extendInfo.put(EnvironmentConstant.KEY_STORAGE_TYPE, "1");
        environment.setExtendInfo(extendInfo);
        return environment;
    }
}
