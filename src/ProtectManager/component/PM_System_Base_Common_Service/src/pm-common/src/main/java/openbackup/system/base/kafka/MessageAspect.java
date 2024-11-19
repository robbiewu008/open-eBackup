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
package openbackup.system.base.kafka;

import com.fasterxml.jackson.core.JsonProcessingException;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.MessageRetryException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.ExprUtil;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.kafka.annotations.MessageContext;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.kafka.annotations.TopicMessage;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.constants.JobContextKeys;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.job.util.JobLogBoUtil;
import openbackup.system.base.security.exterattack.ExterAttack;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.SensitiveDataEliminateService;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.RedisContextService;

import org.apache.commons.lang3.StringUtils;
import org.apache.kafka.common.KafkaException;
import org.aspectj.lang.ProceedingJoinPoint;
import org.aspectj.lang.annotation.Around;
import org.aspectj.lang.annotation.Aspect;
import org.redisson.api.RBucket;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.stereotype.Component;

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
import java.util.UUID;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicReference;
import java.util.function.BiFunction;
import java.util.function.Function;
import java.util.function.Predicate;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * Kafka Method Aspect
 *
 */
@Component
@Aspect
@Slf4j
public class MessageAspect {
    private static final Logger LOGGER = LoggerFactory.getLogger(MessageAspect.class);

    private static final String RETURNS = "returns";

    private static final String JOB_ID = "job_id";

    private static final String STATUS = "status";

    private static final String REQUEST_ID = "request_id";

    private static final Pattern PATTERN_ARG = Pattern.compile("\\{\\s*([^{}:\\s]+)\\s*}");

    private static final Pattern PATTERN_KEY = Pattern.compile("^\\s*([^\":]*)\\s*:");

    private static final String PAYLOAD = "payload";

    private static final String CONTEXT = "context";

    private static final String LOCK_ID = "lock_id";

    private static final String LOCK = "lock";

    private static final String JOB_STATUS = "job_status";

    private static final String RESPONSE_TOPIC = "response_topic";

    private static final String LOCK_RESPONSE = "LockResponse";

    private static final String UNLOCK_RESPONSE = "UnlockResponse";

    private static final String EXPECT = "expect";

    private static final String SUCCESS = "SUCCESS";

    private static final String LIVE_MOUNT_EXECUTE_PROCESS = "live-mount.execute.process";

