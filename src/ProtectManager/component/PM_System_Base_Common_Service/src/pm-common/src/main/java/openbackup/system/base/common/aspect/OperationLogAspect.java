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
import openbackup.system.base.common.annotation.ManualOperationLogging;
import openbackup.system.base.common.annotation.OperationContext;
import openbackup.system.base.common.annotation.OperationContexts;
import openbackup.system.base.common.annotation.OperationLogging;
import openbackup.system.base.common.constants.AspectOrderConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.log.OperationContextConfig;
import openbackup.system.base.common.log.OperationContextLoader;
import openbackup.system.base.common.log.utils.AnnotationUtil;
import openbackup.system.base.common.log.utils.TypeUtil;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.ExprUtil;
import openbackup.system.base.common.utils.MethodUtil;
import openbackup.system.base.common.utils.RequestUtil;
import openbackup.system.base.common.utils.RightsControl;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;

import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.Signature;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.aspectj.lang.reflect.MethodSignature;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.core.annotation.AnnotatedElementUtils;
import org.springframework.core.annotation.Order;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;
import org.springframework.web.method.HandlerMethod;
import org.springframework.web.servlet.mvc.method.annotation.RequestMappingHandlerMapping;

import java.lang.annotation.Annotation;
import java.lang.reflect.Method;
import java.time.Instant;
import java.util.AbstractMap;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.concurrent.CompletionStage;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import javax.annotation.PostConstruct;
import javax.servlet.http.HttpServletRequest;
import javax.validation.ConstraintViolationException;

/**
 * 操作日志切面
 *
 */
@Slf4j
@Aspect
@Component
@Order(AspectOrderConstant.OPERATION_LOG_ASPECT_ORDER)
public class OperationLogAspect {
    /**
     * TOKEN_BO
     */
    public static final String TOKEN_BO = "token_bo";

    /**
     * 当返回null, 界面显示为--
     */
    private static final String DISPLAY_NULL = "--";

    private final Map<Method, List<OperationContextConfig>> methodOperationContextConfigs = new HashMap<>();

    @Autowired
    private OperationLogService operationLogService;

    @Autowired
    private RequestMappingHandlerMapping requestMappingHandlerMapping;

    @Autowired
    private ApplicationContext applicationContext;

    @Autowired
    private List<OperationInterceptor<?>> operationInterceptors;

    @Autowired
    private TokenVerificationService tokenVerificationService;

    @Autowired
    private List<DataConverter> dataConverters;

    @Autowired
    private AuthNativeApi authNativeApi;

    /**
     * 加载controller上的配置和loader
     */
    @PostConstruct
    public void init() {
        this.loadConfig();
    }

    /**
     * 日志切面
     *
     * @param joinPoint 切点
     * @param operationLogging 注解类
     * @return 返回值
     * @throws Throwable throwable
     */
    @Around(value = "@annotation(operationLogging)", argNames = "joinPoint,operationLogging")
    public Object processManualOperationLogging(ProceedingJoinPoint joinPoint, ManualOperationLogging operationLogging)
            throws Throwable {
        return processRightsControl(joinPoint, null);
    }

