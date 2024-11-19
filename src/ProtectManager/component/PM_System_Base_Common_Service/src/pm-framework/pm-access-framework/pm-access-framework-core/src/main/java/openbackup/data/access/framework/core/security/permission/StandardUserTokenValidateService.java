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

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.aspect.IgnorePasswordValidityPeriodVerification;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.apache.commons.lang3.StringUtils;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.Signature;
import org.aspectj.lang.reflect.MethodSignature;
import org.redisson.api.RedissonClient;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.stereotype.Component;

import java.lang.reflect.Method;
import java.util.Optional;

/**
 * Standard User Token Validate Service
 *
 */
@Component
@Slf4j
public class StandardUserTokenValidateService implements UserTokenValidateService {
    private final AuthNativeApi authNativeApi;

    private final RedissonClient redissonClient;

    /**
     * 构造函数
     *
     * @param authNativeApi auth服务
     * @param redissonClient redisson
     */
    public StandardUserTokenValidateService(AuthNativeApi authNativeApi, RedissonClient redissonClient) {
        this.authNativeApi = authNativeApi;
        this.redissonClient = redissonClient;
    }

    @Override
    public void validate(ProceedingJoinPoint joinPoint, TokenBo token) {
        TokenBo.UserBo user = token.getUser();
        Optional<TokenBo.UserInfo> userInfo = authNativeApi.queryUserInfoById(user.getId());
        UserUtils.checkToken(user, userInfo);
        MethodSignature methodSignature;
        Signature signature = joinPoint.getSignature();
        if (signature instanceof MethodSignature) {
            methodSignature = (MethodSignature) signature;
        } else {
            throw new IllegalStateException();
        }

        Method method = methodSignature.getMethod();
        // Dorado用户校验的时候，用来告警统计，支持巡检功能，不用校验是否修改密码
        checkDeviceManagerUser(method, user);
    }

    @ExterAttack
    private void checkDeviceManagerUser(Method method, TokenBo.UserBo userBo) {
        if (UserUtils.isNeedValidatePasswordVersion(userBo.getUserType()) && !StringUtils.equals(
            String.valueOf(Constants.ROLE_DEVICE_MANAGER), userBo.getRoles().get(0).getId())) {
            // 检验是否修改密码
            IgnorePasswordValidityPeriodVerification ignore = AnnotatedElementUtils.findMergedAnnotation(method,
                IgnorePasswordValidityPeriodVerification.class);
            if (ignore == null && authNativeApi.queryUserInfoByName(userBo.getName()).isMustModifyPassword()) {
                // 抛出该异常的时候，前台会捕获该异常，然后跳转到登录接口并清除Cookie，
                // 后台也需要同时删除redis中用户相关的信息
                UserUtils.deleteUserCacheAndSessionInfo(redissonClient, userBo.getId());
                throw new LegoCheckedException(CommonErrorCode.PASSWORD_FIRST_MODIFY_NOTICE);
            }
        }
    }
}
