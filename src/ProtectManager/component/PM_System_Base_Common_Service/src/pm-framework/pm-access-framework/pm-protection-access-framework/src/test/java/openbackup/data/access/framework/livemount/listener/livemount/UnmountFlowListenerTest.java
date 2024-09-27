/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.listener.livemount;

import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.data.UnmountFlowListenerTestData;
import openbackup.data.access.framework.livemount.listener.UnmountFlowListener;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.kafka.MessageObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.util.MessageTemplate;

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
 * Mount Flow Listener Test
 *
 * @author l00272247
 * @since 2020-12-29
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest
@ContextConfiguration(classes = UnmountFlowListener.class)
public class UnmountFlowListenerTest {
    @InjectMocks
    @Autowired
    private UnmountFlowListener listener;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private LiveMountRestApi liveMountClientRestApi;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Mock
    private LiveMountService liveMountService;

    @Mock
    private MessageTemplate<String> messageTemplate;

    @Mock
    private LiveMountPolicyEntityDao liveMountPolicyEntityDao;

    @Mock
    private EnvironmentRestApi environmentRestApi;

    @Mock
    private ResourceRestApi resourceRestApi;

    @Mock
    private LiveMountEntityDao liveMountEntityDao;

    @Mock
    private RedissonClient redissonClient;

    /**
     * 测试卸载，资源被锁，卸载失败
     */
    @Test
    public void test_unmount_resource_lock_fail() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        data.set("request_id", UUIDGenerator.getUUID());
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        data.set("job_status", "SUCCESS");
        data.set("force_delete", "false");

        RMap rMap = Mockito.mock(RMap.class);
        Mockito.when(rMap.get("lock")).thenReturn(new JSONObject().set("status", "fail"));
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), (Codec) ArgumentMatchers.any()))
            .thenReturn(rMap);
        PowerMockito.doNothing().when(liveMountService).deleteLiveMount(ArgumentMatchers.any());
        PowerMockito.doNothing().when(jobCenterRestApi).completeJob(ArgumentMatchers.any(), ArgumentMatchers.any());

        MessageObject result = listener.deleteLiveMount(data.toString(), acknowledgment);
        Assert.assertNull(result);
    }

    /**
     * 测试请求销毁流程成功
     */
    @Test
    public void test_request_live_mount_unmount_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        data.set("request_id", UUIDGenerator.getUUID());
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        data.set("force_delete", "false");

        PowerMockito.doNothing().when(liveMountService).checkTargetEnvironmentStatus(ArgumentMatchers.any());
        PowerMockito.doNothing()
            .when(liveMountService)
            .updateLiveMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        RMap rMap = Mockito.mock(RMap.class);
        rMap.put("lock", new JSONObject().set("status", "fail"));
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), (Codec) ArgumentMatchers.any()))
            .thenReturn(rMap);
        listener.requestLiveMountUnmount(data.toString(), acknowledgment);
        Mockito.verify(liveMountService, Mockito.times(1)).updateLiveMountStatus(ArgumentMatchers.any(),
                ArgumentMatchers.any());
    }

    /**
     * 测试处理销毁成功消息
     */
    @Test
    public void test_live_mount_unmount_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        data.set("request_id", UUIDGenerator.getUUID());
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        data.set("job_status", "SUCCESS");
        data.set("force_delete", "false");

        PowerMockito.doNothing()
            .when(liveMountService)
            .updateLiveMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        RMap rMap = Mockito.mock(RMap.class);
        rMap.put("lock", new JSONObject().set("status", "fail"));
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), (Codec) ArgumentMatchers.any()))
            .thenReturn(rMap);
        PowerMockito.doNothing().when(liveMountService).deleteLiveMount(ArgumentMatchers.any());
        PowerMockito.doNothing().when(jobCenterRestApi).completeJob(ArgumentMatchers.any(), ArgumentMatchers.any());
        MessageObject messageObject = listener.deleteLiveMount(data.toString(), acknowledgment);
        Assert.assertEquals(messageObject.getString("message"), data.toString());
        Assert.assertEquals(messageObject.getString("status"), "SUCCESS");
        Assert.assertEquals(messageObject.getString("job_status"), "SUCCESS");
    }

    /**
     * 测试请求销毁成功消息
     */
    @Test
    public void test_do_live_mount_unmount_success() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        data.set("request_id", UUIDGenerator.getUUID());
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("live_mount.debuts", false);
        data.set("force_delete", "false");
        data.set("mounted_copy", UnmountFlowListenerTestData.getVMWareMountedCopy());

        PowerMockito.doNothing()
            .when(liveMountService)
            .updateLiveMountStatus(ArgumentMatchers.any(), ArgumentMatchers.any());
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.any()))
            .thenReturn(UnmountFlowListenerTestData.getVMWareMountedCopy());
        listener.doLiveMountUnmount(data.toString(), acknowledgment);
        Assert.assertNotNull(listener);
    }
}
