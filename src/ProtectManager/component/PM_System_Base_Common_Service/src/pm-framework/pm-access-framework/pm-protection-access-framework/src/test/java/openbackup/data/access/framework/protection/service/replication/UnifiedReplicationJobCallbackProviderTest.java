package openbackup.data.access.framework.protection.service.replication;

import openbackup.data.access.framework.protection.service.replication.UnifiedReplicationJobCallbackProvider;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;

/**
 * RestoreJobCallbackProvider 类测试
 *
 * @since 2022-09-12
 */
@RunWith(PowerMockRunner.class)
public class UnifiedReplicationJobCallbackProviderTest {
    @Mock
    private RedissonClient redissonClient;

    @InjectMocks
    private UnifiedReplicationJobCallbackProvider unifiedReplicationJobCallbackProvider;


    /**
     * 用例场景：测试applicable接口成功
     * 前置条件：输入不同的jobType
     * 检查点：复制回调成功
     */
    @Test
    public void applicable_unified_replication_callback_success() {
        Assert.assertTrue(unifiedReplicationJobCallbackProvider.applicable("copy_replication"));
    }

    /**
     * 用例场景：测试callback接口成功。
     * 前置条件：输入正确的jobBo
     * 检查点：成功调用接口，不抛出异常。
     */
    @Test
    public void replication_callback_success() {
        JobBo copyReplicationJob = mockJob(JobTypeEnum.COPY_REPLICATION.getValue());
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE))).thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        PowerMockito.when(map.get(ArgumentMatchers.eq("target_cluster"))).thenReturn(targetClusterVo);
        unifiedReplicationJobCallbackProvider.doCallback(copyReplicationJob);
        Mockito.verify(redissonClient, Mockito.times(1)).getMap(copyReplicationJob.getJobId(), StringCodec.INSTANCE);
    }
    private JobBo mockJob(String jobType) {
        JobBo jobBo = new JobBo();
        jobBo.setJobId(UUIDGenerator.getUUID());
        jobBo.setCopyId(UUIDGenerator.getUUID());
        jobBo.setType(jobType);
        return jobBo;
    }
}
