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
package openbackup.system.base.util;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.PMModuleEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;

/**
 * VariableParameterService实现类
 *
 */
@Slf4j
public class VariableParameterUtil {
    /**
     * 获取configmap（pm-variable-parameter-conf）对应的参数，若没有则为默认值
     *
     * @param moduleEnum 模块名称
     * @param paramName 参数名称
     * @param defaultValue 默认值
     * @return 对应参数的值
     */
    public static <T> T getConfigValueOrDefault(PMModuleEnum moduleEnum, String paramName, T defaultValue) {
        preCheck(moduleEnum, paramName, defaultValue);
        String correctParamName = moduleEnum.getModuleName() + "_" + paramName;
        if (!PMModuleEnum.getAllRelatedKeys().contains(correctParamName)) {
            log.info("The current field: {} is not in the trustlist", correctParamName);
            return defaultValue;
        }
        String valueInConfigMap = ConfigMapUtil.getValueInConfigMap(ConfigMapUtil.VARIABLE_PARAMETER_CONF,
            correctParamName);
        // 如果配置中没有该参数，直接返回默认值
        if (VerifyUtil.isEmpty(valueInConfigMap)) {
            return defaultValue;
        }
        try {
            if (defaultValue instanceof String) {
                return (T) valueInConfigMap;
            } else if (defaultValue instanceof Integer) {
                return (T) Integer.valueOf(valueInConfigMap);
            } else if (defaultValue instanceof Boolean) {
                return (T) Boolean.valueOf(valueInConfigMap);
            } else if (defaultValue instanceof Long) {
                return (T) Long.valueOf(valueInConfigMap);
            } else if (defaultValue instanceof Float) {
                return (T) Float.valueOf(valueInConfigMap);
            } else if (defaultValue instanceof Double) {
                return (T) Double.valueOf(valueInConfigMap);
            } else if (defaultValue instanceof Character) {
                return (T) Character.valueOf(valueInConfigMap.charAt(0));
            } else {
                log.error("Unsupported type: {}", defaultValue.getClass().getName());
                return defaultValue;
            }
        } catch (Exception e) {
            log.error("Failed to convert valueInConfigMap: {} to type: {}", valueInConfigMap,
                defaultValue.getClass().getName(), e);
            return defaultValue;
        }
    }

    private static <T> void preCheck(PMModuleEnum moduleEnum, String paramName, T defaultValue) {
        if (VerifyUtil.isEmpty(moduleEnum)) {
            log.error("The PMModuleEnum is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        if (VerifyUtil.isEmpty(paramName)) {
            log.error("The paramName is empty.");
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
        // 如果 defaultValue 为空，直接抛出异常
        if (defaultValue == null) {
            log.error("The default parameter is empty for param: {}", paramName);
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM);
        }
    }
}
