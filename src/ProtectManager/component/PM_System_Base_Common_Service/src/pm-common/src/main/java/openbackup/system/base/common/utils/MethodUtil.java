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

import org.springframework.core.MethodParameter;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

/**
 * method util
 *
 * @author l00272247
 * @since 2019-11-14
 */
public class MethodUtil {
    /**
     * constrcutor
     */
    protected MethodUtil() {
    }

    /**
     * get method parameters
     *
     * @param method method
     * @return method parameters
     */
    public static List<MethodParameter> getMethodParameters(Method method) {
        List<MethodParameter> parameters = new ArrayList<>();
        for (int index = 0, count = method.getParameterCount(); index < count; index++) {
            parameters.add(new MethodParameter(method, index));
        }
        return parameters;
    }
}
