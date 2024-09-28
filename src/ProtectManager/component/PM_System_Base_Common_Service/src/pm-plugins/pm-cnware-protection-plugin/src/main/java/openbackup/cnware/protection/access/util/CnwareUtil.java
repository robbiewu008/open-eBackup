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
package openbackup.cnware.protection.access.util;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * CNware工具类
 *
 */
@Slf4j
public class CnwareUtil {
    /**
     * 检查endpoint合法性
     *
     * @param endpoint endpoint
     */
    public static void checkEndpoint(String endpoint) {
        // 校验是否是ip，并校验是否通过ipv6和ipv4的正则校验
        if (StringUtils.isBlank(endpoint) || !Pattern.matches(RegexpConstants.IP_V4V6_ADDRESS, endpoint)) {
            // 如果不是ip，校验是否为合法domain
            Pattern pattern = Pattern.compile(CnwareConstant.DOMAIN_REGEX);
            Matcher matcher = pattern.matcher(endpoint);
            if (!matcher.matches() || endpoint.length() > CnwareConstant.CNWARE_DOMAINNAME_MAX_LENGTH) {
                log.error("CNware check endpoint({}), param is invalid!", endpoint);
                throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid CNware param endpoint.");
            }
        }
    }

    /**
     * 检查端口合法性
     *
     * @param port 端口
     */
    public static void checkPort(int port) {
        int minPort = LegoNumberConstant.ZERO;
        if (port < minPort || port > CnwareConstant.MAX_PORT) {
            log.error("CNware check port({}), param is invalid!", port);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid CNware param port.");
        }
    }

    /**
     * 检查环境名称
     *
     * @param name 环境名称
     */
    public static void verifyEnvName(String name) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(name);
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(name);
    }
}
