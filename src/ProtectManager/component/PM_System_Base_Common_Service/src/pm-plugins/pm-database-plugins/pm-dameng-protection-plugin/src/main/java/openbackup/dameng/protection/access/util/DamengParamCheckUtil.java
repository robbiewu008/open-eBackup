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
package openbackup.dameng.protection.access.util;

import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.validator.constants.RegexpConstants;

import lombok.extern.slf4j.Slf4j;

import java.util.regex.Pattern;

/**
 * dameng参数校验工具类
 *
 */
@Slf4j
public class DamengParamCheckUtil {
    /**
     * 校验前端传入的端口
     *
     * @param port 端口
     */
    public static void checkPort(String port) {
        if (VerifyUtil.isEmpty(port)) {
            return;
        }
        try {
            int intPort = Integer.parseInt(port);
            if (intPort < LegoNumberConstant.ZERO || intPort > DamengConstant.DAMENG_NODE_MAX_PORT) {
                log.error("check Dameng database port : {} is not illegal.", port);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Dameng port is illegal");
            }
        } catch (NumberFormatException e) {
            log.error("check Dameng database port : {} is not illegal.", port);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Dameng port is not integer");
        }
    }

    /**
     * 校验db端口
     *
     * @param port 端口
     */
    public static void checkRestoreDbPort(String port) {
        if (VerifyUtil.isEmpty(port)) {
            return;
        }
        try {
            int intPort = Integer.parseInt(port);
            if (intPort < LegoNumberConstant.THROUND_TWENTY_FOUR || intPort >= DamengConstant.DAMENG_NODE_MAX_PORT) {
                log.error("check Dameng database port : {} is not illegal.", port);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Dameng port is illegal");
            }
        } catch (NumberFormatException e) {
            log.error("check Dameng database port : {} is not illegal.", port);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Dameng port is not integer");
        }
    }

    /**
     * 校验前端传入的数据库用户名
     *
     * @param databaseUsername 数据库用户名
     */
    public static void checkAuthKey(String databaseUsername) {
        if (VerifyUtil.isEmpty(databaseUsername)) {
            return;
        }
        if (!Pattern.compile(RegexpConstants.ENV_NAME_PATTERN).matcher(databaseUsername).matches()
            || databaseUsername.length() > RegexpConstants.STRING_LENGTH_64) {
            log.error("check Dameng database username pattern: {} is not correct.", databaseUsername);
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "Dameng database username is not pattern");
        }
    }
}
