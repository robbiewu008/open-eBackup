package openbackup.system.base.common.aspect;

import openbackup.system.base.common.annotation.Routing;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.service.IpRuleService;

import lombok.extern.slf4j.Slf4j;

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
import java.util.stream.Collectors;

/**
 * 更新路由切面
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/2/27
 */
@Aspect
@Component
@Slf4j
public class RoutingAspect {
    private static final int INITIAL_CAPACITY = 8;

    private static final String ANNOTATION_PARAM = "^#\\{\\D*\\}";

    private static final String LEFT_BRACES = "#{";

    private static final String RIGHT_BRACES = "}";

    private static final String PUNCTUATION_MARK = ".";

    private static final String BACKSLASH_PLUS_DOT = "\\.";

    private static final String PORT = "port";

    @Autowired
    IpRuleService ipRuleService;

    /**
     * 访问外部方法切点 注解拦截
     */
    @Pointcut("@annotation(openbackup.system.base.common.annotation.Routing)")
    public void routingOnMethod() {
    }

    /**
     * 访问外部方法切点 注解拦截
     */
    @Pointcut("within(@openbackup.system.base.common.annotation.Routing *)")
    public void routingOnType() {
    }

    /**
     * 环绕通知，调用目标方法前添加路由，调用方法后删除路由
     *
     * @param joinPoint 接入点方法
     * @return 目标方法调用结果
     * @throws Throwable 异常
     */
    @Around(
        "((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
            + "&& (routingOnMethod() || routingOnType())")
    public Object invokeMethodWithRoute(ProceedingJoinPoint joinPoint) throws Throwable {
        Object result;
        Optional<Routing> annotation = getAnnotation(joinPoint);
        if (!annotation.isPresent()) {
            log.warn("annotation not found, wrong use of annotation");
            return joinPoint.proceed();
        }
        Routing routing = annotation.get();
        String[] requires = routing.requires();
        String taskType = routing.onNetPlane();
        for (String condition : requires) {
            if (!isMeetingCondition(condition, joinPoint)) {
                return joinPoint.proceed();
            }
        }
        String[] destinationIps = getAnnotationValue(joinPoint, routing.destinationIp()).split(",");

        // 只有x系列需要添加路由 agent通信和复制集群通信
        List<String> needAddingRouteIp =
            Arrays.stream(destinationIps).filter(ip -> !VerifyUtil.isEmpty(ip)).collect(Collectors.toList());
        log.info("Adding routing before method call, destinationIp:{}", needAddingRouteIp);
        try {
            needAddingRouteIp.forEach(ip -> ipRuleService.addIpRule(ip, taskType));
            result = joinPoint.proceed();
        } finally {
            log.info("Removing routing after method call, destinationIp:{}", Arrays.toString(destinationIps));
            needAddingRouteIp.forEach(ip -> ipRuleService.deleteIpRule(ip, taskType));
        }
        return result;
    }

    /**
     * 参看方法参数是否满足注解要求，满足才执行切面逻辑
     *
     * @param condition 注解要求
     * @param joinPoint joinPoint
     * @return 是否满足
     */
    private boolean isMeetingCondition(String condition, ProceedingJoinPoint joinPoint) {
        String[] conditionArray = condition.split("=");
        if (conditionArray.length != IsmNumberConstant.TWO) {
            return false;
        }
        String paramValue = getAnnotationValue(joinPoint, conditionArray[0]);
        return paramValue.equals(conditionArray[1]);
    }

    /**
     * 从方法和类上获取注解，方法优先
     *
     * @param joinPoint joinPoint
     * @return 注解
     */
    private Optional<Routing> getAnnotation(JoinPoint joinPoint) {
        Optional<Routing> methodAnnotation = getMethodAnnotation(joinPoint);
        if (methodAnnotation.isPresent()) {
            return methodAnnotation;
        }
        return getTypeAnnotation(joinPoint);
    }

    private Optional<Routing> getMethodAnnotation(JoinPoint joinPoint) {
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            return Optional.ofNullable(signature.getMethod().getAnnotation(Routing.class));
        }
        return Optional.empty();
    }

    private Optional<Routing> getTypeAnnotation(JoinPoint joinPoint) {
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            Class<?> declaringType = signature.getDeclaringType();
            return Optional.ofNullable(declaringType.getAnnotation(Routing.class));
        }
        return Optional.empty();
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
        if (paramName.matches(ANNOTATION_PARAM)) {
            // 获取参数名
            paramName = paramName.replace(LEFT_BRACES, StringUtils.EMPTY).replace(RIGHT_BRACES, StringUtils.EMPTY);

            // 是否是复杂的参数类型:对象.参数名
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
        for (int i = 0; i < fieldChain.length - 1; i++) {
            if (!curRoot.containsKey(fieldChain[i])) {
                return Strings.EMPTY;
            }
            curRoot = curRoot.getJSONObject(fieldChain[i]);
        }
        return String.valueOf(curRoot.get(fieldChain[fieldChain.length - 1]));
    }

    private String getUriValue(String[] fieldChain, URI uri) {
        String uriFiled = fieldChain[0];
        if (PORT.equals(uriFiled)) {
            return String.valueOf(uri.getPort());
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
        Map<String, Object> params = new HashMap<>(INITIAL_CAPACITY);
        List<Object> args = Arrays.asList(joinPoint.getArgs());
        if (joinPoint.getSignature() instanceof MethodSignature) {
            MethodSignature signature = (MethodSignature) joinPoint.getSignature();
            List<String> names = Arrays.asList(signature.getParameterNames());
            for (int i = 0; i < args.size(); i++) {
                params.put(names.get(i), args.get(i));
            }
            return params;
        }
        log.error("Annotation type: {} error.", joinPoint.getSignature().getName());
        throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Annotation type error.");
    }
}
