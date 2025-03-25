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
package openbackup.system.base.common.aspect;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.annotation.HcsRouting;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.service.IpRuleService;

import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.aspectj.lang.JoinPoint;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.annotation.Pointcut;
import org.aspectj.lang.reflect.MethodSignature;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.net.URI;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * 功能描述 HcsRoutingAspect
 *
 */
@Aspect
@Component
@Slf4j
class HcsRoutingAspect {
    private static final int HTTP_DEFAULT_PORT = 80;

    private static final int HTTPS_DEFAULT_PORT = 443;

    private static final int ILLEGAL_PORT = -1;

    private static final Pattern ANNOTATION_PATTERN = Pattern.compile("^#\\{\\D*\\}");

    private static final String LEFT_BRACES = "#{";

    private static final String RIGHT_BRACES = "}";

    private static final String PUNCTUATION_MARK = ".";

    private static final String BACKSLASH_PLUS_DOT = "\\.";

    private static final String PORT = "port";

    private static final String EQUAL = "=";

    private static final String HTTPS = "https";

    private static final String HTTP = "http";

    @Autowired
    IpRuleService ipRuleService;

    /**
     * 访问外部方法切点 注解拦截
     */
    @Pointcut("@annotation(openbackup.system.base.common.annotation.HcsRouting)")
    public void routingOnMethod() {
    }

    /**
     * 访问外部方法切点 注解拦截
     */
    @Pointcut("within(@openbackup.system.base.common.annotation.HcsRouting *)")
    public void routingOnType() {
    }

    /**
     * 环绕通知，调用目标方法前添加路由，调用方法后删除路由
     *
     * @param joinPoint 接入点方法
     * @return 目标方法调用结果
     * @throws Throwable 异常
     */
    @Around("((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
        + "&& (routingOnMethod() || routingOnType())")
    public Object invokeMethodWithRoute(ProceedingJoinPoint joinPoint) throws Throwable {
        Object result = new Object();
        Optional<HcsRouting> annotation = getAnnotation(joinPoint);
        if (!annotation.isPresent()) {
            log.warn("Annotation not found, wrong use of annotation.");
            return joinPoint.proceed();
        }
        HcsRouting routing = annotation.get();
        String[] requires = routing.requires();
        for (String condition : requires) {
            if (!isMeetingCondition(condition, joinPoint)) {
                return joinPoint.proceed();
            }
        }
        String taskType = routing.onNetPlane();
        String port = getAnnotationValue(joinPoint, routing.port());
        String destinationDomain = getAnnotationValue(joinPoint, routing.destinationDomain());
        log.info("Adding routing before method call, destinationDomain:{}, port:{}, taskType:{}.", destinationDomain,
            port, taskType);
        try {
            ipRuleService.addIpRule(destinationDomain, port, taskType);
            result = joinPoint.proceed();
        } finally {
            ipRuleService.deleteIpRule(destinationDomain, taskType);
        }
        return result;
    }

    /**
     * 从方法和类上获取注解，方法优先
     *
     * @param joinPoint joinPoint
     * @return 注解
     */
    private Optional<HcsRouting> getAnnotation(JoinPoint joinPoint) {
        Optional<HcsRouting> methodAnnotation = getMethodAnnotation(joinPoint);
        if (methodAnnotation.isPresent()) {
            return methodAnnotation;
        }
        return getTypeAnnotation(joinPoint);
    }

