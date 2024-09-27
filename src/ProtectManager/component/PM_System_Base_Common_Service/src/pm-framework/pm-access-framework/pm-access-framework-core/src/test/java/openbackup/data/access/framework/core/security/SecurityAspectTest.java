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
package openbackup.data.access.framework.core.security;

import static org.junit.jupiter.api.Assertions.assertThrows;
import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.security.journal.ContextEvaluateService;
import openbackup.data.access.framework.core.security.journal.LoggingAspect;
import openbackup.data.access.framework.core.security.journal.LoggingEvaluateService;
import openbackup.data.access.framework.core.security.permission.PermissionAspect;
import openbackup.data.access.framework.core.security.permission.UserTokenValidateService;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.security.callee.CalleeMethodScan;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RedissonClient;
import org.springframework.aop.aspectj.annotation.AnnotationAwareAspectJAutoProxyCreator;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.Consumer;

/**
 * Security Aspect Test
 *
 * @author l00272247
 * @since 2022-01-08
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@Import(AnnotationAwareAspectJAutoProxyCreator.class)
@SpringBootTest(
        classes = {
            Operation.class,
            LoggingAspect.class,
            PermissionAspect.class,
            ContextEvaluateService.class,
            LoggingEvaluateService.class
        })
@MockBean({
    OperationLogService.class,
    UserTokenValidateService.class,
    DomainBasedOwnershipVerifier.class,
    RedissonClient.class,
    TokenVerificationService.class,
    AuthRestApi.class,
    AuthNativeApi.class
})
@CalleeMethodScan
public class SecurityAspectTest {
    @Autowired
    private Operation operation;

    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private OperationLogService operationLogService;

    @Autowired
    private AuthRestApi authRestApi;

    @Autowired
    private DomainBasedOwnershipVerifier domainBasedOwnershipVerifier;

    private List<LegoInternalEvent> eventReferences;

    @Before
    public void init() {
        TokenBo.UserBo userBo =
                TokenBo.UserBo.builder()
                        .id("0")
                        .name("username")
                        .roles(
                                Collections.singletonList(
                                        TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_RD_ADMIN).build()))
                        .passwordVersion(0L)
                        .build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);

        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setId(userBo.getId());
        userInfo.setName(userBo.getName());
        userInfo.setRoles(userBo.getRoles());
        userInfo.setPasswordVersion(userBo.getPasswordVersion());
        PowerMockito.when(authRestApi.queryUserInfoByName(any())).thenReturn(userInfo);

        PowerMockito.when(domainBasedOwnershipVerifier.getType()).thenReturn("resource");
        AtomicReference<TokenBo.UserBo> userBoAtomicReference = new AtomicReference<>();
        AtomicReference<List<String>> resourceUuidListReference = new AtomicReference<>();
        PowerMockito.doAnswer(
                        invocation -> {
                            userBoAtomicReference.set(invocation.getArgument(0));
                            resourceUuidListReference.set(invocation.getArgument(1));
                            return null;
                        })
                .when(domainBasedOwnershipVerifier)
                .verify(any(), any());

        eventReferences = new ArrayList<>();
        PowerMockito.doAnswer(
                        invocation -> {
                            LegoInternalEvent event = invocation.getArgument(0);
                            eventReferences.add(event);
                            return null;
                        })
                .when(operationLogService)
                .sendEvent(any(LegoInternalEvent.class));
    }

    @Test
    public void test_access_denied() {
        LegoCheckedException error;
        try {
            operation.operate0(new AbstractMap.SimpleEntry<>("resource", Arrays.asList("1", "2")));
            error = null;
        } catch (LegoCheckedException e) {
            error = e;
        }
        Assert.assertNotNull(error);
        Assert.assertEquals(CommonErrorCode.ACCESS_DENIED, error.getErrorCode());
        Assert.assertTrue(eventReferences.isEmpty());
    }

    @Test
    public void test_operation_failed() {
        LegoCheckedException error;
        try {
            operation.operate1(new AbstractMap.SimpleEntry<>("resource", Arrays.asList("1", "2")));
            error = null;
        } catch (LegoCheckedException e) {
            error = e;
        }
        Assert.assertNotNull(error);
        Assert.assertEquals(CommonErrorCode.OPERATION_FAILED, error.getErrorCode());
        Assert.assertEquals("operation_target_resource1_label", eventReferences.get(0).getSourceType());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource"}, eventReferences.get(0).getEventParam());
        Assert.assertEquals(false, eventReferences.get(0).getIsSuccess());
    }

    @Test
    public void test_context() {
        List<String> value = Arrays.asList("1", "2");
        Object result = operation.operate2(new AbstractMap.SimpleEntry<>("resource", value));
        Assert.assertEquals("operation_target_resource1_label", eventReferences.get(0).getSourceType());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:resource-data"},
                eventReferences.get(0).getEventParam());
        Assert.assertEquals(true, eventReferences.get(0).getIsSuccess());
        Assert.assertSame(value, result);
    }

    @Test
    public void test_callee_method() {
        List<String> value = Arrays.asList("1", "2");
        Object result = operation.operate3(new AbstractMap.SimpleEntry<>("resource", value));
        Assert.assertEquals("operation_target_resource1_label", eventReferences.get(0).getSourceType());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:resource-data"},
                eventReferences.get(0).getEventParam());
        Assert.assertEquals(true, eventReferences.get(0).getIsSuccess());
        Assert.assertSame(value, result);
    }

    @Test
    public void test_return_value() {
        List<String> value = Arrays.asList("1", "2");
        Object result = operation.operate4(new AbstractMap.SimpleEntry<>("resource", value));
        Assert.assertEquals("operation_target_resource1_label", eventReferences.get(0).getSourceType());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:1"}, eventReferences.get(0).getEventParam());
        Assert.assertEquals(true, eventReferences.get(0).getIsSuccess());
        Assert.assertSame(value, result);
    }

    @Test
    public void test_batch() {
        List<String> value = Arrays.asList("1", "2");
        Object result = operation.operate5(new AbstractMap.SimpleEntry<>("resource", value));
        Assert.assertEquals("operation_target_resource1_label", eventReferences.get(0).getSourceType());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:1:1"}, eventReferences.get(0).getEventParam());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:2:1"}, eventReferences.get(1).getEventParam());
        Assert.assertEquals(true, eventReferences.get(0).getIsSuccess());
        Assert.assertEquals(true, eventReferences.get(1).getIsSuccess());
        Assert.assertSame(value, result);
    }

    @Test
    public void test_requires() {
        test_requires(value -> operation.operate6(new AbstractMap.SimpleEntry<>("resource", value)));
        test_requires(value -> operation.operate7(new AbstractMap.SimpleEntry<>("resource", value)));
    }

    private void test_requires(Consumer<List<String>> consumer) {
        List<String> value = Arrays.asList("1", "2");
        LegoCheckedException exception = assertThrows(LegoCheckedException.class, () -> consumer.accept(value));
        Assert.assertEquals("operation_target_resource1_label", eventReferences.get(0).getSourceType());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:1"}, eventReferences.get(0).getEventParam());
        Assert.assertArrayEquals(
                new String[] {"username", "127.0.0.1", "resource:2"}, eventReferences.get(1).getEventParam());
        Assert.assertEquals(false, eventReferences.get(0).getIsSuccess());
        Assert.assertEquals(false, eventReferences.get(1).getIsSuccess());
        Assert.assertEquals(CommonErrorCode.OBJ_NOT_EXIST, exception.getErrorCode());
        Assert.assertTrue(exception.getMessage().contains("object not exist"));
    }

    @Test
    public void test_data_protection_access_exception() {
        List<String> value = Arrays.asList("1", "2");
        DataProtectionAccessException dataProtectionAccessException =
                assertThrows(
                        DataProtectionAccessException.class,
                        () -> operation.operate8(new AbstractMap.SimpleEntry<>("resource", value)));
        Assert.assertEquals(1, dataProtectionAccessException.getErrorCode());
        Assert.assertArrayEquals(new String[] {"1"}, dataProtectionAccessException.getParameters());

        LegoCheckedException legoCheckedException =
                assertThrows(
                        LegoCheckedException.class,
                        () -> operation.operate9(new AbstractMap.SimpleEntry<>("resource", value)));
        Assert.assertEquals(2, legoCheckedException.getErrorCode());
        Assert.assertArrayEquals(new String[] {"2"}, legoCheckedException.getParameters());
    }
}
