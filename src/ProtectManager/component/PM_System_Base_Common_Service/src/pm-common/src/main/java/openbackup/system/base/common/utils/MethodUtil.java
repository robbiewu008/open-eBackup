/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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
