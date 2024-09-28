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
package openbackup.system.base.common.aspect;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExprUtil;
import openbackup.system.base.common.utils.RightsControl;
import openbackup.system.base.common.utils.UserUtils;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.util.DefaultRoleHelper;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

import javax.servlet.http.HttpServletRequest;

/**
 * rights control interceptor
 *
 */
@Slf4j
@Component
public class RightsControlInterceptor implements OperationInterceptor<RightsControl> {
    private static final String GET_REQUEST = "GET";

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private List<DomainBasedOwnershipVerifier> verifiers;

    @Autowired
    private AuthNativeApi authNativeApi;

    /**
     * get supported annotation type
     *
     * @return supported annotation type
     */
    @Override
    public Class<RightsControl> getSupportedAnnotationType() {
        return RightsControl.class;
    }

    /**
     * intercept
     *
     * @param method method
     * @param rightsControl annotation
     * @param context context
     * @param tokenBo token bo
     */
    @Override
    public void intercept(Method method, RightsControl rightsControl, Map<String, Object> context, TokenBo tokenBo) {
        if (tokenBo == null) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
        TokenBo.UserBo userBo = tokenBo.getUser();
        Optional<TokenBo.UserInfo> userInfo = authNativeApi.queryUserInfoById(tokenBo.getUser().getId());
        UserUtils.checkToken(userBo, userInfo);
        // Dorado用户校验的时候，用来告警统计，支持巡检功能，不用校验是否修改密码
        if (!StringUtils.equals(String.valueOf(Constants.ROLE_DEVICE_MANAGER), userBo.getRoles().get(0).getId())
            && UserUtils.isNeedValidatePasswordVersion(userBo.getUserType())) {
            // 检验是否修改密码
            IgnorePasswordValidityPeriodVerification ignore = AnnotatedElementUtils.findMergedAnnotation(method,
                IgnorePasswordValidityPeriodVerification.class);
            if (ignore == null && authNativeApi.queryUserInfoByName(userBo.getName()).isMustModifyPassword()) {
                // 抛出该异常的时候，前台会捕获该异常，然后跳转到登录接口并清除Cookie
                // 后台也需要同时删除redis中用户相关的信息
                UserUtils.deleteUserCacheAndSessionInfo(redissonClient, userBo.getId());
                throw new LegoCheckedException(CommonErrorCode.PASSWORD_FIRST_MODIFY_NOTICE);
            }
        }
        checkRoles(userBo, rightsControl.roles());
        checkResources(context, userBo, rightsControl.resources());
    }

    private void checkRoles(TokenBo.UserBo userBo, String[] roles) {
        if (roles.length == 0) {
            return;
        }
        String role = userBo.getRoles().get(0).getName();
        // 审计员角色具有系统只读权限
        if (isGetRequest() && hasRoleAuditor(userBo)) {
            return;
        }
        if (StringUtils.equals(UserTypeEnum.HCS.getValue(), userBo.getUserType())) {
            if (!userBo.isHcsUserManagePermission() && !isGetRequest()) {
                throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "hcs permission denied!");
            }
        }
        if (Arrays.stream(roles).noneMatch(item -> Objects.equals(item, role))) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED);
        }
    }

    private boolean hasRoleAuditor(TokenBo.UserBo userBo) {
        return userBo.getRoles().stream().anyMatch(roleBo -> Constants.Builtin.ROLE_AUDITOR.equals(roleBo.getName()));
    }

    private static boolean isGetRequest() {
        RequestAttributes requestAttributes = RequestContextHolder.getRequestAttributes();
        HttpServletRequest request;
        if (requestAttributes instanceof ServletRequestAttributes) {
            request = ((ServletRequestAttributes) requestAttributes).getRequest();
        } else {
            return false;
        }
        return GET_REQUEST.equalsIgnoreCase(request.getMethod());
    }

    private void checkResources(Map<String, Object> context, TokenBo.UserBo userBo, String[] resources) {
        Map<String, List<String>> resolvedResources = resolveResources(context, resources);
        for (Map.Entry<String, List<String>> entry : resolvedResources.entrySet()) {
            getDomainBasedVerifierByType(entry.getKey()).ifPresent(
                verifier -> verify(userBo, entry.getValue(), verifier));
        }
    }

    private void verify(TokenBo.UserBo userBo, List<String> resources, DomainBasedOwnershipVerifier verifier) {
        if (!DefaultRoleHelper.isAdminOrAudit(userBo.getId())) {
            verifier.verify(userBo, resources);
        }
    }

    private Optional<DomainBasedOwnershipVerifier> getDomainBasedVerifierByType(String type) {
        return verifiers.stream().filter(verifier -> Objects.equals(type, verifier.getType())).findFirst();
    }

    private Map<String, List<String>> resolveResources(Map<String, Object> context, String[] resources) {
        Map<String, List<String>> map = new HashMap<>();
        for (String resource : resources) {
            int index = resource.indexOf(":");
            if (index == -1) {
                continue;
            }
            String type = resource.substring(0, index);
            String[] parts = resource.substring(index + 1).split(",");
            List<Object> items = Arrays.stream(parts)
                .map(String::trim)
                .filter(item -> !item.isEmpty())
                .map(item -> ExprUtil.eval(context, item, false))
                .filter(Objects::nonNull)
                .collect(Collectors.toList());
            List<String> list = map.getOrDefault(type, new ArrayList<>());
            list.addAll(unfold(items));
            map.put(type, list);
        }
        return map;
    }

    private List<String> unfold(Collection items) {
        List<String> results = new ArrayList<>();
        for (Object item : items) {
            if (item instanceof Collection) {
                results.addAll(unfold((Collection) item));
            } else if (item != null) {
                results.add(item.toString());
            } else {
                results.addAll(Collections.emptyList());
            }
        }
        return results;
    }
}
