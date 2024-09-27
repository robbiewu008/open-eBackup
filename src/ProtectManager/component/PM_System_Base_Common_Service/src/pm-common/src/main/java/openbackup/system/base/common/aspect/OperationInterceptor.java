/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 */

package openbackup.system.base.common.aspect;

import openbackup.system.base.common.constants.TokenBo;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.util.Map;

/**
 * Operation Aspect
 *
 * @param <A> template type
 * @author l00272247
 * @since 2020-11-14
 */
public interface OperationInterceptor<A extends Annotation> {
    /**
     * get supported annotation type
     *
     * @return supported annotation type
     */
    Class<A> getSupportedAnnotationType();

    /**
     * intercept
     *
     * @param method method
     * @param annotation annotation
     * @param context    context
     * @param tokenBo    token bo
     */
    void intercept(Method method, A annotation, Map<String, Object> context, TokenBo tokenBo);
}
