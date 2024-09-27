/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * @author t30049904
 * @version [OceanProtect DataBack 1.5.0]
 * @since 2024/1/2
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