    /**
     * 日志切面
     *
     * @param joinPoint 切点
     * @param rightsControl 注解类
     * @return 返回值
     * @throws Throwable throwable
     */
    @Around(value = "@annotation(rightsControl)", argNames = "joinPoint,rightsControl")
    public Object processRightsControl(ProceedingJoinPoint joinPoint, RightsControl rightsControl) throws Throwable {
        Signature signature = joinPoint.getSignature();
        MethodSignature methodSignature;
        if (signature instanceof MethodSignature) {
            methodSignature = (MethodSignature) signature;
        } else {
            throw new LegoCheckedException("logic error");
        }
        Method method = methodSignature.getMethod();
        OperationLogging operationLog = AnnotatedElementUtils.findMergedAnnotation(method, OperationLogging.class);
        List<Object> args = Arrays.asList(joinPoint.getArgs());
        Map<String, Object> contextParameters = loadAllOperationContexts(args, method);
        Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig;
        if (operationLog != null) {
            if (operationLog.name().isEmpty()) {
                throw new LegoCheckedException("operation log name is empty");
            }
            if (operationLog.target().isEmpty()) {
                throw new LegoCheckedException("operation log target is empty");
            }
            operationLogConfig = this.getOperationLogConfig(contextParameters, operationLog);
        } else {
            operationLogConfig = null;
        }
        RequestAttributes requestAttributes = Objects.requireNonNull(RequestContextHolder.getRequestAttributes());
        HttpServletRequest request;
        if (requestAttributes instanceof ServletRequestAttributes) {
            request = ((ServletRequestAttributes) requestAttributes).getRequest();
        } else {
            throw new LegoCheckedException("logic error");
        }
        try {
            TokenBo tokenBo = tokenVerificationService.parsingTokenFromRequest();
            intercept(method, contextParameters, tokenBo, rightsControl);
            request.setAttribute(TOKEN_BO, tokenBo);
            Object result = joinPoint.proceed(joinPoint.getArgs());
            this.recordOperation(request, operationLogConfig, result);
            return result;
        } catch (Exception e) {
            log.error("Execute failed", ExceptionUtil.getErrorMessage(e));
            throw handleFailure(operationLogConfig, request, e);
        }
    }

    private LegoCheckedException handleFailure(Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig,
            HttpServletRequest request, Exception error) {
        LegoCheckedException legoCheckedException = ExceptionUtil.lookFor(error, LegoCheckedException.class);
        if (legoCheckedException != null && legoCheckedException.getErrorCode() == CommonErrorCode.ACCESS_DENIED) {
            return legoCheckedException;
        }
        this.recordFailureOperationLog(request, operationLogConfig, null, error);
        if (legoCheckedException != null) {
            return legoCheckedException;
        } else {
            return new LegoCheckedException(error.getMessage(), error.getCause());
        }
    }

    private void intercept(Method method, Map<String, Object> context, TokenBo tokenBo, RightsControl rightsControl) {
        if (rightsControl == null) {
            return;
        }
        List<OperationInterceptor<RightsControl>> interceptors = findRightControlOperationInterceptorByAnnotationType();
        for (OperationInterceptor<RightsControl> interceptor : interceptors) {
            intercept(method, context, tokenBo, interceptor, rightsControl);
        }
        operationInterceptors.stream().filter(interceptor -> !interceptors.contains(interceptor))
                .forEach(interceptor -> intercept(method, context, tokenBo, interceptor));
    }

    private List<OperationInterceptor<RightsControl>> findRightControlOperationInterceptorByAnnotationType() {
        List<OperationInterceptor<RightsControl>> items = new ArrayList<>();
        for (OperationInterceptor operationInterceptor : operationInterceptors) {
            if (Objects.equals(operationInterceptor.getSupportedAnnotationType(), RightsControl.class)) {
                items.add(operationInterceptor);
            }
        }
        return items;
    }

    private <A extends Annotation> void intercept(Method method, Map<String, Object> context, TokenBo tokenBo,
            OperationInterceptor<A> operationInterceptor) {
        Class<A> annotationType = operationInterceptor.getSupportedAnnotationType();
        A annotation = AnnotatedElementUtils.findMergedAnnotation(method, annotationType);
        intercept(method, context, tokenBo, operationInterceptor, annotation);
    }

    private <A extends Annotation> void intercept(Method method, Map<String, Object> context, TokenBo tokenBo,
            OperationInterceptor<A> operationInterceptor, A annotation) {
        if (annotation != null) {
            operationInterceptor.intercept(method, annotation, context, tokenBo);
        }
    }

    private Map.Entry<OperationLogging, List<List<Evaluation>>> getOperationLogConfig(
            Map<String, Object> contextParameters, OperationLogging operationLog) {
        List<?> targets = this.eval(contextParameters, operationLog.target());
        List<List<Evaluation>> collections = evalDetails(contextParameters, operationLog);
        List<Evaluation> targetEvalList = targets.stream().filter(Evaluation.class::isInstance)
                .map(Evaluation.class::cast).collect(Collectors.toList());
        collections.add(0, targetEvalList);
        return new AbstractMap.SimpleEntry<>(operationLog, collections);
    }

