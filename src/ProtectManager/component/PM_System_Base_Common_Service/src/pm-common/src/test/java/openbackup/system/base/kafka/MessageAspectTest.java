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

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertThrows;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.MessageRetryException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.annotations.MessageContext;
import openbackup.system.base.kafka.annotations.MessageListener;
import openbackup.system.base.kafka.annotations.TopicMessage;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.SensitiveDataEliminateService;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;
import openbackup.system.base.util.RedisContextService;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RBucket;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.aop.aspectj.annotation.AnnotationAwareAspectJAutoProxyCreator;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.autoconfigure.sql.init.SqlInitializationAutoConfiguration;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.context.annotation.Import;
import org.springframework.kafka.annotation.TopicPartition;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.junit4.SpringRunner;

import java.lang.annotation.Annotation;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Message Aspect Test
 *
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@Import(AnnotationAwareAspectJAutoProxyCreator.class)
@SpringBootTest(classes = {MessageAspect.class, MessageTestListener.class, SqlInitializationAutoConfiguration.class})
@MockBean( {
    MessageTemplate.class, RedissonClient.class, RedisContextService.class, JobCenterRestApi.class,
    SensitiveDataEliminateService.class, ProviderRegistry.class, DeployTypeService.class
})
public class MessageAspectTest {
    @Autowired
    private MessageTestListener listener;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private ProviderRegistry providerRegistry;

    /**
     * 用例名称：验证入參消息的request id不正确场景的逻辑处理。<br/>
     * 前置条件：入參消息的request id不正确。<br/>
     * check点：抛出特定错误消息及错误码。<br/>
     */
    @Test
    public void test_request_id_empty() {
        test_request_id_empty(new JSONObject().set("job_id", "job_id").set("request_id", ""), "request id is empty");
        test_request_id_empty(new JSONObject().set("job_id", "job_id"),
            "field of request id may be incorrect or request id missing");
    }

    private void test_request_id_empty(JSONObject message, String errorMessage) {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        assertThrows(MessageRetryException.class,
            () -> listener.throwLegoCheckedException(message.toString(), acknowledgment));
    }

    /**
     * 用例名称：验证业务逻辑抛出LegoCheckedException场景任务状态更新。<br/>
     * 前置条件：业务逻辑抛出LegoCheckedException。<br/>
     * check点：任务状态更新为失败。<br/>
     */
    @Test
    public void test_throw_lego_checked_exception() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        JSONObject message = new JSONObject().set("request_id", "request_id").set("job_id", "job_id");
        mockContextMap("request_id");
        mockBucket();

        LegoCheckedException exception = assertThrows(LegoCheckedException.class,
            () -> listener.throwLegoCheckedException(message.toString(), acknowledgment));

