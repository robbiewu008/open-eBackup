/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.datamover.core.listener.task;

import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.listener.v1.TaskCompleteListener;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.service.RedissonService;
import openbackup.system.base.util.ProviderRegistry;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;

import java.util.UUID;

/**
 * Task Complete Listener Test
 *
 * @author lwx776769
 * @since 2021-01-04
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(TaskCompleteListener.class)
@AutoConfigureMockMvc
public class TaskCompleteListenerTest {
    @Mock
    private RedissonService redissonService;

    @Mock
    private ProviderRegistry registry;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @InjectMocks
    private TaskCompleteListener listener;

    @Before
    public void setUp() {
        MockitoAnnotations.initMocks(this);
    }

    @Test
    public void testTaskComplete() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonService.getMap(ArgumentMatchers.anyString())).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        String requestId = UUID.randomUUID().toString();
        taskCompleteMessageBo.setJobRequestId(requestId);
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);
        PowerMockito.when(map.get("job_type")).thenReturn(JobTypeEnum.BACKUP.getValue());
        TaskCompleteHandler handler = PowerMockito.mock(TaskCompleteHandler.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(), ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(handler);

        listener.taskComplete(JSONObject.fromObject(taskCompleteMessageBo).toString(), acknowledgment);
        Mockito.verify(jobCenterRestApi, Mockito.times(1)).enforceStop(requestId,true);
    }
}