    private List<List<Evaluation>> evalDetails(Map<String, Object> contextParameters, OperationLogging operationLog) {
        List<List<Evaluation>> collections = new ArrayList<>();
        for (String item : operationLog.detail()) {
            List<?> eval = this.eval(contextParameters, item);
            if (eval instanceof OperationLoggingList) {
                for (String value : (OperationLoggingList) eval) {
                    List<Evaluation> evaluation = new ArrayList<>();
                    evaluation.add(object -> value);
                    collections.add(evaluation);
                }
            } else {
                List<Evaluation> list = eval.stream().filter(Evaluation.class::isInstance).map(Evaluation.class::cast)
                        .collect(Collectors.toList());
                collections.add(list);
            }
        }
        return collections;
    }

    private List<?> eval(Map<String, Object> contextParameters, String expr) {
        if (expr == null) {
            return Collections.emptyList();
        }
        List<Evaluation> evaluations = new ArrayList<>();
        if (expr.startsWith("@")) {
            String value = expr.substring(1);
            evaluations.add(object -> value);
        } else if (expr.matches("^return\\b.*")) {
            evaluations.add(object -> evaluate(object, expr));
        } else {
            Object evalResult = evaluate(contextParameters, expr);
            if (evalResult instanceof OperationLoggingList) {
                return (OperationLoggingList) evalResult;
            }
            for (String value : arrayify(Optional.ofNullable(evalResult).map(Object::toString).orElse(DISPLAY_NULL))) {
                evaluations.add(object -> value);
            }
        }
        return evaluations;
    }

    private Object evaluate(Object contextParameters, String expr) {
        String[] items = expr.split("!");
        Object result = contextParameters;
        int count = 0;
        AtomicBoolean skip = new AtomicBoolean();
        for (String item : items) {
            if (result == null) {
                return null;
            }
            if (count++ == 0) {
                result = ExprUtil.eval(contextParameters, item, false);
                continue;
            }
            if (item.trim().isEmpty()) {
                skip.set(true);
                continue;
            }
            result = evaluateWithConverter(result, item, skip.get());
            skip.set(false);
        }
        return result;
    }

    private Object evaluateWithConverter(Object object, String item, boolean isOriginal) {
        int index = item.indexOf('.');
        String name;
        String suffix;
        if (index == -1) {
            name = item;
            suffix = null;
        } else {
            name = item.substring(0, index);
            suffix = item.substring(index + 1);
        }
        DataConverter converter = getDataConverterByName(name);
        Collection<?> arguments;
        Object result = object;
        if (isOriginal) {
            arguments = Collections.singletonList(result);
        } else {
            arguments = CollectionUtils.listify(result);
        }
        Collection<?> collection = converter.convert(arguments);
        if (isOriginal) {
            result = collection.stream().findAny().orElse(null);
        } else if (result instanceof Collection) {
            result = collection;
        } else {
            result = collection.stream().findAny().orElse(null);
        }
        if (suffix != null) {
            result = ExprUtil.eval(result, suffix, false);
        }
        return result;
    }

    private DataConverter getDataConverterByName(String name) {
        List<DataConverter> converters = dataConverters.stream()
                .filter(dataConverter -> name.equals(dataConverter.getName())).collect(Collectors.toList());
        if (converters.isEmpty()) {
            throw new LegoCheckedException("data converter missing. name: " + name);
        }
        if (converters.size() != 1) {
            throw new LegoCheckedException("data converter conflicted. name: " + name);
        }
        return converters.get(0);
    }

    private List<String> arrayify(Object object) {
        return CollectionUtils.listify(object).stream().map(StringUtil::stringify).collect(Collectors.toList());
    }

    private void loadConfig() {
        getAllRestOperationMethods()
                .forEach(method -> methodOperationContextConfigs.put(method, getAllOperationContextConfigs(method)));
    }

    private Stream<OperationContextConfig> getMethodOperationContextConfigStream(Method method) {
        return AnnotationUtil
                .getAnnotations(method, OperationContext.class, OperationContexts.class, OperationContexts::value)
                .stream().map(context -> new OperationContextConfig(method, context, null));
    }