        assertTrue(exception.getMessage().startsWith("mock error. message: "));
        ArgumentCaptor<String> jobIdCaptor = ArgumentCaptor.forClass(String.class);
        ArgumentCaptor<JobStatusEnum> statusCaptor = ArgumentCaptor.forClass(JobStatusEnum.class);
        verify(jobCenterRestApi).completeJob(jobIdCaptor.capture(), statusCaptor.capture());
        assertEquals("job_id", jobIdCaptor.getValue());
        assertEquals(JobStatusEnum.FAIL, statusCaptor.getValue());
    }

    private void mockBucket() {
        RBucket<Object> bucket = PowerMockito.mock(RBucket.class);
        PowerMockito.when(redissonClient.getBucket(any(), eq(StringCodec.INSTANCE))).thenReturn(bucket);
    }

    private void mockContextMap(String requestId) {
        RMap<Object, Object> map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(eq(requestId), eq(StringCodec.INSTANCE))).thenReturn(map);
    }

    @Test
    public void test_intercept_lock_response() {
        JSONObject message = JSONObject.fromObject(
            "{\"msg_id\":\"b8401927-5ba2-4255-b783-87170465ae97\",\"request_id\":\"cc93c69d-bd91-4cc0-98e4-cda742b71e82\",\"default_publish_topic\":\"UnlockResponse\",\"response_topic\":\"\",\"status\":\"success\",\"error_desc\":\"\"}");
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        mockContextMap(message.getString("request_id"));
        mockBucket();
        listener.returnTopicMessage(message.toString(), acknowledgment);
        verify(providerRegistry, times(0)).findProvider(MessageTestApplicable.class, "");
    }

    /**
     * 测试获取锁配置功能
     *
     * @throws NoSuchMethodException NoSuchMethodException
     * @throws InvocationTargetException InvocationTargetException
     * @throws IllegalAccessException IllegalAccessException
     */
    @Test
    public void test_build_lock_params()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        MessageAspect messageAspect = new MessageAspect();
        Method method = MessageAspect.class.getDeclaredMethod("buildLockParams", MessageListener.class,
            JSONObject.class);
        method.setAccessible(true);

        test_build_lock_params(messageAspect, method, new String[] {"resources:{'r'=payload.source_copy.uuid}"},
            new JSONObject().set("payload",
                new JSONObject().set("source_copy", new JSONObject().set("uuid", "copy_uuid"))),
            "{\"resources\":[{\"id\":\"copy_uuid\",\"lock_type\":\"r\"}]}");

        test_build_lock_params(messageAspect, method, new String[] {"resources:{payload.clone_copy.uuid}"},
            new JSONObject().set("payload",
                new JSONObject().set("clone_copy", new JSONObject().set("uuid", "copy_uuid"))),
            "{\"resources\":[{\"id\":\"copy_uuid\",\"lock_type\":\"w\"}]}");

        test_build_lock_params(messageAspect, method,
            new String[] {"resources:{payload.live_mount.mounted_copy_id,payload.live_mount.mounted_resource_id}"},
            new JSONObject().set("payload", new JSONObject().set("live_mount",
                new JSONObject().set("mounted_copy_id", "copy_uuid").set("mounted_resource_id", "resource_uuid"))),
            "{\"resources\":[{\"id\":\"copy_uuid\",\"lock_type\":\"w\"},{\"id\":\"resource_uuid\",\"lock_type\":\"w\"}]}");

        test_build_lock_params(messageAspect, method, new String[] {
                "resources:{'r'=payload.live_mount.mounted_copy_id,payload.live_mount.mounted_resource_id}"
            }, new JSONObject().set("payload", new JSONObject().set("live_mount",
                new JSONObject().set("mounted_copy_id", "copy_uuid").set("mounted_resource_id", "resource_uuid"))),
            "{\"resources\":[{\"id\":\"copy_uuid\",\"lock_type\":\"r\"},{\"id\":\"resource_uuid\",\"lock_type\":\"r\"}]}");
    }

    private void test_build_lock_params(MessageAspect messageAspect, Method method, String[] lock, JSONObject data,
        String expect) throws IllegalAccessException, InvocationTargetException {
        MessageListener messageTestListener = new MockMessageTestListener(lock);
        Object object = method.invoke(messageAspect, messageTestListener, data);
        assertEquals(expect, object.toString());
    }

    static class MockMessageTestListener implements MessageListener {
        private final String[] lock;

        public MockMessageTestListener(String[] lock) {
            this.lock = lock;
        }

        @Override
        public TopicMessage[] messages() {
            return new TopicMessage[0];
        }

        @Override
        public MessageContext[] messageContexts() {
            return new MessageContext[0];
        }

        @Override
        public String[] stack() {
            return new String[0];
        }

        @Override
        public boolean loadStack() {
            return false;
        }

        @Override
        public String[] data() {
            return new String[0];
        }

        @Override
        public String contextField() {
            return null;
        }

        @Override
        public String[] log() {
            return new String[0];
        }

        @Override
        public String[] lock() {
            return lock;
        }

        @Override
        public boolean unlock() {
            return false;
        }

        @Override
        public String[] sensitive() {
            return new String[0];
        }

        @Override
        public boolean terminatedMessage() {
            return false;
        }

        @Override
        public String[] failures() {
            return new String[0];
        }

        @Override
        public boolean retryable() {
            return false;
        }

        @Override
        public boolean enforceStop() {
            return false;
        }

        @Override
        public String id() {
            return null;
        }

        @Override
        public String containerFactory() {
            return null;
        }

        @Override
        public String[] topics() {
            return new String[0];
        }

        @Override
        public String topicPattern() {
            return null;
        }

        @Override
        public TopicPartition[] topicPartitions() {
            return new TopicPartition[0];
        }

        @Override
        public String containerGroup() {
            return null;
        }

        @Override
        public String errorHandler() {
            return null;
        }

        @Override
        public String groupId() {
            return null;
        }

        @Override
        public boolean idIsGroup() {
            return false;
        }

        @Override
        public String clientIdPrefix() {
            return null;
        }

        @Override
        public String beanRef() {
            return null;
        }

        @Override
        public String concurrency() {
            return null;
        }

        @Override
        public String autoStartup() {
            return null;
        }

        @Override
        public String[] properties() {
            return new String[0];
        }

        @Override
        public Class<? extends Annotation> annotationType() {
            return null;
        }
    }
}
