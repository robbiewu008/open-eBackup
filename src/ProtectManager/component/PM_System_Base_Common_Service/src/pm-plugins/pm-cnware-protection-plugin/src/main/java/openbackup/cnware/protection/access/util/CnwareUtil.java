/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-11
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