    private Optional<HcsRouting> getMethodAnnotation(JoinPoint joinPoint) {
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            return Optional.ofNullable(signature.getMethod().getAnnotation(HcsRouting.class));
        }
        return Optional.empty();
    }

    private Optional<HcsRouting> getTypeAnnotation(JoinPoint joinPoint) {
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            Class<?> declaringType = signature.getDeclaringType();
            return Optional.ofNullable(declaringType.getAnnotation(HcsRouting.class));
        }
        return Optional.empty();
    }

    /**
     * 参看方法参数是否满足注解要求，满足才执行切面逻辑
     *
     * @param condition 注解要求
     * @param joinPoint joinPoint
     * @return 是否满足
     */
    private boolean isMeetingCondition(String condition, ProceedingJoinPoint joinPoint) {
        String[] conditionArray = condition.split(EQUAL);
        if (conditionArray.length != IsmNumberConstant.TWO) {
            return false;
        }
        String paramValue = getAnnotationValue(joinPoint, conditionArray[0]);
        return paramValue.equals(conditionArray[1]);
    }

    /**
     * 获取注解中传递的动态参数的参数值
     *
     * @param joinPoint 切入点
     * @param name 参数名
     * @return 注解中传递的动态参数的参数值
     */
    private String getAnnotationValue(JoinPoint joinPoint, String name) {
        String paramName = name;

        // 获取方法中所有的参数
        Map<String, Object> params = getParams(joinPoint);

        // 参数是否是动态的:#{paramName}
        Matcher matcher = ANNOTATION_PATTERN.matcher(paramName);
        if (matcher.matches()) {
            // 获取参数名
            paramName = paramName.replace(LEFT_BRACES, StringUtils.EMPTY).replace(RIGHT_BRACES, StringUtils.EMPTY);

            // 是否是复杂的参数类型:对象.参数名  eg(paramName:"uri.host",split为["uri","host"])
            if (paramName.contains(PUNCTUATION_MARK)) {
                String[] split = paramName.split(BACKSLASH_PLUS_DOT);

                // 获取方法中对象的内容
                Object object = getValue(params, split[0]).orElse(new Object());
                return getFiledValue(Arrays.copyOfRange(split, 1, split.length), object);
            }

            // 简单的动态参数直接返回
            return String.valueOf(getValue(params, paramName).orElse(new Object()));
        }

        // 非动态参数直接返回
        return name;
    }

    private String getFiledValue(String[] fieldChain, Object rootObj) {
        if (fieldChain.length < 1) {
            return Strings.EMPTY;
        }
        if (rootObj instanceof URI) {
            return getUriValue(fieldChain, (URI) rootObj);
        }
        JSONObject curRoot = JSONObject.fromObject(rootObj);
        for (int index = 0; index < fieldChain.length - 1; index++) {
            if (!curRoot.containsKey(fieldChain[index])) {
                return Strings.EMPTY;
            }
            curRoot = curRoot.getJSONObject(fieldChain[index]);
        }
        return String.valueOf(curRoot.get(fieldChain[fieldChain.length - 1]));
    }

    private String getUriValue(String[] fieldChain, URI uri) {
        String uriFiled = fieldChain[0];
        if (PORT.equals(uriFiled)) {
            // 若uri中没有获取到port，https请求返回默认端口443，http请求返回默认端口80
            int port = uri.getPort();
            if (port == ILLEGAL_PORT) {
                String protocol = uri.getScheme();
                if (HTTPS.equals(protocol)) {
                    return String.valueOf(HTTPS_DEFAULT_PORT);
                }
                if (HTTP.equals(protocol)) {
                    return String.valueOf(HTTP_DEFAULT_PORT);
                }
            }
            return String.valueOf(port);
        }
        return uri.getHost();
    }

    /**
     * 根据参数名返回对应的值
     *
     * @param map 切入点方法的参数名,参数映射
     * @param paramName 参数名
     * @return 参数名返回对应的值
     */
    private Optional<Object> getValue(Map<String, Object> map, String paramName) {
        for (Map.Entry<String, Object> entry : map.entrySet()) {
            if (entry.getKey().equals(paramName)) {
                return Optional.ofNullable(entry.getValue());
            }
        }
        return Optional.empty();
    }

    /**
     * 获取方法的参数名和值
     *
     * @param joinPoint 切入点
     * @return 方法的参数名和值
     */
    private Map<String, Object> getParams(JoinPoint joinPoint) {
        Map<String, Object> params = new HashMap<>();
        List<Object> args = Arrays.asList(joinPoint.getArgs());
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            List<String> names = Arrays.asList(signature.getParameterNames());
            for (int index = 0; index < args.size(); index++) {
                params.put(names.get(index), args.get(index));
            }
            return params;
        }
        log.error("Annotation type: {} error.", joinPoint.getSignature().getName());
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Annotation type error.");
    }
}
