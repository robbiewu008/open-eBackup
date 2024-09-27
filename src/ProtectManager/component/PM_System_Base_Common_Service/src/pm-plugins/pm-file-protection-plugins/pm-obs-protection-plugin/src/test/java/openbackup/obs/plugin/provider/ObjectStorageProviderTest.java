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
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.obs.plugin.service.ObjectStorageAgentService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.enums.ObjectStorageTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import org.apache.commons.lang3.StringUtils;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Answers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PowerMockIgnore;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author w00607005
 * @since 2023-11-15
 */
@RunWith(PowerMockRunner.class)
@PowerMockIgnore( {"javax.management.*", "jdk.internal.reflect.*"})
@PrepareForTest(value = {
    ObjectStorageTypeEnum.class, ImmutableMap.class, StringUtils.class, Long.class, Lists.class, Collections.class,
    Integer.class, String.class, EnvironmentLinkStatusHelper.class
})
public class ObjectStorageProviderTest {
    @InjectMocks
    private ObjectStorageProvider objectStorageProvider;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ResourceService resourceService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private EncryptorService encryptorService;

    @Mock(answer = Answers.RETURNS_DEEP_STUBS)
    private ObjectStorageAgentService agentService;

    @Before
    public void init() {
        PowerMockito.mockStatic(EnvironmentLinkStatusHelper.class);
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    @Test
    public void test_applicable_success() {
        // run the test
        boolean result = objectStorageProvider.applicable(ResourceSubTypeEnum.OBJECT_STORAGE.getType());

        // verify the results
        Assert.assertTrue(result);
    }

    @Test
    public void test_check_success() {
        // setup
        PowerMockito.when(resourceService.getResourceById(anyString()))
            .thenReturn(Optional.of(createProtectedEnvironment()));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any(ImmutableMap.class)).getTotalCount())
            .thenReturn(0);

        PowerMockito.when(encryptorService.encrypt(anyString())).thenReturn("string");
        PowerMockito.when(encryptorService.decrypt(anyString())).thenReturn("string");

        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        actionResult.setMessage("tenantid");
        PowerMockito.when(agentService.checkConnection(any(ProtectedEnvironment.class)))
            .thenReturn(new ActionResult[] {actionResult});
        PowerMockito.when(resourceService.getResourceById(eq("storage_ObjectStorage_2_tenantid")))
            .thenReturn(Optional.empty());

        // run the test
        objectStorageProvider.register(createProtectedEnvironment());
    }

    @Test
    public void test_check_name() {
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        actionResult.setMessage("tenantid");
        PowerMockito.when(agentService.checkConnection(any(ProtectedEnvironment.class)))
            .thenReturn(new ActionResult[] {actionResult});
        PowerMockito.when(resourceService.getResourceById(eq("storage_ObjectStorage_2_tenantid")))
            .thenReturn(Optional.empty());

        // run the test
        ProtectedEnvironment environment = createProtectedEnvironment();
        assertThatNoException().isThrownBy(() -> objectStorageProvider.register(environment));

        environment.setName("123");
        Assert.assertThrows(LegoCheckedException.class, () -> objectStorageProvider.register(environment));

        ProtectedEnvironment environment2 = createHttpProtectedEnvironment();
        assertThatNoException().isThrownBy(() -> objectStorageProvider.register(environment2));
    }

    @Test
    public void test_health_check_success() {
        // setup
        ActionResult actionResult = new ActionResult();
        actionResult.setCode(0L);
        actionResult.setMessage("tenantid");
        PowerMockito.when(agentService.checkConnection(any(ProtectedEnvironment.class)))
            .thenReturn(new ActionResult[] {actionResult});

        // run the test
        objectStorageProvider.validate(createProtectedEnvironment());
    }

    @Test
    public void test_check_proxy_success() throws Exception {
        // setup
        ProtectedEnvironment environment = new ProtectedEnvironment();
        Authentication auth = new Authentication();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("proxyEnable", "1");
        extendInfo.put("proxyHostName", "http://20.161.161.232:8081");
        auth.setExtendInfo(extendInfo);
        environment.setAuth(auth);
        // run the test
        Whitebox.invokeMethod(objectStorageProvider, "checkProxy", environment);
        Assert.assertEquals("", environment.getAuth().getExtendInfo().get("proxyUserName"));
    }

    @Test
    public void test_check_env_link_status() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("1");
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setLinkStatus("1");
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner(eq("1")))
            .thenReturn(Optional.of(protectedEnvironment));
        assertThatNoException().isThrownBy(
            () -> Whitebox.invokeMethod(objectStorageProvider, "checkEnvLinkStatus", environment));

        protectedEnvironment.setLinkStatus("0");
        PowerMockito.when(resourceService.getResourceByIdIgnoreOwner(eq("1")))
            .thenReturn(Optional.of(protectedEnvironment));
        PowerMockito.when(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(any()))
            .thenReturn(LinkStatusEnum.OFFLINE.getStatus().toString());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(objectStorageProvider, "checkEnvLinkStatus", environment));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.RESOURCE_LINK_STATUS_OFFLINE);
    }

    private ProtectedEnvironment createProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("testName");
        protectedEnvironment.setEndpoint("8.40.97.140");
        protectedEnvironment.setPort(0);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("storageType", "2");
        extendInfo.put("agents", "3d6ab3a6-3aa6-4051-8e48-c028ac4de213");
        protectedEnvironment.setExtendInfo(extendInfo);
        protectedEnvironment.setType("storage");
        protectedEnvironment.setSubType("ObjectStorage");
        protectedEnvironment.setCreatedTime(String.valueOf(System.currentTimeMillis()));
        protectedEnvironment.setRegisterType("1");
        Authentication auth = new Authentication();
        Map<String, String> map = new HashMap<>();
        map.put("ak", "testak");
        map.put("sk", "testsk");
        map.put("certification", "");
        map.put("useHttps", "1");
        map.put("proxyEnable", "0");
        map.put("proxyHostName", "");
        map.put("proxyUserName", "");
        map.put("proxyUserPwd", "");
        auth.setExtendInfo(map);
        protectedEnvironment.setAuth(auth);
        return protectedEnvironment;
    }

    private ProtectedEnvironment createHttpProtectedEnvironment() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("testName");
        protectedEnvironment.setEndpoint("www.baidu.com:8088");
        protectedEnvironment.setPort(0);
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("storageType", "2");
        extendInfo.put("agents", "3d6ab3a6-3aa6-4051-8e48-c028ac4de213");
        protectedEnvironment.setExtendInfo(extendInfo);
        protectedEnvironment.setType("storage");
        protectedEnvironment.setSubType("ObjectStorage");
        protectedEnvironment.setCreatedTime(String.valueOf(System.currentTimeMillis()));
        protectedEnvironment.setRegisterType("1");
        Authentication auth = new Authentication();
        Map<String, String> map = new HashMap<>();
        map.put("ak", "testak");
        map.put("sk", "testsk");
        map.put("certification", "");
        map.put("useHttps", "0");
        map.put("proxyEnable", "0");
        map.put("proxyHostName", "");
        map.put("proxyUserName", "");
        map.put("proxyUserPwd", "");
        auth.setExtendInfo(map);
        protectedEnvironment.setAuth(auth);
        return protectedEnvironment;
    }
}