    private Map<String, Object> loadAllOperationContexts(List<Object> arguments, Method method) {
        List<OperationContextLoader> loaders = this.getOperationContextLoaders();
        int index = 0;
        Map<String, Object> contextParameters = new HashMap<>();

        for (int length = arguments.size(); index < length; ++index) {
            contextParameters.put(Integer.toString(index), arguments.get(index));
        }
        List<OperationContextConfig> configs = Optional.ofNullable(methodOperationContextConfigs.get(method))
                .orElse(Collections.emptyList());
        for (OperationContextConfig config : configs) {
            config.load(loaders, contextParameters, arguments);
        }
        return contextParameters;
    }

    private List<OperationContextLoader> getOperationContextLoaders() {
        return new ArrayList<>(this.applicationContext.getBeansOfType(OperationContextLoader.class).values());
    }

    private List<OperationContextConfig> getAllOperationContextConfigs(Method method) {
        List<OperationContextConfig> configs = Stream
                .concat(getMethodOperationContextConfigStream(method), getParameterOperationContextConfigs(method))
                .collect(Collectors.toList());

        Map<String, Long> counts = configs.stream().filter(config -> config.getName() != null)
                .collect(Collectors.groupingBy(OperationContextConfig::getName, Collectors.counting()));
        List<String> conflicts = counts.entrySet().stream().filter(entry -> entry.getValue() > 1).map(Map.Entry::getKey)
                .collect(Collectors.toList());
        if (!conflicts.isEmpty()) {
            throw new LegoCheckedException("context name conflicts. names: " + conflicts + ", method: " + method);
        }

        configs.forEach(config -> config.initialize(configs));
        Collections.sort(configs);
        return configs;
    }

    private List<Method> getAllRestOperationMethods() {
        return this.requestMappingHandlerMapping.getHandlerMethods().values().stream().map(HandlerMethod::getMethod)
                .filter(TypeUtil::isRestOperationMethod).collect(Collectors.toList());
    }

    private Stream<OperationContextConfig> getParameterOperationContextConfigs(Method method) {
        return MethodUtil.getMethodParameters(method).stream().map(parameter -> new OperationContextConfig(method,
                parameter.getParameterAnnotation(OperationContext.class), parameter));
    }

    private void recordOperation(HttpServletRequest request,
            Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig, Object result) {
        if (operationLogConfig == null) {
            return;
        }
        if (result instanceof CompletionStage) {
            CompletionStage<?> stage = (CompletionStage) result;
            stage.whenComplete((data, error) -> recordAsyncOperation(request, operationLogConfig, data, error));
        }
        recordSuccessOperationLog(request, operationLogConfig, result);
    }

    private void recordAsyncOperation(HttpServletRequest request,
            Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig, Object data, Throwable error) {
        if (error == null) {
            recordSuccessOperationLog(request, operationLogConfig, data);
        } else {
            recordFailureOperationLog(request, operationLogConfig, data, error);
        }
    }

    private void recordSuccessOperationLog(HttpServletRequest request,
            Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig, Object result) {
        recordOperationLog(request, operationLogConfig, result, true, null);
    }

    private void recordFailureOperationLog(HttpServletRequest request,
            Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig, Object result,
            Throwable exception) {
        MethodArgumentNotValidException methodArgumentNotValidException = ExceptionUtil.lookFor(exception,
                MethodArgumentNotValidException.class);
        if (methodArgumentNotValidException != null) {
            throw LegoCheckedException.rethrow(methodArgumentNotValidException);
        }
        ConstraintViolationException constraintViolationException = ExceptionUtil.lookFor(exception,
                ConstraintViolationException.class);
        if (constraintViolationException != null) {
            throw constraintViolationException;
        }
        LegoCheckedException legoCheckedException = ExceptionUtil.lookFor(exception, LegoCheckedException.class);
        if (legoCheckedException != null) {
            recordOperationLog(request, operationLogConfig, result, false, legoCheckedException);
            throw legoCheckedException;
        } else {
            recordOperationLog(request, operationLogConfig, result, false, null);
            throw LegoCheckedException.rethrow(exception);
        }
    }

