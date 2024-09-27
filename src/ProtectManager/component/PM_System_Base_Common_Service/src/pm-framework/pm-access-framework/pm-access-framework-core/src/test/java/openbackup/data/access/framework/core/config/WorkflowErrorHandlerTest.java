package openbackup.data.access.framework.core.config;

import static org.assertj.core.api.BDDAssertions.thenThrownBy;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.data.access.framework.core.config.WorkflowErrorHandler;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.RedisTimeoutException;
import org.redisson.client.codec.Codec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.kafka.core.KafkaTemplate;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * Workflow Error Handler Test
 *
 * @author l00272247
 * @since 2022-03-21
 */
@RunWith(PowerMockRunner.class)
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {WorkflowErrorHandler.class})
@MockBean( {KafkaTemplate.class})
public class WorkflowErrorHandlerTest {
    @Autowired
    private WorkflowErrorHandler handler;

    @MockBean
    private RedissonClient redissonClient;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private JobService jobService;

    /**
     * 用例名称：验证WorkflowErrorHandler异常处理逻辑对任务详情的初始化是否正确。<br/>
     * 前置条件：异常信息中包含LegoCheckedException。<br/>
     * check点：任务详情的初始化正确；<br/>
     */
    @Test
    public void test_handle() {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(any(), any(Codec.class))).thenReturn(map);
        handler.handle("topic", new JSONObject().set("request_id", "request_id").toString(),
            new LegoCheckedException("some error"));
        ArgumentCaptor<UpdateJobRequest> updateJobRequestArgumentCaptor = ArgumentCaptor.forClass(
            UpdateJobRequest.class);
        verify(jobCenterRestApi).updateJob(any(), updateJobRequestArgumentCaptor.capture());
        UpdateJobRequest request = updateJobRequestArgumentCaptor.getValue();
        JobLogBo jobLogBo = request.getJobLogs().get(0);
        Assert.assertEquals("1677929219", jobLogBo.getLogDetail());
    }

    /**
     * 用例名称：验证任务正常结束失败时可以执行强制终止<br/>
     * 前置条件：操作redis发生异常<br/>
     * check点：<br/>
     * 1、强制终止方法被调用1次<br/>
     * 2、任务更新方法未被调用<br/>
     */
    @Test
    public void should_force_stop_workflow_when_complete_failed() {
        PowerMockito.when(redissonClient.getMap(any(), any(Codec.class)))
            .thenThrow(new RedisTimeoutException("command execute timeout"));
        thenThrownBy(() -> handler.handle("topic", new JSONObject().set("request_id", "request_id").toString(),
            new LegoCheckedException("some error"))).isInstanceOf(RedisTimeoutException.class)
            .hasMessage("command execute timeout");
        verify(jobCenterRestApi, never()).updateJob(any(), any());
        verify(jobService, times(1)).forceStopJob(eq("request_id"), eq(false));
    }
}
