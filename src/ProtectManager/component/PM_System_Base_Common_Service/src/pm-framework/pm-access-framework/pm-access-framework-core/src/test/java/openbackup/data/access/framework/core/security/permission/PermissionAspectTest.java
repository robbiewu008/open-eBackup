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
package openbackup.data.access.framework.core.security.permission;

import static org.mockito.ArgumentMatchers.any;

import openbackup.data.access.framework.core.security.Operation;

import openbackup.system.base.common.aspect.DomainBasedOwnershipVerifier;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.scurity.TokenVerificationService;

import openbackup.system.base.sdk.user.model.RolePo;
import openbackup.system.base.service.DeployTypeService;
import com.huawei.oceanprotect.system.base.user.service.UserInternalService;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.springframework.aop.aspectj.annotation.AnnotationAwareAspectJAutoProxyCreator;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.AbstractMap;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

/**
 * Logging Aspect Test
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@Import(AnnotationAwareAspectJAutoProxyCreator.class)
@SpringBootTest(classes = {PermissionAspect.class, Operation.class})
@MockBean({TokenVerificationService.class, UserTokenValidateService.class, DomainBasedOwnershipVerifier.class,
    DeployTypeService.class, UserInternalService.class})
public class PermissionAspectTest {
    @Autowired
    private Operation operation;

    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private DomainBasedOwnershipVerifier domainBasedOwnershipVerifier;

    @Autowired
    private UserInternalService userInternalService;

    @Autowired
    private DeployTypeService deployTypeService;

    @Test
    public void test_aspect_success_when_X_series_check_permisssion_false() {
        TokenBo.UserBo userBo = TokenBo.UserBo.builder()
            .id("0")
            .name("username")
            .roles(Collections.singletonList(TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_DP_ADMIN).build()))
            .passwordVersion(0L)
            .build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);
        PowerMockito.when(userInternalService.getDomainDefaultRoleSet(Mockito.anyString())).thenReturn(
            prepareRolePo(Constants.Builtin.ROLE_DP_ADMIN, "123"));
        PowerMockito.when(deployTypeService.getDeployType()).thenReturn(DeployTypeEnum.X8000);
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setId(userBo.getId());
        userInfo.setName(userBo.getName());
        userInfo.setRoles(userBo.getRoles());
        userInfo.setPasswordVersion(userBo.getPasswordVersion());

        PowerMockito.when(domainBasedOwnershipVerifier.getType()).thenReturn("resource");

        AtomicReference<TokenBo.UserBo> userBoAtomicReference = new AtomicReference<>();
        AtomicReference<List<String>> resourceUuidListReference = new AtomicReference<>();
        PowerMockito.doAnswer(invocation -> {
            userBoAtomicReference.set(invocation.getArgument(0));
            resourceUuidListReference.set(invocation.getArgument(1));
            return null;
        }).when(domainBasedOwnershipVerifier).verify(any(), any());

        Object result = operation.operate0(new AbstractMap.SimpleEntry<>("resource", Arrays.asList("1", "2")));

        Assert.assertNotNull(userBoAtomicReference.get());
        Assert.assertNotNull(resourceUuidListReference.get());
        Assert.assertSame(userBo, userBoAtomicReference.get());
        Assert.assertEquals(Arrays.asList("1", "2"), resourceUuidListReference.get());
        Assert.assertEquals(Arrays.asList("1", "2"), result);
    }

    private RolePo prepareRolePo(String roleName, String roleId) {
        RolePo rolePo = new RolePo();
        rolePo.setRoleName(roleName);
        rolePo.setRoleId(roleId);
        return rolePo;
    }
}
