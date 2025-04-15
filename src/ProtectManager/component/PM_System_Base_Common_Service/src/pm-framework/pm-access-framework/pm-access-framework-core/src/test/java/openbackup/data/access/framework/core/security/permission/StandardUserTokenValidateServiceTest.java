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

import openbackup.data.access.framework.core.security.permission.StandardUserTokenValidateService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.constants.TokenBo.RoleBo;
import openbackup.system.base.common.constants.TokenBo.UserBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.Signature;
import org.aspectj.lang.reflect.MethodSignature;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.springframework.core.annotation.AnnotatedElementUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

@RunWith(PowerMockRunner.class)
@PrepareForTest( {UserUtils.class, AnnotatedElementUtils.class})
public class StandardUserTokenValidateServiceTest {
    @InjectMocks
    private StandardUserTokenValidateService standardUserTokenValidateService;

    @Mock
    private AuthNativeApi authNativeApi;

    /**
     * 用例场景：用户第一次登录的时候密码未修改
     * 前置条件：无
     * 检查点：有异常抛出
     */
    @Test
    public void should_throw_IllegalStateException_when_password_first_modify() throws Exception {
        ProceedingJoinPoint joinPoint = Mockito.mock(ProceedingJoinPoint.class);
        MethodSignature signature = Mockito.mock(MethodSignature.class);
        PowerMockito.when(joinPoint.getSignature()).thenReturn(signature);

        TokenBo token = mockToken("COMMON", "233");
        UserBo userBo = token.getUser();

        PowerMockito.when(authNativeApi.queryUserInfoByName(Mockito.anyString()))
            .thenReturn(mockUserInfo(userBo.getId(), userBo.getPasswordVersion(), userBo, true));
        PowerMockito.mockStatic(UserUtils.class);
        PowerMockito.when(UserUtils.isNeedValidatePasswordVersion(Mockito.any())).thenReturn(true);
        PowerMockito.doNothing().when(UserUtils.class, "deleteUserCacheAndSessionInfo", Mockito.any(), Mockito.any());
        PowerMockito.mockStatic(AnnotatedElementUtils.class);
        PowerMockito.when(AnnotatedElementUtils.findMergedAnnotation(Mockito.any(), Mockito.any())).thenReturn(null);
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> standardUserTokenValidateService.validate(joinPoint, token));
        Assert.assertEquals(CommonErrorCode.PASSWORD_FIRST_MODIFY_NOTICE, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：无需修改密码
     * 前置条件：无
     * 检查点：运行成功，无异常抛出
     */
    @Test
    public void test_not_modify_password() {
        ProceedingJoinPoint joinPoint = Mockito.mock(ProceedingJoinPoint.class);
        MethodSignature signature = Mockito.mock(MethodSignature.class);
        PowerMockito.when(joinPoint.getSignature()).thenReturn(signature);

        TokenBo token = mockToken("COMMON", "233");
        UserBo userBo = token.getUser();
        userBo.getRoles().get(0).setId(String.valueOf(4L));
        TokenBo.UserInfo userInfo = mockUserInfo(userBo.getId(), userBo.getPasswordVersion(), userBo, true);
        Optional<TokenBo.UserInfo> optional = Optional.of(userInfo);
        PowerMockito.when(authNativeApi.queryUserInfoById(Mockito.anyString()))
            .thenReturn(optional);
        standardUserTokenValidateService.validate(joinPoint, token);
        Mockito.verify(authNativeApi, Mockito.times(1)).queryUserInfoById(Mockito.anyString());
    }

    /**
     * 用例场景：签名不是方法签名的实例
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_IllegalStateException_when_signature_not_instanceof_MethodSignature() {
        ProceedingJoinPoint joinPoint = Mockito.mock(ProceedingJoinPoint.class);
        Signature signature = Mockito.mock(Signature.class);
        PowerMockito.when(joinPoint.getSignature()).thenReturn(signature);
        TokenBo token = mockToken("233", "233");
        Assert.assertThrows(LegoCheckedException.class,
            () -> standardUserTokenValidateService.validate(joinPoint, token));
    }

    /**
     * 用例场景：用户被删除，然后重新创建相同的用户，之前登录用户token未失效
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_id_not_equal() {
        ProceedingJoinPoint joinPoint = Mockito.mock(ProceedingJoinPoint.class);
        MethodSignature signature = Mockito.mock(MethodSignature.class);
        PowerMockito.when(joinPoint.getSignature()).thenReturn(signature);
        TokenBo token = mockToken("COMMON", String.valueOf(Constants.ROLE_RD_ADMIN));
        UserBo userBo = token.getUser();

        PowerMockito.when(authNativeApi.queryUserInfoByName(Mockito.anyString()))
            .thenReturn(
                mockUserInfo(String.valueOf(Constants.ROLE_DR_ADMIN), userBo.getPasswordVersion(), userBo, false));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> standardUserTokenValidateService.validate(joinPoint, token));
        Assert.assertEquals(CommonErrorCode.ACCESS_DENIED, legoCheckedException.getErrorCode());
    }

    /**
     * 用例场景：userDbInfo的passwordVersion为空
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_userDbInfo_pwdVersion_is_null() {
        checkPasswordVersion(0L, null);
    }

    /**
     * 用例场景：userDbInfo和用户信息bo的passwordVersion不一致
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_userDbInfo_pwdVersion_is_not_equal_userBo() {
        checkPasswordVersion(0L, 1L);
    }

    /**
     * 用例场景：用户信息bo的passwordVersion为空
     * 前置条件：无
     * 检查点：抛异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_userBo_pwdVersion_is_null() {
        checkPasswordVersion(null, 1L);
    }

    private TokenBo.UserInfo mockUserInfo(String id, Long passwordVersion, UserBo userBo, boolean mustModifyPassword) {
        TokenBo.UserInfo userInfo = new TokenBo.UserInfo();
        userInfo.setId(id);
        userInfo.setName(userBo.getName());
        userInfo.setRoles(userBo.getRoles());
        userInfo.setUserType("COMMON");
        userInfo.setPasswordVersion(passwordVersion);
        userInfo.setMustModifyPassword(mustModifyPassword);
        return userInfo;
    }

    private void checkPasswordVersion(Long userBoPasswordVersion, Long userDbInfoPasswordVersion) {
        ProceedingJoinPoint joinPoint = Mockito.mock(ProceedingJoinPoint.class);
        MethodSignature signature = Mockito.mock(MethodSignature.class);
        PowerMockito.when(joinPoint.getSignature()).thenReturn(signature);

        TokenBo token = Mockito.mock(TokenBo.class);
        UserBo userBo = new UserBo();
        userBo.setId(String.valueOf(Constants.ROLE_RD_ADMIN));
        userBo.setName("123");
        userBo.setUserType("COMMON");
        List<RoleBo> roles = new ArrayList<>();
        roles.add(new RoleBo());
        userBo.setRoles(roles);
        userBo.setPasswordVersion(userBoPasswordVersion);
        PowerMockito.when(token.getUser()).thenReturn(userBo);
        PowerMockito.when(authNativeApi.queryUserInfoByName(Mockito.anyString()))
            .thenReturn(mockUserInfo(userBo.getId(), userDbInfoPasswordVersion, userBo, false));

        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> standardUserTokenValidateService.validate(joinPoint, token));
        Assert.assertEquals(CommonErrorCode.ACCESS_DENIED, legoCheckedException.getErrorCode());
    }

    @Test
    public void checkLdapUserId() {
        ProceedingJoinPoint joinPoint = Mockito.mock(ProceedingJoinPoint.class);
        MethodSignature signature = Mockito.mock(MethodSignature.class);
        PowerMockito.when(joinPoint.getSignature()).thenReturn(signature);
        TokenBo token = mockToken("LDAP", String.valueOf(Constants.ROLE_RD_ADMIN));
        UserBo userBo = token.getUser();

        PowerMockito.when(authNativeApi.queryUserInfoByName(Mockito.anyString()))
            .thenReturn(mockUserInfo("2", userBo.getPasswordVersion(), userBo, false));
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> standardUserTokenValidateService.validate(joinPoint, token));
        Assert.assertEquals(CommonErrorCode.ACCESS_DENIED, legoCheckedException.getErrorCode());

    }

    private TokenBo mockToken(String userType, String id) {
        TokenBo token = Mockito.mock(TokenBo.class);
        UserBo userBo = new UserBo();
        userBo.setId(id);
        userBo.setName("123");
        userBo.setUserType(userType);
        List<RoleBo> roles = new ArrayList<>();
        roles.add(new RoleBo());
        userBo.setRoles(roles);
        userBo.setPasswordVersion(0L);
        PowerMockito.when(token.getUser()).thenReturn(userBo);
        return token;
    }
}