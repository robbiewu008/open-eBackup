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
package openbackup.data.access.framework.core.security.journal;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.security.Evaluation;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionRejectException;
import openbackup.system.base.common.aspect.LogParam;
import openbackup.system.base.common.aspect.OperationLogService;
import openbackup.system.base.common.constants.AspectOrderConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalEvent;
import openbackup.system.base.common.constants.RequestForwardRetryConstant;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.CollectionUtils;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.RequestUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.security.context.Context;
import openbackup.system.base.security.journal.Logging;
import openbackup.system.base.security.journal.Loggings;
import openbackup.system.base.service.DeployTypeService;

import org.apache.logging.log4j.util.Strings;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.ApplicationContext;
import org.springframework.context.ApplicationContextAware;
import org.springframework.core.annotation.Order;
import org.springframework.stereotype.Component;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.time.Instant;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

import javax.servlet.http.HttpServletRequest;
import javax.validation.ConstraintViolationException;

/**
 * Logging Aspect
 *
 */
@Slf4j
@Aspect
@Component
@Order(AspectOrderConstant.LOGGING_ASPECT_ORDER)
public class LoggingAspect implements ApplicationContextAware {
    private ApplicationContext applicationContext;

    private final OperationLogService operationLogService;

    private final AuthNativeApi authNativeApi;

    private final TokenVerificationService tokenVerificationService;

    private final ContextEvaluateService contextEvaluateService;

    private final LoggingEvaluateService loggingEvaluateService;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * constructor
     *
     * @param operationLogService operationLogService
     * @param authNativeApi authNativeApi
     * @param tokenVerificationService tokenVerificationService
     * @param contextEvaluateService contextEvaluateService
     * @param loggingEvaluateService loggingEvaluateService
     */
    public LoggingAspect(OperationLogService operationLogService, AuthNativeApi authNativeApi,
        TokenVerificationService tokenVerificationService, ContextEvaluateService contextEvaluateService,
        LoggingEvaluateService loggingEvaluateService) {
        this.operationLogService = operationLogService;
        this.authNativeApi = authNativeApi;
        this.tokenVerificationService = tokenVerificationService;
        this.contextEvaluateService = contextEvaluateService;
        this.loggingEvaluateService = loggingEvaluateService;
    }

    /**
     * normalize batch arguments
     *
     * @param args args
     * @return argument list
     */
    public static List<List<?>> normalizeBatchArguments(List<Object> args) {
        if (args == null) {
            return Collections.emptyList();
        }
        int count = args.stream().map(CollectionUtils::listify).mapToInt(List::size).max().orElse(0);
        List<List<?>> temp = args.stream()
            .map(detail -> CollectionUtils.listify(detail, count))
            .collect(Collectors.toList());
        List<List<?>> batchDetails = new ArrayList<>();
        for (int i = 0; i < count; i++) {
            List<Object> details = new ArrayList<>();
            for (List<?> item : temp) {
                Object detail = item.get(i);
                details.add(detail);
            }
            batchDetails.add(details);
        }
        return batchDetails;
    }

    public void setApplicationContext(ApplicationContext applicationContext) {
        this.applicationContext = Objects.requireNonNull(applicationContext);
    }

    private void evaluateBeforeExecute(Logging logging, LoggingContext loggingContext) {
        List<Context> contexts = Arrays.asList(logging.context());
        contextEvaluateService.evaluateBeforeExecute(loggingContext, contexts);
        loggingEvaluateService.evaluateBeforeExecute(loggingContext, logging);
    }

    /**
     * aspect loggings
     * 1. 先去找匹配deployType的
     * 2. 1找不到，去找deployType里面是空的，
     * 3. 2找不到，取第一个
     * <p>
     * 默认log deployType里面不定义
     *
     * @param joinPoint join point
     * @param loggings loggings
     * @return result
     */
    @Around(value = "@annotation(loggings)", argNames = "joinPoint,loggings")
    public Object aspectLogging(ProceedingJoinPoint joinPoint, Loggings loggings) {
        DeployTypeEnum curDeployType = deployTypeService.getDeployType();
        Logging logging = Arrays.stream(loggings.value())
            .filter(temp -> isMatchDeployType(temp, curDeployType))
            .findFirst()
            .orElse(Arrays.stream(loggings.value())
                .filter(temp -> isMatchNoneDeployType(temp))
                .findFirst()
                .orElse(loggings.value()[0]));
        return aspectLogging(joinPoint, logging);
    }

