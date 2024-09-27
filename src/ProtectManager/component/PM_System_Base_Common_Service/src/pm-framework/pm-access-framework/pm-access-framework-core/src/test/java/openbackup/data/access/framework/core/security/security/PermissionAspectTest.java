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
package openbackup.data.access.framework.core.security.security;

import static org.mockito.ArgumentMatchers.any;

import com.huawei.oceanprotect.system.base.user.service.UserInternalService;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.core.security.Operation;
import openbackup.data.access.framework.core.security.journal.LoggingAspect;
import openbackup.data.access.framework.core.security.permission.PermissionAspect;
import openbackup.data.access.framework.core.security.permission.UserTokenValidateService;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.config.DataSourceConfig;
import openbackup.system.base.sdk.user.model.RolePo;
import openbackup.system.base.security.permission.Permission;
import openbackup.system.base.service.DeployTypeService;

import com.zaxxer.hikari.HikariDataSource;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.reflect.MethodSignature;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.mock.web.MockHttpServletRequest;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.lang.reflect.Method;
import java.util.Collections;

/**
 * PermissionAspectTest
 *
 * @author jwx701567
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-03-21
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {
    PermissionAspect.class, Operation.class
})
public class PermissionAspectTest {
    @Autowired
    private Operation operation;

    @Autowired
    private PermissionAspect permissionAspect;

    @MockBean
    private TokenVerificationService tokenVerificationService;

    @MockBean
    private ProceedingJoinPoint proceedingJoinPoint;

    @MockBean
    private UserTokenValidateService userTokenValidateService;

    @MockBean
    private MethodSignature methodSignature;

    @MockBean
    private Permission permission;

    @MockBean
    private LoggingAspect loggingAspect;

    @MockBean
    private UserInternalService userInternalService;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private ProviderManager providerManager;

    /**
     * 用例名称：当前用户审计员，接口权限设备管理员角色，post接口
     * <p>
     * 前置条件：无
     * 检查点： 1、当前用户审计员，接口权限设备管理员角色，post接口 会报权限不对的异常
     */
    @Test
    public void should_throw_ACCESS_DENIED_if_post_request_and_Role_Auditor_when_access_denied() throws Throwable {
        // Given
        ProceedingJoinPoint proceedingJoinPoint = PowerMockito.mock(ProceedingJoinPoint.class);
        Permission permission = PowerMockito.mock(Permission.class);
        TokenBo.UserBo userBo = TokenBo.UserBo.builder()
            .id("0")
            .name("username")
            .roles(
                Collections.singletonList(TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_DEVICE_MANAGER).build()))
            .passwordVersion(0L)
            .build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        // when
        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);
        PowerMockito.when(permission.roles()).thenReturn(new String[] {"Role_Auditor"});
        PowerMockito.when(permission.resources()).thenReturn(new String[] {"resource"});
        PowerMockito.when(proceedingJoinPoint.getArgs()).thenReturn(new String[] {});
        PowerMockito.when(proceedingJoinPoint.getSignature()).thenReturn(methodSignature);
        Method method = DataSourceConfig.class.getDeclaredMethod("initPoolConfig", HikariDataSource.class);
        PowerMockito.when(methodSignature.getMethod()).thenReturn(method);
        RolePo rolePo = new RolePo();
        rolePo.setRoleId("1");
        rolePo.setRoleName("SYSADMIN");
        PowerMockito.when(userInternalService.getDomainDefaultRoleSet(any())).thenReturn(rolePo);
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("POST");
        RequestContextHolder.setRequestAttributes(new ServletRequestAttributes(request));
        PowerMockito.doNothing().when(userTokenValidateService).validate(proceedingJoinPoint, tokenBo);
        // Then
        Assert.assertThrows(LegoCheckedException.class,
            () -> permissionAspect.aspectAccess(proceedingJoinPoint, permission));
    }

    @Test
    public void should_throw_ACCESS_DENIED_if_post_request_and_hcs_read_only_when_access_denied() throws Throwable {
        // Given
        ProceedingJoinPoint proceedingJoinPoint = PowerMockito.mock(ProceedingJoinPoint.class);
        Permission permission = PowerMockito.mock(Permission.class);
        TokenBo.UserBo userBo = TokenBo.UserBo.builder()
            .id("0")
            .name("username")
            .roles(
                Collections.singletonList(TokenBo.RoleBo.builder().name(Constants.Builtin.ROLE_DP_ADMIN).build()))
            .passwordVersion(0L).userType("HCS").isHcsUserManagePermission(false)
            .build();
        TokenBo tokenBo = TokenBo.builder().user(userBo).build();
        // when
        PowerMockito.when(tokenVerificationService.parsingTokenFromRequest()).thenReturn(tokenBo);
        PowerMockito.when(permission.roles()).thenReturn(new String[] {"Role_DP_Admin"});
        PowerMockito.when(permission.resources()).thenReturn(new String[] {"resource"});
        PowerMockito.when(proceedingJoinPoint.getArgs()).thenReturn(new String[] {});
        PowerMockito.when(proceedingJoinPoint.getSignature()).thenReturn(methodSignature);
        Method method = DataSourceConfig.class.getDeclaredMethod("initPoolConfig", HikariDataSource.class);
        PowerMockito.when(methodSignature.getMethod()).thenReturn(method);
        RolePo rolePo = new RolePo();
        rolePo.setRoleId("1");
        rolePo.setRoleName("SYSADMIN");
        PowerMockito.when(userInternalService.getDomainDefaultRoleSet(any())).thenReturn(rolePo);
        MockHttpServletRequest request = new MockHttpServletRequest();
        request.setMethod("POST");
        RequestContextHolder.setRequestAttributes(new ServletRequestAttributes(request));
        PowerMockito.doNothing().when(userTokenValidateService).validate(proceedingJoinPoint, tokenBo);
        // Then
        Assert.assertThrows(LegoCheckedException.class,
            () -> permissionAspect.aspectAccess(proceedingJoinPoint, permission));
    }
}
