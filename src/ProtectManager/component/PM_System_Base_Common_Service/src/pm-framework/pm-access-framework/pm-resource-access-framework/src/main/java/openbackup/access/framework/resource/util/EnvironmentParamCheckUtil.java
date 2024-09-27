/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.access.framework.resource.util;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.network.AddressUtil;
import openbackup.system.base.common.utils.network.Ipv4AddressUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;

import java.util.regex.Pattern;

/**
 * 检查环境名称工具类
 *
 * @author x30038064
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/12/13
 */
@Slf4j
public class EnvironmentParamCheckUtil {
    /**
     * 检查环境名称不为空
     *
     * @param name 环境名称
     */
    public static void checkEnvironmentNameEmpty(String name) {
        if (StringUtils.isEmpty(name)) {
            log.error("checkEnvironmentNamePattern name: {} isEmpty.", name);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "environment name is empty");
        }
    }

    /**
     * 检查环境名称是否满足校验条件
     *
     * @param name 环境名称
     */
    public static void checkEnvironmentNamePattern(String name) {
        if (!Pattern.compile(RegexpConstants.ENV_NAME_PATTERN).matcher(name).matches()
            || name.length() > RegexpConstants.STRING_LENGTH_64) {
            log.error("checkEnvironmentNamePattern name: {} is not correct.", name);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "environment name is not pattern");
        }
    }

    /**
     * 校验ip是否有效
     *
     * @param ip ip地址
     */
    public static void checkValidIP(String ip) {
        boolean isValid = Ipv4AddressUtil.isValidIpv4All(ip) || AddressUtil.isValidIPv6Only(ip);
        if (!isValid) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "endpoint is not valid");
        }
    }
}
