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
package openbackup.system.base.controller;

import openbackup.system.base.controller.validator.BaseParamValidator;
import openbackup.system.base.util.SpringBeanUtils;

import org.apache.poi.ss.formula.functions.T;
import org.springframework.web.bind.WebDataBinder;
import org.springframework.web.bind.annotation.InitBinder;

import java.util.HashMap;
import java.util.Map;

/**
 * 基本的controller校验器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/3/5
 **/
public class BaseValidatorController {
    private static final HashMap<Class<?>, BaseParamValidator<T>> VALIDATOR_MAP = new HashMap<>();

    /**
     * 初始化绑定器，次数用户绑定参数的校验器
     *
     * @param binder 框架中的绑定器
     */
    @InitBinder
    public void initBinder(WebDataBinder binder) {
        if (binder.getTarget() == null) {
            return;
        }
        final Class<?> targetClass = binder.getTarget().getClass();
        if (!VALIDATOR_MAP.containsKey(targetClass)) {
            final Map<String, BaseParamValidator> map = SpringBeanUtils.getBeansByClass(BaseParamValidator.class);
            map.forEach(
                    (key, value) -> {
                        if (value.supports(targetClass)) {
                            VALIDATOR_MAP.put(targetClass, value);
                        }
                    });
        }
        if (VALIDATOR_MAP.containsKey(targetClass)) {
            binder.setValidator(VALIDATOR_MAP.get(targetClass));
        }
    }
}
