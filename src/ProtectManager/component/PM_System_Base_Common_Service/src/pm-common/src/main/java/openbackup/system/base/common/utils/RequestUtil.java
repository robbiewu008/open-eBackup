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
package openbackup.system.base.common.utils;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.PlatformEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.CreateUserLogParam;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;
import openbackup.system.base.service.DeployTypeService;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.util.Objects;

import javax.servlet.http.HttpServletRequest;

/**
 * Http Request 工具类，封装一些Request的公共工具方法
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-05
 */
@Component
@CalleeMethods(name = "request_util", value = {
    @CalleeMethod(name = "getUserName"), @CalleeMethod(name = "getIpAddress"),
    @CalleeMethod(name = "getDefaultCreateUserLogParam")
})
public class RequestUtil {
    /**
     * client-ip
     */
    public static final String CLIENT_IP = "client-ip";

    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private DeployTypeService deployTypeService;

    private RequestUtil() {
    }

    /**
     * 获取发起请求的IP地址
     *
     * @param request HTTP请求
     * @return String 发起请求的IP地址
     */
    public static String getClientIpAddress(HttpServletRequest request) {
        return getIp(request);
    }

    private static String getIp(HttpServletRequest request) {
        String clientIp = request.getHeader(CLIENT_IP);
        if (StringUtils.isNotBlank(clientIp)) {
            return clientIp;
        }
        String ip = request.getHeader("x-forwarded-for");
        if (StringUtils.isNotBlank(ip) && !"unknown".equalsIgnoreCase(ip)) {
            int index = ip.indexOf(",");
            if (index > 0) {
                ip = ip.substring(0, index);
            }
        } else {
            ip = request.getRemoteAddr();
        }
        return ip;
    }

    /**
     * getIpAddress
     *
     * @return String
     */
    public String getIpAddress() {
        final RequestAttributes requestAttributes = Objects.requireNonNull(RequestContextHolder.getRequestAttributes());
        if (!(requestAttributes instanceof ServletRequestAttributes)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "requestAttributes is not instance of ServletRequestAttributes");
        }
        HttpServletRequest request = ((ServletRequestAttributes) requestAttributes).getRequest();
        return getIp(request);
    }

    /**
     * 获取发起请求的用户名
     *
     * @return String 发起请求的用户名
     */
    public String getUserName() {
        TokenBo token = tokenVerificationService.parsingTokenFromRequest();
        return token.getUser().getName();
    }

    /**
     * 获取CreateUserLogParam
     *
     * @return createUserLogParam CreateUserLogParam
     */
    public CreateUserLogParam getDefaultCreateUserLogParam() {
        CreateUserLogParam createUserLogParam = new CreateUserLogParam();
        String platForm = PlatformEnum.OCEAN_PROTECT.getName();
        if (deployTypeService.isCyberEngine()) {
            platForm = PlatformEnum.CYBER_ENGINE.getName();
        }
        createUserLogParam.setPlatform(platForm);
        createUserLogParam.setUserName(getUserName());
        createUserLogParam.setIp(getIpAddress());
        return createUserLogParam;
    }
}
