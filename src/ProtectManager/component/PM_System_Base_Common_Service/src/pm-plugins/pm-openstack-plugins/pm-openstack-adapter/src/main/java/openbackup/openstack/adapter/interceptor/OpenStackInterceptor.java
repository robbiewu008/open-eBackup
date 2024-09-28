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
package openbackup.openstack.adapter.interceptor;

import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.service.OpenStackUserManager;
import openbackup.openstack.protection.access.keystone.KeyStoneService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;
import org.springframework.web.servlet.HandlerInterceptor;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

/**
 * OpenStack北向接口Web拦截器
 *
 */
@Slf4j
@Component
public class OpenStackInterceptor implements HandlerInterceptor {
    private static final String X_AUTH_TOKEN = "X-Auth-Token";

    private final OpenStackUserManager userManager;
    private final KeyStoneService keyStoneService;

    public OpenStackInterceptor(OpenStackUserManager userManager, KeyStoneService keyStoneService) {
        this.userManager = userManager;
        this.keyStoneService = keyStoneService;
    }

    @Override
    public boolean preHandle(HttpServletRequest request, HttpServletResponse response, Object handler)
        throws Exception {
        userManager.setTokenToRequest();
        // 校验token
        String projectId = verifyToken(request);
        request.setAttribute(OpenStackConstants.PROJECT_ID, projectId);

        log.info("Openstack verify success and get project id: {}", projectId);
        return HandlerInterceptor.super.preHandle(request, response, handler);
    }

    private String verifyToken(HttpServletRequest request) {
        String token = request.getHeader(X_AUTH_TOKEN);
        if (StringUtils.isBlank(token)) {
            throw new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, "Token not exists.");
        }
        return keyStoneService.verifyProjectToken(token);
    }
}