    private boolean isMatchDeployType(Logging logging, DeployTypeEnum curDeployType) {
        return Arrays.stream(logging.deployType()).anyMatch(deployTypeEnum -> deployTypeEnum.equals(curDeployType));
    }

    private boolean isMatchNoneDeployType(Logging logging) {
        return logging.deployType() == null || logging.deployType().length == 0;
    }

    /**
     * aspect logging
     *
     * @param joinPoint join point
     * @param logging logging
     * @return result
     */
    @Around(value = "@annotation(logging)", argNames = "joinPoint,logging")
    public Object aspectLogging(ProceedingJoinPoint joinPoint, Logging logging) {
        List<List<?>> args;
        List<Object> argList = Arrays.asList(joinPoint.getArgs());
        args = logging.batch().length > 0 ? normalizeBatchArguments(Arrays.stream(logging.batch())
            .map(statement -> new Evaluation(applicationContext, statement))
            .map(evaluation -> evaluateBatchStatement(argList, evaluation))
            .collect(Collectors.toList())) : Collections.singletonList(Arrays.asList(joinPoint.getArgs()));
        // 初始化上下文
        List<LoggingContext> loggingContexts = args.stream().map(LoggingContext::new).collect(Collectors.toList());
        RequestAttributes requestAttributes = Objects.requireNonNull(RequestContextHolder.getRequestAttributes());
        HttpServletRequest request;
        if (requestAttributes instanceof ServletRequestAttributes) {
            request = ((ServletRequestAttributes) requestAttributes).getRequest();
        } else {
            throw new LegoCheckedException("logic error");
        }
        Object returns;
        try {
            loggingContexts.forEach(loggingContext -> evaluateBeforeExecute(logging, loggingContext));
            loggingContexts.forEach(loggingContext -> evaluateRequires(logging, loggingContext));
            returns = joinPoint.proceed();
            updateReturnValueInContext(loggingContexts, returns);
            recordSuccessOperationLog(logging, request, loggingContexts);
        } catch (DataProtectionRejectException e) {
            log.error("error:{}", ExceptionUtil.getErrorMessage(e));
            throw e;
        } catch (DataProtectionAccessException e) {
            log.error("error:{}", ExceptionUtil.getErrorMessage(e));
            if (e.getErrorCode() != CommonErrorCode.ACCESS_DENIED) {
                updateReturnValueInContext(loggingContexts, null);
                recordFailureOperationLog(logging, request, loggingContexts, e);
            }
            throw e;
        } catch (LegoCheckedException e) {
            log.error("error:{}", ExceptionUtil.getErrorMessage(e));
            if (e.getErrorCode() != CommonErrorCode.ACCESS_DENIED) {
                updateReturnValueInContext(loggingContexts, null);
                recordFailureOperationLog(logging, request, loggingContexts, e);
            }
            throw e;
        } catch (Throwable e) {
            log.error("error:{}", ExceptionUtil.getErrorMessage(e));
            updateReturnValueInContext(loggingContexts, null);
            recordFailureOperationLog(logging, request, loggingContexts, e);
            throw LegoCheckedException.cast(e);
        }
        return returns;
    }

    private <T> void updateReturnValueInContext(List<LoggingContext> loggingContexts, T value) {
        loggingContexts.forEach(loggingContext -> {
            Map<String, Object> params = loggingContext.get(LoggingContext.PARAMS);
            params.put("$return", value);
            contextEvaluateService.evaluateAfterExecute(loggingContext);
            loggingEvaluateService.evaluateAfterExecute(loggingContext);
        });
    }

