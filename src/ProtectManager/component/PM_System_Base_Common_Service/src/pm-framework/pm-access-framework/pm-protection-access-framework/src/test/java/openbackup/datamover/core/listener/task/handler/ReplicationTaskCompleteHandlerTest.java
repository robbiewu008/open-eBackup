package openbackup.datamover.core.listener.task.handler;

import static org.mockito.ArgumentMatchers.anyString;

import openbackup.data.access.framework.protection.handler.v1.replication.ReplicationCopyProcessor;
import openbackup.data.access.framework.protection.handler.v1.replication.ReplicationTaskCompleteHandler;
import openbackup.data.protection.access.provider.sdk.job.TaskCompleteMessageBo;
import openbackup.system.base.common.utils.JobSpeedConverter;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.util.ReflectionTestUtils;

import java.util.UUID;
import java.util.concurrent.atomic.AtomicStampedReference;

/**
 * ReplicationTaskCompleteHandler LLT
 *
 * @author m00576658
 * @since 2021-03-25
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(ReplicationTaskCompleteHandler.class)
@AutoConfigureMockMvc
public class ReplicationTaskCompleteHandlerTest {
    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ProviderRegistry registry;

    @Mock
    private MessageTemplate<?> messageTemplate;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @InjectMocks
    private ReplicationTaskCompleteHandler replicationTaskCompleteHandler;

    @Test
    public void testOnTaskCompleteSuccess() {
        TaskCompleteMessageBo taskCompleteMessageBo = new TaskCompleteMessageBo();
        taskCompleteMessageBo.setJobRequestId(UUID.randomUUID().toString());
        taskCompleteMessageBo.setJobId(UUID.randomUUID().toString().replace("-", ""));
        taskCompleteMessageBo.setJobProgress(100);
        taskCompleteMessageBo.setJobStatus(3);

        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        PowerMockito.when(map.get(ArgumentMatchers.eq("target_cluster"))).thenReturn(targetClusterVo);
        ReplicationCopyProcessor processor = PowerMockito.mock(ReplicationCopyProcessor.class);
        PowerMockito.when(registry.findProvider(ArgumentMatchers.any(),ArgumentMatchers.any(),ArgumentMatchers.any())).thenReturn(processor);
        AtomicStampedReference<Boolean> stampedReference = new AtomicStampedReference<Boolean>(true,0);
        PowerMockito.when(processor.process(ArgumentMatchers.any())).thenReturn(stampedReference);
        replicationTaskCompleteHandler.onTaskCompleteSuccess(taskCompleteMessageBo);
        replicationTaskCompleteHandler.onTaskCompleteFailed(taskCompleteMessageBo);
    }

    @Test
    public void testApplicable() {
        boolean applicable = replicationTaskCompleteHandler.applicable("copy_replication");
        Assert.assertTrue(applicable);
        applicable = replicationTaskCompleteHandler.applicable("copy_replication-v2");
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：复制任务完成参数组成成功
     * 前置条件：复制任务完成
     * 检查点: 参数组装成功，速度正确
     */
    @Test
    public void test_recordeReplicatedCopyNumber() {
        String jobId = "78309289-f90e-4b94-9905-d1e01ebc6765";
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(jobId, StringCodec.INSTANCE)).thenReturn(map);
        PowerMockito.when(map.get("job_status")).thenReturn("SUCCESS");
        PowerMockito.when(map.get("job_id")).thenReturn(jobId);
        TaskCompleteMessageBo completeMessageBo = new TaskCompleteMessageBo();
        completeMessageBo.setSpeed(1025L);
        completeMessageBo.setTaskId(jobId);
        completeMessageBo.setJobStatus(3);
        completeMessageBo.setJobProgress(95);
        completeMessageBo.setJobRequestId(jobId);
        ReflectionTestUtils.invokeMethod(replicationTaskCompleteHandler, "recordeReplicatedCopyNumber",
            completeMessageBo, 1);
        ArgumentCaptor<UpdateJobRequest> argumentCaptor = ArgumentCaptor.forClass(UpdateJobRequest.class);
        Mockito.verify(jobCenterRestApi, Mockito.times(1)).updateJob(anyString(), argumentCaptor.capture());
        UpdateJobRequest value = argumentCaptor.getValue();
        Assert.assertEquals(value.getSpeed(),
            JobSpeedConverter.convertJobSpeed(String.valueOf(completeMessageBo.getSpeed())));
    }
}
