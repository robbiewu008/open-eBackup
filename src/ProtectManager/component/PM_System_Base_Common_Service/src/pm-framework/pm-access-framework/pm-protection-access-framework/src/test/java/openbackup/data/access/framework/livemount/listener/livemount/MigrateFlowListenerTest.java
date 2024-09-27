package openbackup.data.access.framework.livemount.listener.livemount;

import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.data.access.framework.livemount.listener.MigrateFlowListener;
import openbackup.data.access.framework.livemount.provider.LiveMountFlowService;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.kafka.MessageObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.util.ProviderRegistry;

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
import org.redisson.client.codec.Codec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.ContextConfiguration;

/**
 * 功能描述
 *
 * @author h30003246
 * @since 2021-04-21
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest
@ContextConfiguration(classes = MigrateFlowListener.class)
public class MigrateFlowListenerTest {
    /**
     * mounted copy
     */
    protected static final String MOUNTED_COPY = "mounted_copy";

    private static final String JOB_ID = "job_id";

    private static final String REQUEST_ID = "request_id";

    private static final String LIVE_MOUNT = "live_mount";

    private static final String MIGRATE = "migrate";

    /**
     * 任务执行失败，根据下发迁移任务状态设置已挂载副本状态。
     */
    private static final String HAS_DELIVER_MIGRATE = "has_deliver_migrate";

    private static final String TRUE = "true";

    private static final String JOB_STATUS = "job_status";

    /**
     * source copy
     */
    private static final String SOURCE_COPY = "source_copy";

    @Mock
    private ProviderRegistry providerRegistry;

    @Mock
    private LiveMountService liveMountService;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private ScheduleRestApi scheduleRestApi;

    @InjectMocks
    @Autowired
    private MigrateFlowListener listener;

    /**
     * 测试请求迁移流程成功
     */
    @Test
    public void test_request_Live_mount_migrate_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        JSONObject data = new JSONObject();
        data.set(REQUEST_ID, "request_id");
        data.set(LIVE_MOUNT, liveMountEntity);
        data.set(SOURCE_COPY, new Copy());
        data.set(MOUNTED_COPY, new Copy());

        PowerMockito.doNothing().when(liveMountService).checkTargetEnvironmentStatus(ArgumentMatchers.any());
        PowerMockito.doNothing()
            .when(liveMountService)
            .updateLiveMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.doNothing().when(jobCenterRestApi).updateJob(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.any())).thenReturn(new Copy());
        PowerMockito.doNothing().when(copyRestApi).updateCopyStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.when(providerRegistry.findProvider(LiveMountFlowService.class,
            liveMountEntity.getResourceSubType())).thenReturn(null);
        RMap rMap = Mockito.mock(RMap.class);
        rMap.put(data.get(REQUEST_ID), new JSONObject().set(HAS_DELIVER_MIGRATE, String.valueOf(true)));
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), (Codec) ArgumentMatchers.any()))
            .thenReturn(rMap);
        listener.requestLiveMountMigrate(data.toString(), acknowledgment);
        Mockito.verify(copyRestApi, Mockito.times(1)).updateCopyStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
    }

    /**
     * 测试处理迁移成功消息
     */
    @Test
    public void test_migrate_live_mount_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        JSONObject data = new JSONObject();
        data.set(REQUEST_ID, "request_id");
        data.set(LIVE_MOUNT, liveMountEntity);
        data.set(SOURCE_COPY, new Copy());
        data.set(MOUNTED_COPY, new Copy());
        data.set(JOB_STATUS, "SUCCESS");

        PowerMockito.doNothing()
            .when(liveMountService)
            .updateLiveMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.doNothing().when(liveMountService).deleteLiveMount(ArgumentMatchers.any());
        PowerMockito.doNothing().when(jobCenterRestApi).updateJob(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.any())).thenReturn(new Copy());
        PowerMockito.doNothing().when(copyRestApi).updateCopyStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.doNothing().when(scheduleRestApi).deleteSchedule(ArgumentMatchers.any());
        RMap rMap = Mockito.mock(RMap.class);
        rMap.put(data.get(REQUEST_ID), new JSONObject().set(HAS_DELIVER_MIGRATE, String.valueOf(true)));
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), (Codec) ArgumentMatchers.any()))
            .thenReturn(rMap);
        MessageObject messageObject = listener.migrateLiveMount(data.toString(), acknowledgment);
        Assert.assertEquals(messageObject.getString("topic"), TopicConstants.LIVE_MOUNT_CLEAN_CLONE_COPY);
        Assert.assertEquals(messageObject.getString("message"), data.toString());
    }
}