    private void evaluateRequires(Logging logging, LoggingContext loggingContext) {
        Map<String, Object> params = loggingContext.get(LoggingContext.PARAMS);
        List<String> missing = Stream.of(logging.context())
            .filter(Context::required)
            .map(Context::name)
            .filter(name -> params.get("$" + name) == null)
            .collect(Collectors.toList());
        if (!missing.isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "object not exist. items: " + missing);
        }
        Arrays.stream(logging.requires()).forEach(statement -> evaluateRequiredObject(loggingContext, statement));
    }

    private void evaluateRequiredObject(LoggingContext loggingContext, String statement) {
        Evaluation evaluation = new Evaluation(applicationContext, statement);
        if (evaluation.isDependOnReturnValue()) {
            throw new LegoCheckedException("config 'requires' is not allow to depend on return value");
        }
        Object result = evaluateWithParamsInContext(loggingContext, evaluation);
        if (result == null) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "object not exist");
        }
    }

    private Object evaluateBatchStatement(List<Object> args, Evaluation evaluation) {
        // batches参数不允许依赖$return返回值
        return Optional.ofNullable(evaluation)
            .filter(evaluation1 -> !evaluation1.isDependOnReturnValue())
            .map(evaluation1 -> evaluation1.evaluate(() -> Evaluation.buildParameters(args, null)))
            .orElse(null);
    }

    private void recordSuccessOperationLog(Logging logging, HttpServletRequest request, List<LoggingContext> batches) {
        recordOperationLog(logging, request, batches, true, null);
    }

    private List<Object> obtainEvaluationValue(LoggingContext loggingContext) {
        List<Evaluation> evaluations = loggingContext.get(LoggingContext.EVALUATIONS);
        return evaluations.stream()
            .map(evaluation -> evaluation.evaluate(Collections::emptyMap))
            .collect(Collectors.toList());
    }

    private Object evaluateWithParamsInContext(LoggingContext loggingContext, Evaluation evaluation) {
        Map<String, Object> params = loggingContext.get(LoggingContext.PARAMS);
        return evaluation.evaluate(() -> Evaluation.buildParameters(loggingContext.getArgs(), params));
    }

    private void recordFailureOperationLog(Logging logging, HttpServletRequest request, List<LoggingContext> batches,
        Throwable error) {
        rethrowValidateException(error);
        LegoCheckedException legoCheckedException = ExceptionUtil.lookFor(error, LegoCheckedException.class);
        if (legoCheckedException != null) {
            recordOperationLog(logging, request, batches, false, legoCheckedException);
            log.info("error,{}", ExceptionUtil.getErrorMessage(error));
            throw legoCheckedException;
        } else {
            recordOperationLog(logging, request, batches, false, null);
        }
        DataProtectionAccessException dataProtectionAccessException = ExceptionUtil.lookFor(error,
            DataProtectionAccessException.class);
        if (dataProtectionAccessException != null) {
            log.info("error,{}", ExceptionUtil.getErrorMessage(dataProtectionAccessException));
            throw dataProtectionAccessException;
        } else {
            log.info("error,{}", ExceptionUtil.getErrorMessage(error));
            throw LegoCheckedException.rethrow(error);
        }
    }

    private void rethrowValidateException(Throwable error) {
        MethodArgumentNotValidException methodArgumentNotValidException = ExceptionUtil.lookFor(error,
            MethodArgumentNotValidException.class);
        if (methodArgumentNotValidException != null) {
            throw LegoCheckedException.rethrow(methodArgumentNotValidException);
        }
        ConstraintViolationException constraintViolationException = ExceptionUtil.lookFor(error,
            ConstraintViolationException.class);
        if (constraintViolationException != null) {
            throw constraintViolationException;
        }
    }

    private void recordOperationLog(Logging logging, HttpServletRequest request, List<LoggingContext> loggingContexts,
        boolean isSuccess, LegoCheckedException legoCheckedException) {
        if (!VerifyUtil.isEmpty(request.getHeader(RequestForwardRetryConstant.HTTP_HEADER_INTERNAL_RETRY))) {
            // 内部跨控制器转发的重试请求，不记录操作日志，由原始的请求记录
            log.info("The request is retry by another node, no need record operation log again");
            return;
        }

        List<List<Object>> details = loggingContexts.stream()
            .map(this::obtainEvaluationValue)
            .collect(Collectors.toList());
        for (List<?> list : details) {
            String target = list.get(0).toString();
            String[] detailArray = list.stream()
                .skip(1)
                .map(detail -> Optional.ofNullable(detail).map(String::valueOf).orElse("--"))
                .toArray(String[]::new);
            LogParam param = new LogParam(target, detailArray, isSuccess, legoCheckedException);
            sendEvent(logging, request, param);
        }
    }

    private void sendEvent(Logging logging, HttpServletRequest request, LogParam param) {
        String[] params = initParams(request, logging, param.getDetails());
        param.setDetails(params);
        LegoInternalEvent event = buildEvent(request, logging, param);
        // 如果needCheckLogging为false，则不需要发送事件
        if (logging.needCheckLogging() && deployTypeService.isCyberEngine()) {
            boolean isNeedLogging = Boolean.parseBoolean(params[params.length - 1]);
            String[] originParams = Arrays.copyOfRange(params, 0, params.length - 1);
            param.setDetails(originParams);
            if (!isNeedLogging) {
                return;
            }
        }
        if (!logging.manual()) {
            event.setUserId(getUserIdFromAuthToken());
        } else {
            Optional<String> userIdOpt = getUserIdByUsername(param.getDetails()[0]);
            userIdOpt.ifPresent(event::setUserId);
        }
        operationLogService.sendEvent(event);
    }

    private LegoInternalEvent buildEvent(HttpServletRequest request, Logging logging, LogParam param) {
        LegoInternalEvent event = new LegoInternalEvent();
        String sourceType = param.getTarget().toLowerCase(Locale.ENGLISH);
        event.setSourceType("operation_target_" + sourceType + "_label");
        event.setEventLevel(logging.rank());
        event.setMoName(logging.name());
        event.setMoIP(RequestUtil.getClientIpAddress(request));
        event.setEventParam(param.getDetails());
        event.setEventId(logging.name());
        event.setSourceId(logging.name());
        event.setEventTime(Instant.now().getEpochSecond());
        event.setEventSequence(Instant.now().getNano());
        event.setIsSuccess(param.isSuccess());
        if (Objects.nonNull(param.getLegoCheckedException())) {
            event.setLegoErrorCode(String.valueOf(param.getLegoCheckedException().getErrorCode()));
        }
        return event;
    }

    private String[] initParams(HttpServletRequest request, Logging logging, String[] details) {
        String[] params;
        if (!logging.manual()) {
            params = new String[details.length + IsmNumberConstant.TWO];
            params[0] = getUsernameFromAuthToken();
            params[1] = RequestUtil.getClientIpAddress(request);
            System.arraycopy(details, 0, params, IsmNumberConstant.TWO, details.length);
        } else {
            params = details;
        }
        return params;
    }

    private String getUsernameFromAuthToken() {
        TokenBo token = tokenVerificationService.parsingTokenFromRequest();
        if (token == null) {
            return Strings.EMPTY;
        }
        return token.getUser().getName();
    }

    private Optional<String> getUserIdByUsername(String username) {
        try {
            TokenBo.UserInfo userInfo = authNativeApi.queryUserInfoByName(username);
            return Optional.ofNullable(userInfo.getId());
        } catch (Exception error) {
            return Optional.empty();
        }
    }

    private String getUserIdFromAuthToken() {
        TokenBo token = tokenVerificationService.parsingTokenFromRequest();
        if (VerifyUtil.isEmpty(token) || VerifyUtil.isEmpty(token.getUser())
            || VerifyUtil.isEmpty(token.getUser().getId())) {
            // 特定场景 如当触发邮箱找回密码操作时 用户未登录 则没有对应的userId
            log.warn("Logging aspect, fail to find userId from token.");
            return Strings.EMPTY;
        }
        return token.getUser().getId();
    }
}
