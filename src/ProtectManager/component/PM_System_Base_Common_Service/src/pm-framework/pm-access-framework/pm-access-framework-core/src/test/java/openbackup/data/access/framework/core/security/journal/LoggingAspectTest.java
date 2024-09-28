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
package openbackup.data.access.framework.core.security.journal;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.security.Operation;

import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;

import org.junit.Assert;
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
import java.util.Arrays;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Logging Aspect Test
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@Import(AnnotationAwareAspectJAutoProxyCreator.class)
@SpringBootTest(
        classes = {Operation.class, LoggingAspect.class, ContextEvaluateService.class, LoggingEvaluateService.class})
@MockBean({
    OperationLogService.class,
    AuthRestApi.class,
    DomainBasedOwnershipVerifier.class,
    RedissonClient.class,
    TokenVerificationService.class,
    AuthNativeApi.class
})
public class LoggingAspectTest {
    @Autowired
    private Operation operation;

    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private OperationLogService operationLogService;


    @Test
    public void test_aspect() {
        TokenBo tokenBo = TokenBo.builder().user(TokenBo.UserBo.builder().name("admin").build()).build();
        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);
        AtomicReference<LegoInternalEvent> reference = new AtomicReference<>();
        PowerMockito.doAnswer(
                        invocation -> {
                            LegoInternalEvent event = invocation.getArgument(0);
                            reference.set(event);
                            return null;
                        })
                .when(operationLogService)
                .sendEvent(any(LegoInternalEvent.class));

        Object result = operation.operate0(new AbstractMap.SimpleEntry<>("resource", Arrays.asList("1", "2")));

        LegoInternalEvent event = reference.get();
        Assert.assertNotNull(event);
        Assert.assertEquals("operation_target_resource1_label", event.getSourceType());
        Assert.assertArrayEquals(new String[] {"admin", "127.0.0.1", "resource"}, event.getEventParam());
        Assert.assertEquals(Arrays.asList("1", "2"), result);
    }

    @Test
    public void test_normalize_batch_arguments() {
        List<List<?>> args =
                LoggingAspect.normalizeBatchArguments(Arrays.asList("123", Arrays.asList("abc", "abc"), null));
        Assert.assertEquals(Arrays.asList(Arrays.asList("123", "abc", null), Arrays.asList("123", "abc", null)), args);
    }
}