    private void recordOperationLog(HttpServletRequest request,
            Map.Entry<OperationLogging, List<List<Evaluation>>> operationLogConfig, Object result, boolean isSuccess,
            LegoCheckedException exception) {
        if (operationLogConfig == null) {
            return;
        }
        OperationLogging logging = operationLogConfig.getKey();
        List<List<Object>> collections = evaluate(operationLogConfig.getValue(), result);
        List<Object> targets = collections.get(0);
        for (int i = 0; i < targets.size(); i++) {
            List<String> details = new ArrayList<>();
            for (int j = 1; j < collections.size(); j++) {
                List<Object> list = collections.get(j);
                String detail;
                if (i < list.size() && list.get(i) instanceof OperationLoggingList) {
                    OperationLoggingList loggingList = (OperationLoggingList) list.get(i);
                    loggingList.forEach(item -> details.add(item != null ? item : DISPLAY_NULL));
                    continue;
                } else {
                    detail = i < list.size() ? list.get(i).toString() : null;
                }
                details.add(detail != null ? detail : DISPLAY_NULL);
            }
            String target = targets.get(i).toString();
            LogParam logParam = new LogParam(target, details.toArray(new String[0]), isSuccess, exception);
            sendEvent(request, logging, logParam);
        }
    }

    private List<List<Object>> evaluate(List<List<Evaluation>> collections, Object result) {
        List<List<Object>> results = new ArrayList<>();
        Map<String, Object> data = Collections.singletonMap("return", result);
        for (List<Evaluation> evaluations : collections) {
            List<Object> items = new ArrayList<>();
            for (Evaluation evaluation : evaluations) {
                Object evalResult = evaluation.evaluate(data);
                if (evalResult instanceof OperationLoggingList) {
                    items.add(evalResult);
                } else {
                    items.addAll(arrayify(evalResult));
                }
            }
            results.add(items);
        }
        return results;
    }

    private String getUserIdFromAuthToken() {
        TokenBo token = tokenVerificationService.parsingTokenFromRequest();
        return token.getUser().getId();
    }

    private void sendEvent(HttpServletRequest request, OperationLogging logging, LogParam param) {
        String[] params;
        if (!logging.manual()) {
            params = new String[param.getDetails().length + IsmNumberConstant.TWO];
            params[0] = getUsernameFromAuthToken();
            params[1] = RequestUtil.getClientIpAddress(request);
            System.arraycopy(param.getDetails(), 0, params, IsmNumberConstant.TWO, param.getDetails().length);
        } else {
            params = param.getDetails();
        }
        LegoInternalEvent event = new LegoInternalEvent();
        String sourceType = param.getTarget().toLowerCase(Locale.ENGLISH);
        event.setSourceType("operation_target_" + sourceType + "_label");
        event.setEventLevel(logging.rank());
        event.setMoName(logging.name());
        event.setMoIP(RequestUtil.getClientIpAddress(request));
        event.setEventParam(params);
        event.setEventId(logging.name());
        event.setSourceId(logging.name());
        event.setEventTime(Instant.now().getEpochSecond());
        event.setEventSequence(Instant.now().getNano());
        event.setIsSuccess(param.isSuccess());
        if (!logging.manual()) {
            event.setUserId(getUserIdFromAuthToken());
        } else {
            Optional<String> userIdOpt = getUserIdByUsername(params[0]);
            userIdOpt.ifPresent(event::setUserId);
        }
        if (Objects.nonNull(param.getLegoCheckedException())) {
            event.setLegoErrorCode(String.valueOf(param.getLegoCheckedException().getErrorCode()));
        }
        operationLogService.syncSendEvent(event, logging.isSync());
    }

    private Optional<String> getUserIdByUsername(String username) {
        try {
            TokenBo.UserInfo userInfo = authNativeApi.queryUserInfoByName(username);
            return Optional.ofNullable(userInfo.getId());
        } catch (Exception e) {
            return Optional.empty();
        }
    }

    private String getUsernameFromAuthToken() {
        TokenBo token = tokenVerificationService.parsingTokenFromRequest();
        return token.getUser().getName();
    }
}
