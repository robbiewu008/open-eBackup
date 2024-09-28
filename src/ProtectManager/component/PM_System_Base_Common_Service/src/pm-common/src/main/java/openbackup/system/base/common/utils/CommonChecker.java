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
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.regex.Pattern;

/**
 *  检查ip合法性
 *
 */
@Slf4j
public class CommonChecker {
    private static final int MAX_PORT = 65535;

    /**
     * 检查ip合法性
     *
     * @param envName     环境名称
     * @param ip          ip
     */
    public static void checkIp(String envName, String ip) {
        if (StringUtils.isBlank(ip) || !Pattern.matches(RegexpConstants.IP_V4V6_ADDRESS, ip)) {
            log.error("param ip is invalid, env:{}, ip:{}", envName, ip);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid param ip.");
        }
    }

    /**
     * 检查端口合法性
     *
     * @param envName     环境名称
     * @param port        端口
     */
    public static void checkPort(String envName, int port) {
        int minPort = LegoNumberConstant.ZERO;
        if (port < minPort || port > MAX_PORT) {
            log.error("param port is invalid, env:{}, port:{}", envName, port);
            throw new LegoCheckedException(CommonErrorCode.IP_PORT_ERROR, "Invalid param port.");
        }
    }
}