    @Autowired
    private MessageTemplate messageTemplate;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private RedisContextService redisContextService;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private SensitiveDataEliminateService sensitiveDataEliminateService;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * enhance kafka method
     *
     * @param joinPoint join point
     * @param messageListener message listener
     * @return result
     * @throws Throwable throwable
     */
    @Around(value = "((execution(* com.huawei..*(..))) || (execution(* openbackup..*(..)))) "
            + "&& @annotation(messageListener)", argNames = "joinPoint, messageListener")
    public Object enhanceKafkaMethod(ProceedingJoinPoint joinPoint, MessageListener messageListener) throws Throwable {
        Object result = null;
        final List<Object> argumentList = Arrays.asList(joinPoint.getArgs());
        JSONObject params = new JSONObject();
        JSONObject payload;
        long startTime = System.currentTimeMillis();
        MessageAspectListenerContext messageContext = new MessageAspectListenerContext(messageListener, params,
                argumentList);
        AtomicReference<String> stack = new AtomicReference<>("");
        try {
            int index = getPayloadIndex(argumentList);
            final Object argument = argumentList.get(index);
            if (!(argument instanceof String)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "argument is not instance of String");
            }
            payload = JSONObject.fromObject(argument);
            final Optional<String> optional = handleRequestId(payload, messageListener);
            if (!optional.isPresent()) {
                acknowledge(messageListener, params, argumentList, "request empty force ack");
                return null;
            }
            final String requestId = optional.get();
            this.buildContext(requestId);
            // 检查是否中止任务
            if (checkSendDeleteJobOrNot(messageListener, requestId)) {
                return null;
            }
            recordPayload(messageListener, payload, "origin");
            Map<String, String> context = getContextData(payload, messageListener);
            params.put(PAYLOAD, payload);
            params.put(CONTEXT, context);
            if (dropDeduplicateMessages(params, messageListener, argumentList)) {
                return null;
            }
            stack.set(context.get(MessageContext.STACK));
            if (Boolean.TRUE.equals(payload.get("message.retry.failed"))) {
                return processFailureAfterRetry(messageContext, argumentList, startTime);
            }
            if (interceptMessage(messageContext, payload, startTime)) {
                acknowledge(messageListener, params, argumentList, "intercept message");
                return null;
            }
            result = procBusinessLogic(params, messageListener, joinPoint, index, startTime);
        } catch (Throwable e) {
            handleMessageException(messageListener, e, stack, messageContext, startTime);
        } finally {
            sendMessage(messageContext, MessagePhase.COMPLETE.name());
        }
        return result;
    }

    @ExterAttack
    private void buildContext(String requestId) {
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        if (map.containsKey(JobContextKeys.RUNNING_STATE_COUNT)) {
            map.addAndGet(JobContextKeys.RUNNING_STATE_COUNT, 1);
            log.info("Job is running, RUNNING_STATE_COUNT: {}. JobId: {}", map.get(JobContextKeys.RUNNING_STATE_COUNT),
                    requestId);
        }
    }

    private Optional<String> handleRequestId(JSONObject payload, MessageListener messageListener) {
        try {
            return Optional.of(getRequestId(payload, messageListener));
        } catch (LegoCheckedException ex) {
            log.error("get request id failed", ex);
            return Optional.empty();
        }
    }

    @ExterAttack
    private boolean checkSendDeleteJobOrNot(MessageListener messageListener, String requestId) {
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        if (!map.containsKey(JobContextKeys.ABORTING_STATE_COUNT)) {
            return false;
        }
        // 如果任务中止计数器ABORTING_STATE_COUNT为0，说明已发送任务中止消息，直接返回，不执行当前业务
        if ("0".equals(map.get(JobContextKeys.ABORTING_STATE_COUNT).toString())) {
            log.info("ABORTING_STATE_COUNT equals 0, message delete job has been sent. JobId: {}", requestId);
            return true;
        }
        // 如果还有其他线程在运行，不发送任务中止的消息
        if (!"1".equals(map.get(JobContextKeys.RUNNING_STATE_COUNT).toString())) {
            return false;
        }
        // 如果任务中止计数器ABORTING_STATE_COUNT为1，发送删除任务上下文的消息,返回
        if ("1".equals(map.get(JobContextKeys.ABORTING_STATE_COUNT).toString())) {
            // 解决消费completeTask消息时，因enforceStop=false,导致中止任务消息发送给数据面的情况
            if (messageListener.enforceStop()) {
                jobCenterRestApi.enforceStop(requestId, true);
            }
            log.info("Message {} will not be consumed. JobId: {}", messageListener.topics(), requestId);
            sendDeleteJobMessage(requestId, true);
            map.addAndGet(JobContextKeys.RUNNING_STATE_COUNT, -1);
            return true;
        }
        return false;
    }

    private void handleMessageException(MessageListener messageListener, Throwable error, AtomicReference<String> stack,
            MessageAspectListenerContext messageContext, long startTime) throws Throwable {
        log.error("handle message {} failed", messageListener.topics(), error);
        messageContext.setCommitMsg(false);
        LegoCheckedException legoCheckedException = ExceptionUtil.lookFor(error, LegoCheckedException.class);
        if (legoCheckedException == null) {
            restoreStackData(messageContext, stack.get());
            JSONObject params = messageContext.getParams();
            JSONObject payload = params.getJSONObject(PAYLOAD);
            JSONObject context = params.getJSONObject(CONTEXT);
            String jobId = getStringValue(JOB_ID, payload, context);
            sendDeleteJobMessage(jobId, false);
            if (isRetryable(messageListener)) {
                throw new MessageRetryException(error);
            } else {
                throw error;
            }
        }

        checkRequestIdError(legoCheckedException);

        processTerminatedMessage(messageContext, startTime, JobStatusEnum.FAIL.name(), legoCheckedException);
        boolean isSent = sendFailureMessage(messageContext);
        if (!isSent) {
            throw new LegoCheckedException(error.getMessage(), error.getCause());
        }
    }

    private void checkRequestIdError(LegoCheckedException legoCheckedException) {
        StackTraceElement[] stackTraceElements = legoCheckedException.getStackTrace();
        if (stackTraceElements.length > 0) {
            StackTraceElement element = stackTraceElements[0];
            String methodName = element.getMethodName();
            String clazzName = element.getClassName();
            if ("getRequestId".equals(methodName) && MessageAspect.class.getName().equals(clazzName)) {
                throw legoCheckedException;
            }
        }
    }

    // 对接收的消息进行重复消费判断
    private boolean dropDeduplicateMessages(JSONObject params, MessageListener messageListener, List<Object> args) {
        String requestId = getRequestId(params.getJSONObject(PAYLOAD), messageListener);
        String committedMessageRedisKey = getCommittedMessageRedisKey(requestId, params, messageListener);
        RBucket<Object> bucket = getRedissonBucket(committedMessageRedisKey);
        log.info("consume message key: {}", committedMessageRedisKey);
        if (!VerifyUtil.isEmpty(bucket.get())) {
            acknowledge(messageListener, params, args, "repeat consume message");
            return true;
        }
        return false;
    }

    @ExterAttack
    private RBucket<Object> getRedissonBucket(String committedMessageRedisKey) {
        RBucket<Object> bucket;
        for (int i = 0;; i++) {
            try {
                bucket = redissonClient.getBucket(committedMessageRedisKey, StringCodec.INSTANCE);
                break;
            } catch (Throwable e) {
                log.error("get redis map failed, retry time: {}.", i, e);
            }
        }
        return bucket;
    }

    private boolean isRetryable(MessageListener messageListener) {
        return messageListener.failures().length > 0 || messageListener.terminatedMessage()
                || messageListener.retryable();
    }

    private void restoreStackData(MessageAspectListenerContext messageContext, String stack) {
        if (stack != null && stack.isEmpty()) {
            log.error("An exception may occur when obtaining the original stack.");
            return;
        }
        JSONObject params = messageContext.getParams();
        JSONObject payload = params.getJSONObject(PAYLOAD);
        MessageListener listener = messageContext.getMessageListener();
        String requestId = getRequestId(payload, listener);
        redisContextService.updateStringValue(requestId, MessageContext.STACK, stack);
        log.info("restore stack for retry. topic: {}, request id: {}", listener.topics(), requestId);
    }

    private Object processFailureAfterRetry(MessageAspectListenerContext messageContext, List<Object> args,
            long startTime) {
        JSONObject params = messageContext.getParams();
        JSONObject payload = params.getJSONObject(PAYLOAD);
        payload.remove("message.retry.failed");
        MessageListener messageListener = messageContext.getMessageListener();
        processResponseMessage(messageListener, args, params, payload, startTime);
        markJobFail(params, messageListener);
        processTerminatedMessage(messageContext, startTime, JobStatusEnum.FAIL.name(),
                new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR));
        sendFailureMessage(messageContext);
        acknowledge(messageListener, params, args, "commit retry failed message");
        return null;
    }

    private void markJobFail(JSONObject params, MessageListener messageListener) {
        JSONObject payload = params.getJSONObject(PAYLOAD);
        String requestId = getRequestId(payload, messageListener);
        redisContextService.updateStringValue(requestId, JOB_STATUS, "FAIL");
        payload.set(JOB_STATUS, "FAIL");
        JSONObject context = params.getJSONObject(CONTEXT);
        context.set(JOB_STATUS, "FAIL");
    }

    private Object procBusinessLogic(JSONObject params, MessageListener messageListener, ProceedingJoinPoint joinPoint,
            int index, long startTime) throws Throwable {
        Object[] args = joinPoint.getArgs();
        JSONObject fullPayload = params.getJSONObject(PAYLOAD).duplicate();
        updatePayloadWithStack(messageListener, fullPayload, params);
        updatePayloadWithContextData(messageListener, fullPayload, params);
        recordPayload(messageListener, fullPayload, "complete");
        args[index] = fullPayload.toString();
        Object result = joinPoint.proceed(args);
        log.info("proc business result:{}", result);
        String phase;
        if (result != null) {
            phase = checkResult(result);
            params.put(RETURNS, result.toString());
        } else {
            phase = SUCCESS;
        }
        MessageAspectListenerContext messageAspectListenerContext = new MessageAspectListenerContext(messageListener,
                params, Arrays.asList(args));
        sendMessage(messageAspectListenerContext, phase);
        processTerminatedMessage(messageAspectListenerContext, startTime, phase);

        return result;
    }

    private String checkResult(Object result) {
        if (result instanceof JSONObject) {
            return ((JSONObject) result).getString(STATUS);
        }
        return null;
    }

    private boolean interceptMessage(MessageAspectListenerContext messageContext, JSONObject payload, long startTime) {
        MessageListener messageListener = messageContext.getMessageListener();
        List<Object> args = Arrays.asList(messageContext.getArgs());
        JSONObject params = messageContext.getParams();
        log.info("messageListener.lock(): {}", Arrays.asList(messageListener.lock()));
        if (messageListener.lock().length > 0) {
            if (processLockMessage(messageListener, args, params, payload, startTime)) {
                return true;
            } else {
                prepareMessageContexts(messageListener, params);
            }
        } else {
            JSONObject lock = getLockObject(params);
            log.info("interceptMessage lock:{}", lock);
            String defaultPublishTopic = payload.getString("default_publish_topic");
            if (lock != null) {
                if (matchResponseTopic(lock, defaultPublishTopic, UNLOCK_RESPONSE)) {
                    processUnlockResponseMessage(messageContext, payload, startTime);
                    return true;
                } else {
                    prepareMessageContexts(messageListener, params);
                }
            } else if (messageListener.unlock() && UNLOCK_RESPONSE.equals(defaultPublishTopic)) {
                return processContextLost(messageContext);
            } else {
                prepareMessageContexts(messageListener, params);
            }
        }
        return false;
    }

    private boolean processContextLost(MessageAspectListenerContext messageContext) {
        MessageListener messageListener = messageContext.getMessageListener();
        JSONObject params = messageContext.getParams();
        JSONObject payload = params.getJSONObject(PAYLOAD);
        String requestId = getRequestId(payload, messageListener);
        if (Arrays.asList(messageListener.topics()).contains(UNLOCK_RESPONSE)) {
            log.error("not allow define topic as {}. request id: {}", UNLOCK_RESPONSE, requestId);
        } else {
            log.error("lock message of context may be lost. request id: {}", requestId);
        }
        return true;
    }

    private boolean processLockMessage(MessageListener messageListener, List<Object> args, JSONObject params,
            JSONObject payload, long startTime) {
        if (processResponseMessage(messageListener, args, params, payload, startTime)) {
            return true;
        }
        if (deployTypeService.isCyberEngine()) {
            // 如果任务类型是即时挂载，直接返回false
            JSONObject context = params.getJSONObject(CONTEXT);
            String jobType = context.getString("job_type");
            if (StringUtils.equals(jobType, JobTypeEnum.LIVE_MOUNT.getValue())
                    && Arrays.asList(messageListener.topics()).contains(LIVE_MOUNT_EXECUTE_PROCESS)) {
                return false;
            }
        }
        JSONObject lockObject = getLockObject(params);
        if (lockObject == null) {
            return lockResource(messageListener, params);
        }
        return false;
    }

    private boolean processResponseMessage(MessageListener messageListener, List<Object> args, JSONObject params,
            JSONObject payload, long startTime) {
        JSONObject lock = getLockObject(params);
        String defaultPublishTopic = payload.getString("default_publish_topic");
        MessageAspectListenerContext messageAspectListenerContext = new MessageAspectListenerContext(messageListener,
                params, args);
        if (defaultPublishTopic != null) {
            if (matchResponseTopic(lock, defaultPublishTopic, LOCK_RESPONSE)) {
                return processLockResponseMessage(messageAspectListenerContext, payload);
            } else if (matchResponseTopic(lock, defaultPublishTopic, UNLOCK_RESPONSE)) {
                processUnlockResponseMessage(messageAspectListenerContext, payload, startTime);
                return true;
            } else {
                LOGGER.info("matchResponseTopic error");
            }
        }
        return false;
    }

    private JSONObject getLockObject(JSONObject params) {
        JSONObject context = params.getJSONObject(CONTEXT);
        return context.getJSONObject(LOCK);
    }

    private boolean matchResponseTopic(JSONObject lock, String actual, String expect) {
        if (!Objects.equals(actual, expect) || lock == null) {
            return false;
        }
        String topic = lock.getString(EXPECT);
        if (!Objects.equals(topic, actual)) {
            throw new LegoCheckedException("lock resource failed");
        }
        return true;
    }

    private void processUnlockResponseMessage(MessageAspectListenerContext messageContext, JSONObject payload,
            long startTime) {
        // clean lock id
        JSONObject context = messageContext.getParams().getJSONObject(CONTEXT);
        String requestId = payload.getString(messageContext.getMessageListener().contextField());
        JSONObject lock = context.getJSONObject(LOCK_ID);
        JobStatusEnum status = getJobStatus(payload.getString("status"));
        if (status.checkSuccess()) {
            // 解锁成功场景下，将任务中的lock_id移除；
            // 解锁失败场景下，如果存在任务卡住的情况，用户可通过中止再次解锁
            updateLockIdOfJob(requestId, lock, false);
        }

        context.remove(LOCK);
        redisContextService.updateStringValue(requestId, LOCK, null);

        JSONObject data = loadDataFromStack(messageContext.getMessageListener(), payload, messageContext.getParams(),
                null);
        JSONObject terminatedParams = data.pick(PAYLOAD, RETURNS).set(CONTEXT, context);
        processTerminatedMessage(new MessageAspectListenerContext(messageContext, terminatedParams), startTime,
                status.name(), true, null);
    }

    private JobStatusEnum getJobStatus(String status) {
        if (status == null) {
            return JobStatusEnum.SUCCESS;
        }
        return JobStatusEnum.get(status);
    }

    private boolean hasAnyFailure(String... status) {
        if (status == null) {
            return false;
        }
        for (String state : status) {
            if (JobStatusEnum.FAIL.name().equalsIgnoreCase(state)) {
                return true;
            }
        }
        return false;
    }

    private JobStatusEnum getProcessJobStatus(JSONObject params, String phase) {
        JSONObject context = params.getJSONObject(CONTEXT);
        JSONObject payload = params.getJSONObject(PAYLOAD);
        String contextJobStatus = context.getString(JOB_STATUS);
        String payloadJobStatus = payload.getString(JOB_STATUS);
        String payloadStatus = payload.getString(STATUS);

        // 如果有返回任务结束状态，就直接返回该状态
        JobStatusEnum jobStatusEnum = getJobStatusFromReturns(params);
        if (jobStatusEnum != null) {
            return jobStatusEnum;
        }

        List<String> statusList = Stream.of(contextJobStatus, payloadJobStatus, payloadStatus).filter(Objects::nonNull)
                .map(String::toUpperCase).collect(Collectors.toList());
        statusList.add(phase);
        List<JobStatusEnum> candidateStatusList = Arrays.asList(JobStatusEnum.FAIL, JobStatusEnum.PARTIAL_SUCCESS,
                JobStatusEnum.SUCCESS);
        for (JobStatusEnum candidateStatus : candidateStatusList) {
            if (statusList.contains(candidateStatus.name())) {
                return candidateStatus;
            }
        }
        return JobStatusEnum.SUCCESS;
    }

    private JobStatusEnum getJobStatusFromReturns(JSONObject params) {
        JSONObject returns = params.getJSONObject(RETURNS);
        if (returns == null) {
            return null;
        }
        String returnStatus = returns.getString(JOB_STATUS);
        return returnStatus != null ? JobStatusEnum.get(returnStatus, true) : null;
    }

    private boolean processLockResponseMessage(MessageAspectListenerContext messageContext, JSONObject payload) {
        String payloadStatus = payload.getString(STATUS);
        String requestId = payload.getString(messageContext.getMessageListener().contextField());

        // 加锁成功或失败后，将之前压入栈中的数据弹出
        JSONObject data = loadDataFromStack(messageContext.getMessageListener(), payload, messageContext.getParams(),
                new String[]{PAYLOAD});
        messageContext.getParams().set(PAYLOAD, data.getJSONObject(PAYLOAD));

        if (!"success".equals(payloadStatus)) {
            // 加锁失败场景
            updateLockInfo(requestId, messageContext.getParams(), new JSONObject().set(STATUS, "fail"));
            log.info("lock resource failed. topic: {}, status: {}",
                    Arrays.asList(messageContext.getMessageListener().topics()), payloadStatus);
            sendFailureMessage(messageContext);
            return true;
        } else {
            // 加锁成功场景
            updateLockInfo(requestId, messageContext.getParams(), new JSONObject().set(STATUS, "success"), true);
            return false;
        }
    }

    private void unlockResource(MessageListener messageListener, JSONObject params) {
        JSONObject payload = params.getJSONObject(PAYLOAD);
        JSONObject context = params.getJSONObject(CONTEXT);
        String requestId = payload.getString(messageListener.contextField());
        JSONObject lock = context.getJSONObject(LOCK);
        String lockId = lock.getString(LOCK_ID);
        updateLockInfo(requestId, params, new JSONObject().set(EXPECT, UNLOCK_RESPONSE));
        String topic = messageListener.topics()[0];
        JSONObject json = new JSONObject().set(REQUEST_ID, requestId).set(LOCK_ID, lockId).set(RESPONSE_TOPIC, topic);
        pushStack(messageListener, params, topic, "{\"payload\":{payload},\"returns\":{returns}}");
        send("UnlockRequest", json);
    }

    private boolean sendFailureMessage(MessageAspectListenerContext messageProcessContext) {
        JSONObject params = messageProcessContext.getParams();

        MessageListener messageListener = messageProcessContext.getMessageListener();
        JSONObject payload = params.getJSONObject(PAYLOAD);
        JSONObject data = loadDataFromStack(messageListener, payload, params, null);
        JSONObject json;
        if (data.isEmpty()) {
            json = payload;
        } else {
            // 恢复上下文栈的有效荷载
            json = data.getJSONObject(PAYLOAD);
            params.put(PAYLOAD, json);
        }
        String requestId = json.getString(messageListener.contextField());
        redisContextService.updateStringValue(requestId, JOB_STATUS, JobStatusEnum.FAIL.name());

        JSONObject info = new JSONObject().set(JOB_STATUS, JobStatusEnum.FAIL.name());
        JSONObject context = params.getJSONObject(CONTEXT);
        String jobId = getStringValue(JOB_ID, context, json);
        if (jobId != null) {
            info.set(JOB_ID, jobId);
        }
        messageProcessContext.setTopicMessages(createFailureTopicMessage(messageListener.failures()));
        return sendMessage(messageProcessContext, JobStatusEnum.FAIL.name(), info);
    }

    private TopicMessage[] createFailureTopicMessage(String[] topics) {
        TopicMessage[] topicMessages = new TopicMessage[topics.length];
        for (int i = 0; i < topics.length; i++) {
            String topic = topics[i];
            TopicMessage topicMessage = new TopicMessageInfo(topic, new String[]{MessageContext.PAYLOAD},
                    new MessagePhase[]{MessagePhase.FAILURE});
            topicMessages[i] = topicMessage;
        }
        return topicMessages;
    }

    private boolean lockResource(MessageListener messageListener, JSONObject params) {
        JSONObject json = new JSONObject().set("wait_timeout", IsmNumberConstant.NEGATIVE_ONE).set("priority",
                IsmNumberConstant.TWO);
        JSONObject lockParams = buildLockParams(messageListener, params);
        json.putAll(lockParams);
        JSONArray resources = json.getJSONArray("resources");
        if (VerifyUtil.isEmpty(resources)) {
            log.info("no resources need to lock. topics: {}", Arrays.asList(messageListener.topics()));
            return false;
        }
        JSONObject payload = params.getJSONObject(PAYLOAD);
        JSONObject context = params.getJSONObject(CONTEXT);
        String requestId = payload.getString(messageListener.contextField());
        String jobId = Optional.ofNullable(getStringValue(JOB_ID, payload, context)).orElse("ignore");
        String lockId = UUID.randomUUID() + "@" + jobId;
        String topic = messageListener.topics()[0];
        json.set(REQUEST_ID, requestId).set(LOCK_ID, lockId).set(RESPONSE_TOPIC, topic);
        JSONObject lock = new JSONObject().set(LOCK_ID, lockId).set(EXPECT, LOCK_RESPONSE);
        updateLockInfo(requestId, params, lock);
        prepareMessageContexts(messageListener, params);
        pushStack(messageListener, params, topic, "{\"payload\":{payload}}");
        send("LockRequest", json);
        return true;
    }

    private void pushStack(MessageListener messageListener, JSONObject params, String topic, String message) {
        prepareMessageContext(messageListener, params,
                new MessageContextInfo(new String[]{message}, MessageContextInfo.STACK, topic));
    }

    private void updateLockInfo(String requestId, JSONObject params, JSONObject lock) {
        updateLockInfo(requestId, params, lock, false);
    }

    private void updateLockInfo(String requestId, JSONObject params, JSONObject lock, boolean isInit) {
        updateLockInfo(requestId, params, lock, isInit ? Optional.of(true) : Optional.empty());
    }

    private void updateLockInfo(String requestId, JSONObject params, JSONObject lock, Optional<Boolean> init) {
        JSONObject context = params.getJSONObject(CONTEXT);
        JSONObject rawLock = context.getJSONObject(LOCK);
        JSONObject newLock;
        if (rawLock != null) {
            rawLock.update(lock);
            newLock = rawLock;
        } else {
            newLock = lock;
        }
        redisContextService.update(requestId, LOCK, newLock);
        context.set(LOCK, newLock);
        init.ifPresent(option -> updateLockIdOfJob(requestId, newLock, option));
    }

    private void updateLockIdOfJob(String requestId, JSONObject lock, boolean isInit) {
        String lockId = isInit ? lock.getString(LOCK_ID) : null;
        UpdateJobRequest request = new UpdateJobRequest();
        request.setData(new JSONObject().set(LOCK_ID, lockId));
        jobCenterRestApi.updateJob(requestId, request);
    }

    private void acknowledge(MessageListener messageListener, JSONObject params, List<Object> args,
            String... messages) {
        int index = getIndexOfType(args, Acknowledgment.class);
        Object arg = args.get(index);
        if (arg instanceof Acknowledgment) {
            Acknowledgment acknowledgment = (Acknowledgment) arg;
            acknowledgment.acknowledge();
            addCommittedMessageToRedis(params, messageListener);
            log.info("message committed. topic: {}, messages: {}", messageListener.topics(), messages);
        }
    }

    private void addCommittedMessageToRedis(JSONObject params, MessageListener messageListener) {
        String requestId = getRequestId(params.getJSONObject(PAYLOAD), messageListener);
        String committedMessageRedisKey = getCommittedMessageRedisKey(requestId, params, messageListener);
        RBucket<Object> bucket = getRedissonBucket(committedMessageRedisKey);
        bucket.set(params.getJSONObject(PAYLOAD));
        bucket.expire(20, TimeUnit.MINUTES);
        log.info("committed message redis bucket key = {}", committedMessageRedisKey);
    }

    private String getCommittedMessageRedisKey(String requestId, JSONObject params, MessageListener messageListener) {
        String topics = Arrays.toString(messageListener.topics());
        int payLoadHashCode = params.getJSONObject(PAYLOAD).toString().hashCode();
        return "committed_message" + "_" + requestId + "_" + topics + "_" + payLoadHashCode + "_"
                + messageListener.groupId();
    }

    private JSONObject buildLockParams(MessageListener messageListener, JSONObject params) {
        JSONObject result = new JSONObject();
        String[] templates = messageListener.lock();
        for (String template : templates) {
            Object data = buildJsonMessageByTemplate(template, params);
            JSONObject json = JSONObject.fromObject(data);
            Object value = json.get("resources");
            if (value != null) {
                JSONArray resources = getLockResources(value);
                json.put("resources", resources);
            }
            result.putAll(json);
        }
        return result;
    }

    private JSONArray getLockResources(Object value) {
        JSONArray array;
        if (!(value instanceof JSONArray)) {
            array = new JSONArray(new Object[]{value});
        } else {
            array = (JSONArray) value;
        }
        JSONArray resources = new JSONArray();
        for (Object item : array) {
            addLockResource(resources, item);
        }
        return resources;
    }

    private void addLockResource(JSONArray resources, Object item) {
        if (item instanceof String) {
            addLockResource(resources, item, "w");
        } else if (item instanceof JSONObject) {
            JSONObject json = (JSONObject) item;
            json.forEach((key, value) -> addLockResource(resources, value, key.toString()));
        } else {
            LOGGER.info("item error");
        }
    }

    private void addLockResource(JSONArray resources, Object item, String lockType) {
        if (item instanceof Collection) {
            Collection<?> collection = (Collection<?>) item;
            for (Object element : collection) {
                resources.add(new JSONObject().set("id", element).set("lock_type", lockType));
            }
        } else if (!VerifyUtil.isEmpty(item)) {
            resources.add(new JSONObject().set("id", item.toString()).set("lock_type", lockType));
        } else {
            resources.addAll(Collections.emptyList());
        }
    }

    private void recordPayload(MessageListener messageListener, JSONObject payload, String type) {
        JSONObject data = payload.duplicate();
        sensitiveDataEliminateService.eliminate(data, Arrays.asList(messageListener.sensitive()));
        log.debug("{} message: {}, payload: {}", type, messageListener.topics(), data);
    }

    private Object parseAsJson(String content) {
        try {
            Object result = JSONObject.DEFAULT_OBJ_MAPPER.readValue(content, Map.class);
            if (result == null) {
                return null;
            }
            return JSONObject.fromObject(content);
        } catch (JsonProcessingException e) {
            try {
                JSONObject.DEFAULT_OBJ_MAPPER.readValue(content, List.class);
                return JSONArray.fromObject(content);
            } catch (JsonProcessingException ex) {
                return content;
            }
        }
    }

    private void updatePayloadWithContextData(MessageListener messageListener, JSONObject payload, JSONObject params) {
        JSONObject context = params.getJSONObject(CONTEXT);
        String[] fields = flatFields(messageListener.data());
        payload.update(context.pick(fields));
    }

    private void updatePayloadWithStack(MessageListener messageListener, JSONObject payload, JSONObject params) {
        if (!messageListener.loadStack()) {
            return;
        }
        JSONObject data = loadDataFromStack(messageListener, payload, params);
        payload.putAll(data);
    }

    private JSONObject loadDataFromStack(MessageListener messageListener, JSONObject payload, JSONObject params) {
        return loadDataFromStack(messageListener, payload, params, messageListener.stack());
    }

    private JSONObject loadDataFromStack(MessageListener messageListener, JSONObject payload, JSONObject params,
            String[] fields) {
        JSONObject data;
        JSONObject stack = getStack(params);
        if (isStackTopicMatched(messageListener, stack)) {
            JSONObject message = stack.getJSONObject("message");
            if (fields != null) {
                data = message.pick(flatFields(fields));
            } else {
                data = message;
            }
            JSONObject historyStack = popupStack(messageListener, stack, payload);
            updateContextStack(params, historyStack);
        } else {
            data = new JSONObject();
        }
        return data;
    }

    private void updateContextStack(JSONObject params, JSONObject historyStack) {
        JSONObject context = params.getJSONObject(CONTEXT);
        if (VerifyUtil.isEmpty(historyStack)) {
            context.remove(MessageContext.STACK);
        } else {
            context.put(MessageContext.STACK, historyStack);
        }
    }

    private String[] flatFields(String[] fields) {
        List<String> list = new ArrayList<>();
        for (String field : fields) {
            list.addAll(Arrays.stream(field.split(",")).map(String::trim).collect(Collectors.toList()));
        }
        return list.toArray(new String[0]);
    }

    private JSONObject popupStack(MessageListener messageListener, JSONObject currentStack, JSONObject payload) {
        String requestId = payload.getString(messageListener.contextField());
        JSONObject historyStack = currentStack.getJSONObject(MessageContext.STACK);
        redisContextService.update(requestId, MessageContext.STACK, historyStack);
        return historyStack;
    }

    private JSONObject getStack(JSONObject params) {
        JSONObject context = params.getJSONObject(CONTEXT);
        String stack = context.getString(MessageContext.STACK);
        if (stack == null || "null".equals(stack)) {
            return null;
        }
        JSONObject data = JSONObject.fromObject(stack);
        if (data.isEmpty()) {
            return null;
        }
        return data;
    }

    private boolean isStackTopicMatched(MessageListener messageListener, JSONObject stack) {
        if (stack == null) {
            return false;
        }
        String stackTopic = stack.getString("topic");
        for (String topic : messageListener.topics()) {
            if (Objects.equals(stackTopic, topic)) {
                return true;
            }
        }
        return false;
    }

    private boolean onSuccessTopicMessagePhase(List<MessagePhase> phases) {
        return phases.contains(MessagePhase.SUCCESS) && !phases.contains(MessagePhase.COMPLETE);
    }

    private boolean onFailureTopicMessagePhase(List<MessagePhase> phases) {
        return phases.contains(MessagePhase.FAILURE) && !phases.contains(MessagePhase.COMPLETE);
    }

    private boolean onCompleteTopicMessagePhase(List<MessagePhase> phases) {
        return phases.isEmpty() || phases.contains(MessagePhase.COMPLETE);
    }

    private boolean sendMessage(MessageAspectListenerContext messageProcessContext, String phase, JSONObject... data) {
        updateJobStatus(messageProcessContext, phase);
        Predicate<List<MessagePhase>> predicate = getMessagePredicateByPhase(phase);
        List<TopicMessage> topicMessages = getMatchedTopicMessage(messageProcessContext.getMessageListener(), predicate,
                messageProcessContext.getTopicMessages());
        sendTopicMessages(topicMessages, messageProcessContext.getParams(), data);
        return !topicMessages.isEmpty();
    }

    private Predicate<List<MessagePhase>> getMessagePredicateByPhase(String phase) {
        Map<String, Predicate<List<MessagePhase>>> predicates = new HashMap<>();
        addMessagePhasePredicate(predicates, this::onSuccessTopicMessagePhase, MessagePhase.SUCCESS.name(),
                JobStatusEnum.PARTIAL_SUCCESS.name());
        addMessagePhasePredicate(predicates, this::onFailureTopicMessagePhase, MessagePhase.FAILURE.name(),
                JobStatusEnum.FAIL.name());
        addMessagePhasePredicate(predicates, this::onCompleteTopicMessagePhase, MessagePhase.COMPLETE.name());
        return predicates.get(phase);
    }

    private void addMessagePhasePredicate(Map<String, Predicate<List<MessagePhase>>> predicates,
            Predicate<List<MessagePhase>> predicate, String... keys) {
        for (String key : keys) {
            predicates.put(key, predicate);
        }
    }

    private void updateJobStatus(MessageProcessContext messageProcessContext, String phase) {
        Map<String, String> jobStates = new HashMap<>();
        jobStates.put(MessagePhase.FAILURE.name(), JobStatusEnum.FAIL.name());
        JobStatusEnum status = JobStatusEnum.get(jobStates.getOrDefault(phase, phase), true);
        if (status != null) {
            JSONObject params = messageProcessContext.getParams();
            JSONObject context = params.getJSONObject(CONTEXT);
            JSONObject payload = params.getJSONObject(PAYLOAD);
            boolean isFailure = hasAnyFailure(context.getString(JOB_STATUS), payload.getString(STATUS), status.name());
            if (isFailure) {
                context.set(JOB_STATUS, JobStatusEnum.FAIL.name());
                MessageListener listener = messageProcessContext.getMessageListener();
                String requestId = params.getString(listener.contextField());
                redisContextService.updateStringValue(requestId, JOB_STATUS, JobStatusEnum.FAIL.name());
            }
        }
    }

    private void sendTopicMessages(List<TopicMessage> topicMessages, JSONObject params, JSONObject... data) {
        for (TopicMessage topicMessage : topicMessages) {
            sendTopicMessage(topicMessage, params, data);
        }
    }

    private List<TopicMessage> getMatchedTopicMessage(MessageListener messageListener,
            Predicate<List<MessagePhase>> predicate, TopicMessage... extras) {
        TopicMessage[] topicMessages = Stream.of(messageListener.messages(), extras).filter(Objects::nonNull)
                .map(Arrays::asList).flatMap(List::stream).toArray(TopicMessage[]::new);
        List<TopicMessage> messages = new ArrayList<>();
        List<String> topics = Arrays.asList(messageListener.topics());
        for (TopicMessage topicMessage : topicMessages) {
            if (topics.contains(topicMessage.topic())) {
                continue;
            }
            List<MessagePhase> phases = Arrays.asList(topicMessage.phases());
            if (predicate.test(phases)) {
                messages.add(topicMessage);
            }
        }
        return messages;
    }

    private void sendTopicMessage(TopicMessage topicMessage, JSONObject params, JSONObject... extras) {
        JSONObject message = new JSONObject();
        if (extras != null) {
            for (JSONObject extra : extras) {
                message.update(extra);
            }
        }
        for (String template : topicMessage.messages()) {
            Object json;
            Matcher matcher = PATTERN_KEY.matcher(template);
            if (matcher.find()) {
                String topic = matcher.group(1);
                int index = template.indexOf(':');
                String suffix = template.substring(index + 1);
                Object data = buildTopicMessageByTemplate(suffix, params);
                if (!VerifyUtil.isEmpty(topic)) {
                    json = new JSONObject().set("topic", topic).set("message", data);
                } else {
                    json = data;
                }
            } else {
                json = buildTopicMessageByTemplate(template, params);
            }
            if (json instanceof JSONObject) {
                message.putAll((JSONObject) json);
            } else {
                log.error("json is not JSONObject.");
            }
        }
        String topic = topicMessage.topic();
        send(topic, message);
    }

    private void send(String topic, JSONObject message) {
        try {
            messageTemplate.send(topic, message);
        } catch (KafkaException e) {
            log.error("send message failed. topic: {}", topic, e);
        }
    }

    private void processTerminatedMessage(MessageAspectListenerContext messageProcessContext, long startTime,
            String phase) {
        processTerminatedMessage(messageProcessContext, startTime, phase, null);
    }

    private void processTerminatedMessage(MessageAspectListenerContext messageProcessContext, long startTime,
            String phase, LegoCheckedException legoCheckedException) {
        processTerminatedMessage(messageProcessContext, startTime, phase, false, legoCheckedException);
    }

    private void processTerminatedMessage(MessageAspectListenerContext messageContext, long startTime, String phase,
            boolean isUnlocked, LegoCheckedException legoCheckedException) {
        JSONObject params = messageContext.getParams();
        JSONObject payload = params.getJSONObject(PAYLOAD);
        JSONObject context = params.getJSONObject(CONTEXT);
        log.info("processTerminatedMessage startTime: {}, phase:{}, unlocked:{}", startTime, phase, isUnlocked);
        MessageListener messageListener = messageContext.getMessageListener();
        boolean isTerminatedMessage = isTerminatedMessage(messageListener);

        if (!isUnlocked) {
            // 更新任务日志信息
            String jobId = getStringValue(JOB_ID, payload, context);
            updateJobLog(messageContext, jobId, startTime, phase, legoCheckedException);
            sendDeleteJobMessage(jobId, false);
        }

        JSONObject lock = context.getJSONObject(LOCK);
        if (!isUnlocked
                && needUnlockResource(isTerminatedMessage || messageContext.getMessageListener().unlock(), lock)) {
            String requestId = payload.getString(messageListener.contextField());
            updateLockInfo(requestId, params, new JSONObject().set(STATUS, "unlocking"));
            unlockResource(messageContext.getMessageListener(), messageContext.getParams());
            acknowledge(messageListener, params, Arrays.asList(messageContext.getArgs()), "commit business message");
            return;
        }

        processTerminatedMessage(messageContext, isTerminatedMessage, isUnlocked, phase);
    }

    @ExterAttack
    private void sendDeleteJobMessage(String jobId, boolean isForce) {
        if (VerifyUtil.isEmpty(jobId)) {
            return;
        }
        RMap<Object, Object> map = redissonClient.getMap(jobId, StringCodec.INSTANCE);
        if (map == null) {
            return;
        }
        // 如果不存在RUNNING_STATE_COUNT，对应拦截层开始时的检查
        if (!map.containsKey(JobContextKeys.RUNNING_STATE_COUNT)) {
            return;
        }
        String runningStateCount;
        if (isForce) {
            // 当前业务开始前，获取RUNNING_STATE_COUNT--
            runningStateCount = Integer
                    .toString((Integer.parseInt(map.get(JobContextKeys.RUNNING_STATE_COUNT).toString()) - 1));
        } else {
            // 当前业务结束后，更新RUNNING_STATE_COUNT，并获取返回值
            runningStateCount = map.addAndGet(JobContextKeys.RUNNING_STATE_COUNT, -1).toString();
        }
        String abortingStateCount;
        // 没有正在运行的业务，且ABORTING_STATE_COUNT为1，则删除任务上下文
        if ("0".equals(runningStateCount) && !VerifyUtil.isEmpty(map.get(JobContextKeys.ABORTING_STATE_COUNT))
                && "1".equals(map.get(JobContextKeys.ABORTING_STATE_COUNT).toString())) {
            abortingStateCount = map.addAndGet(JobContextKeys.ABORTING_STATE_COUNT, -1).toString();
            log.info("RunningStateCount: {}, AbortingStateCount: {}", runningStateCount, abortingStateCount);
            if ("0".equals(abortingStateCount)) {
                log.info("Calling delete job when there is no running job. JobId: {}", jobId);
                jobCenterRestApi.abortTask(jobId);
            }
        }
    }

    private void processTerminatedMessage(MessageAspectListenerContext messageProcessContext, boolean isTerminated,
            boolean isUnlock, String phase) {
        log.info("processTerminatedMessage terminated:{}, unlock:{}, commit msg:{}", isTerminated, isUnlock,
                messageProcessContext.isCommitMsg);
        JSONObject params = messageProcessContext.getParams();
        MessageListener messageListener = messageProcessContext.getMessageListener();
        if (isTerminated) {
            JSONObject payload = params.getJSONObject(PAYLOAD);
            JSONObject context = params.getJSONObject(CONTEXT);
            String jobId = getStringValue(JOB_ID, payload, context);
            String requestId = payload.getString(messageListener.contextField());
            String jobType = getStringValue("job_type", payload, context);
            JobStatusEnum status = getProcessJobStatus(params, phase);
            log.info("job {} is finished.(request id: {}, topics: {}, job type: {}, status: {})", jobId, requestId,
                    messageListener.topics(), jobType, status);

            if (jobId != null) {
                jobCenterRestApi.completeJob(jobId, status);
            }
            redisContextService.delete(requestId);
            payload.remove(JOB_ID);
        }
        sendNextTopicMessageOnDemand(params);
        if (!isUnlock && messageProcessContext.isCommitMsg) {
            acknowledge(messageListener, params, Arrays.asList(messageProcessContext.getArgs()),
                    "commit business message");
        }
    }

    private void sendNextTopicMessageOnDemand(JSONObject params) {
        String returns = params.getString(RETURNS);
        if (returns == null) {
            return;
        }
        JSONObject result = JSONObject.fromObject(returns);
        String topic = result.getString("topic");
        if (VerifyUtil.isEmpty(topic)) {
            return;
        }
        send(topic, result.getJSONObject("message"));
    }

    private boolean needUnlockResource(boolean isCondition, JSONObject lock) {
        return isCondition && lock != null && "success".equals(lock.getString(STATUS));
    }

    private boolean isTerminatedMessage(MessageListener messageListener) {
        if (messageListener.terminatedMessage()) {
            return true;
        }
        List<String> topics = Arrays.asList(messageListener.topics());
        List<TopicMessage> topicMessages = getMatchedTopicMessage(messageListener, this::onFailureTopicMessagePhase);
        for (TopicMessage topicMessage : topicMessages) {
            if (topics.contains(topicMessage.topic())) {
                return true;
            }
        }
        return false;
    }

    private void updateJobLog(MessageProcessContext messageProcessContext, String jobId, long startTime, String phase,
            LegoCheckedException legoCheckedException) {
        if (jobId == null) {
            return;
        }
        String[] log = messageProcessContext.getMessageListener().log();
        if (log.length == 0) {
            return;
        }

        // 加锁失败，不打印该流程的日志
        JSONObject lockObject = messageProcessContext.getParams().getJSONObject(CONTEXT).getJSONObject(LOCK);
        if (lockObject != null && "fail".equals(lockObject.getString(STATUS))) {
            return;
        }

        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(startTime);
        jobLogBo.setLogInfo(log[0]);
        JobStatusEnum status = getJobStatus(phase);
        JobLogLevelEnum level = getJobLogLevelByStatus(status);
        jobLogBo.setLevel(level.getValue());
        messageProcessContext.getParams().put(STATUS, status.name().toLowerCase(Locale.ENGLISH));
        List<String> param = buildLogInfoMessage(log, messageProcessContext.getParams());
        jobLogBo.setLogInfoParam(param);
        JobLogBoUtil.initJobLogDetail(jobLogBo, legoCheckedException);
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(Collections.singletonList(jobLogBo));
        jobCenterRestApi.updateJob(jobId, request);
    }

    private JobLogLevelEnum getJobLogLevelByStatus(JobStatusEnum status) {
        JobLogLevelEnum level;
        if (status == JobStatusEnum.PARTIAL_SUCCESS) {
            level = JobLogLevelEnum.WARNING;
        } else if (status.checkSuccess()) {
            level = JobLogLevelEnum.INFO;
        } else {
            level = JobLogLevelEnum.ERROR;
        }
        return level;
    }

    private List<String> buildLogInfoMessage(String[] items, JSONObject params) {
        return Arrays.asList(items).subList(1, items.length).stream()
                .map(item -> buildMessageByTemplate(item, params,
                        result -> result.endsWith("_label") ? result.toLowerCase(Locale.ENGLISH) : result))
                .collect(Collectors.toList());
    }

    private String getStringValue(String field, JSONObject... jsons) {
        for (JSONObject json : jsons) {
            if (json == null) {
                continue;
            }
            String value = json.getString(field);
            if (value != null) {
                return value;
            }
        }
        return null;
    }

    private void prepareMessageContexts(MessageListener messageListener, JSONObject params) {
        MessageContext[] messageContexts = messageListener.messageContexts();
        for (MessageContext messageContext : messageContexts) {
            prepareMessageContext(messageListener, params, messageContext);
        }
    }

    private void prepareMessageContext(MessageListener messageListener, JSONObject params,
            MessageContext messageContext) {
        String[] templates = messageContext.messages();
        for (String template : templates) {
            prepareMessageContextWithTemplate(messageListener, messageContext, template, params);
        }
    }

    private void prepareMessageContextWithTemplate(MessageListener messageListener, MessageContext messageContext,
            String template, JSONObject params) {
        JSONObject payload = params.getJSONObject(PAYLOAD);
        Object message = buildTopicMessageByTemplate(template, params);
        String chain = messageContext.chain();
        String requestId = payload.getString(messageListener.contextField());
        String topic = messageContext.topic();
        if (VerifyUtil.isEmpty(chain)) {
            redisContextService.set(requestId, message);
        } else if (!topic.isEmpty()) {
            JSONObject context = params.getJSONObject(CONTEXT);
            String value = context.getString(chain);
            JSONObject data = new JSONObject();
            data.set("topic", topic);
            data.set("message", message);
            if (!VerifyUtil.isEmpty(value) && !"null".equals(value)) {
                data.put(chain, value);
            }
            redisContextService.update(requestId, Collections.singletonList(new JSONObject().set(chain, data)));
        } else {
            LOGGER.info("topic invalid");
        }
    }

    private Object buildTopicMessageByTemplate(String template, JSONObject arguments) {
        return buildDataByTemplate(template, arguments,
                (key, val) -> new JSONObject().set("topic", key).set("message", val));
    }

    private Object buildJsonMessageByTemplate(String template, JSONObject arguments) {
        return buildDataByTemplate(template, arguments, (key, val) -> new JSONObject().set(key, val));
    }

    @SuppressWarnings("checkstyle:NestedIfDepth")
    private Object buildDataByTemplate(String template, JSONObject arguments,
            BiFunction<String, Object, JSONObject> constructor) {
        String message = template.replaceAll("^\\s+|\\s+$", "");
        if (!message.startsWith("{")) {
            Matcher matcher = PATTERN_KEY.matcher(message);
            if (matcher.find()) {
                int index = message.indexOf(":");
                String topic = matcher.group(1);
                String value = template.substring(index + 1);
                if (VerifyUtil.isEmpty(topic)) {
                    return buildDataByTemplate(value, arguments, constructor);
                }
                Object result = buildDataByTemplate(value, arguments, constructor);
                Object object;
                if (result instanceof String) {
                    object = parseResult(result.toString());
                } else {
                    object = result;
                }
                return constructor.apply(topic, object);
            }
            Object value = getValue(arguments, message);
            if (value == null || value instanceof JSONObject) {
                return value;
            }
            log.error("The value is not a JSON object and will be treated as a empty JSON object.");
            return new JSONObject();
        }
        return buildMessageByTemplate(template, arguments);
    }

    private Object parseResult(String result) {
        Object object;
        Object json = parseAsJson(result);
        if (json instanceof String) {
            if (result.contains(",")) {
                object = Arrays.stream(result.split(",")).map(this::parseAsJson).collect(Collectors.toList());
            } else {
                object = result;
            }
        } else {
            object = json;
        }
        return object;
    }

    private String buildMessageByTemplate(String template, JSONObject arguments,
            Function<String, String>... converters) {
        Matcher matcher = PATTERN_ARG.matcher(template);
        StringBuffer buffer = new StringBuffer();
        while (matcher.find()) {
            String matched = matcher.group(1);
            String expresion = matched.replaceAll("\\s+", "");
            Object value = getValue(arguments, expresion);
            matcher.appendReplacement(buffer, "");
            if (value != null) {
                buffer.append(value);
            } else {
                buffer.append("null");
            }
        }
        matcher.appendTail(buffer);
        String result = buffer.toString();
        for (Function<String, String> converter : converters) {
            result = converter.apply(result);
        }
        return result;
    }

    private Object getValue(JSONObject json, String expresion) {
        return getValue(json, expresion.split("\\|"));
    }

    private Object getValue(JSONObject json, String[] paths) {
        for (String path : paths) {
            String item = path.trim();
            if (item.isEmpty()) {
                continue;
            }
            int index = item.indexOf('=');
            if (index == -1) {
                Object value = getValueByPath(json, item);
                if (value != null) {
                    return value;
                }
            } else {
                String prefix = item.substring(0, index);
                String suffix = item.substring(index + IsmNumberConstant.ONE);
                JSONObject data = new JSONObject();
                Object key = getValueByPath(json, prefix);
                Object value = getValueByPath(json, suffix);
                data.set(key, value);
                return data;
            }
        }
        return null;
    }

    private Object getValueByPath(JSONObject json, String item) {
        String expr = item.trim();
        if (expr.startsWith("'") && expr.endsWith("'")) {
            return expr.substring(IsmNumberConstant.ONE, expr.length() - IsmNumberConstant.ONE);
        }
        if (!item.contains(",")) {
            return ExprUtil.eval(json, item, false);
        } else {
            JSONArray array = getValueArray(json, item);
            if (!array.isEmpty()) {
                return array;
            }
        }
        return null;
    }

    private JSONArray getValueArray(JSONObject json, String path) {
        String[] items = path.split(",");
        JSONArray array = new JSONArray();
        for (String item : items) {
            Object value = ExprUtil.eval(json, item);
            if (value != null) {
                array.add(value);
            }
        }
        return array;
    }

    @ExterAttack
    private Map<String, String> getContextData(JSONObject payload, MessageListener messageListener) {
        String requestId = getRequestId(payload, messageListener);
        RMap<Object, Object> map = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        Map<String, String> context = new HashMap<>();
        for (Map.Entry<Object, Object> entry : map.entrySet()) {
            String key = entry.getKey().toString();
            Object value = entry.getValue();
            if (value != null) {
                context.put(key, value.toString());
            }
        }
        return context;
    }

    private String getRequestId(JSONObject payload, MessageListener messageListener) {
        String requestId = payload.getString(messageListener.contextField());
        if (requestId == null) {
            log.error("field of request id may be incorrect or request id missing");
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "field of request id may be incorrect or request id missing");
        }
        return requestId;
    }

    private int getPayloadIndex(List<Object> args) {
        return getIndexOfType(args, String.class);
    }

    private int getIndexOfType(List<Object> args, Class<?> type) {
        for (int index = 0; index < args.size(); index++) {
            Object arg = args.get(index);
            if (type.isInstance(arg)) {
                return index;
            }
        }
        throw new LegoCheckedException("argument of type(" + type.getName() + ") is missing.");
    }

    static class MessageAspectListenerContext extends MessageProcessContext {
        private final List<Object> messageListenerAspectJoinPointArgs;
        private boolean isCommitMsg = true;

        MessageAspectListenerContext(MessageListener messageListener, JSONObject params, List<Object> args) {
            super(messageListener, params);
            this.messageListenerAspectJoinPointArgs = args;
        }

        MessageAspectListenerContext(MessageAspectListenerContext messageContext, JSONObject params) {
            super(messageContext.getMessageListener(), params, messageContext.getTopicMessages());
            this.messageListenerAspectJoinPointArgs = Arrays.asList(messageContext.getArgs());
        }

        /**
         * get aspect join point args
         *
         * @return args
         */
        public Object[] getArgs() {
            return messageListenerAspectJoinPointArgs.toArray(new Object[0]);
        }

        /**
         * set need commit msg flag
         *
         * @param isCommitMsg commit msg flag
         */
        public void setCommitMsg(boolean isCommitMsg) {
            this.isCommitMsg = isCommitMsg;
        }
    }
}
