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

import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.security.permission.TrustClient;
import openbackup.system.base.service.DeployTypeService;

import com.google.common.collect.ImmutableSet;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.util.Set;

import javax.servlet.http.HttpServletRequest;

/**
 * Trust aspect
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-29
 */
@Aspect
@Component
@Slf4j
public class TrustAspect {
    private static final Set<DeployTypeEnum> NEED_CHECK_CLIENT_TRUST_DEPLOY_TYPE =
        ImmutableSet.of(DeployTypeEnum.X9000, DeployTypeEnum.X8000, DeployTypeEnum.X6000);

    private ResourceService resourceService;

    private final DeployTypeService deployTypeService;

    private DeployTypeEnum deployType;

    public TrustAspect(ResourceService resourceService, DeployTypeService deployTypeService) {
        this.resourceService = resourceService;
        this.deployTypeService = deployTypeService;
    }

    /**
     * aspect trust
     *
     * @param joinPoint join point
     * @param trustClient access
     * @return result
     * @throws Throwable throwable
     */
    @ExterAttack
    @Around(value = "@annotation(trustClient)", argNames = "joinPoint,trustClient")
    public Object checkClientIpTrust(ProceedingJoinPoint joinPoint, TrustClient trustClient) throws Throwable {
        String clientIp = getClientIp();
        log.info("client ip is {}", clientIp);
        if (needCheckTrust() && StringUtils.isNotBlank(clientIp)) {
            resourceService.checkHostIfBeTrustedByEndpoint(clientIp, true);
        }
        return joinPoint.proceed();
    }

    // x-forwarded-for格式：客户端 x-forwarded-for, client ip, pod id，倒数第二个ip就是客户端ip
    private String getClientIp() {
        HttpServletRequest request = null;
        final RequestAttributes requestAttributes = RequestContextHolder.currentRequestAttributes();
        if (requestAttributes instanceof ServletRequestAttributes) {
            request = ((ServletRequestAttributes) requestAttributes).getRequest();
        }

        String ip = request.getHeader("x-forwarded-for");
        log.info("get client ip. x-forwarded-for is {}, remote addr:{}", ip, request.getRemoteAddr());
        if (StringUtils.isBlank(ip) || "unknown".equalsIgnoreCase(ip)) {
            return StringUtils.EMPTY;
        }
        String[] ipList = ip.split(",");
        final int ipCount = ipList.length;
        if (ipCount > 1) {
            return ipList[ipCount - 2];
        }
        return StringUtils.EMPTY;
    }

    private boolean needCheckTrust() {
        if (deployType == null) {
            deployType = deployTypeService.getDeployType();
        }
        if (NEED_CHECK_CLIENT_TRUST_DEPLOY_TYPE.contains(deployType)) {
            return true;
        }
        return false;
    }
}
