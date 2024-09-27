/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.livemount.listener.livemount;

import static org.assertj.core.api.AssertionsForClassTypes.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.ArgumentMatchers.isNull;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.when;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.framework.livemount.TopicConstants;
import openbackup.data.access.framework.livemount.common.LiveMountRestApi;
import openbackup.data.access.framework.livemount.common.enums.RetentionType;
import openbackup.data.access.framework.livemount.common.enums.RetentionUnit;
import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.access.framework.livemount.dao.LiveMountEntityDao;
import openbackup.data.access.framework.livemount.dao.LiveMountPolicyEntityDao;
import openbackup.data.access.framework.livemount.data.MountFlowListenerTestData;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.data.access.framework.livemount.listener.MountFlowListener;
import openbackup.data.access.framework.livemount.provider.LiveMountFlowService;
import openbackup.data.access.framework.livemount.service.LiveMountService;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.cluster.BackupClusterConfigUtil;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.license.LicenseValidateService;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.kafka.MessageObject;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyRetentionPolicy;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.resource.EnvironmentRestApi;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.ScheduleRestApi;
import openbackup.system.base.util.MessageTemplate;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.Codec;
import org.redisson.client.codec.StringCodec;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.kafka.support.Acknowledgment;
import org.springframework.test.context.ContextConfiguration;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Mount Flow Listener Test
 *
 * @author l00272247
 * @since 2020-12-29
 */
@RunWith(PowerMockRunner.class)
@SpringBootTest
@ContextConfiguration(classes = MountFlowListener.class)
@PrepareForTest({BackupClusterConfigUtil.class})
public class MountFlowListenerTest {
    /**
     * live mount id
     */
    private static final String LIVE_MOUNT_ID = "live_mount_id";

    /**
     * copy ids
     */
    private static final String COPY_IDS = "copy_ids";

    /**
     * job id
     */
    private static final String JOB_ID = "job_id";

    /**
     * source copy
     */
    private static final String SOURCE_COPY = "source_copy";

    /**
     * live mount
     */
    private static final String LIVE_MOUNT = "live_mount";

    /**
     * mounted copy
     */
    private static final String MOUNTED_COPY = "mounted_copy";

    /**
     * policy
     */
    private static final String POLICY = "policy";

    /**
     * request id
     */
    private static final String REQUEST_ID = "request_id";

    /**
     * job status
     */
    private static final String JOB_STATUS = "job_status";

    /**
     * live mount debuts
     */
    private static final String LIVE_MOUNT_DEBUTS = "live_mount.debuts";

    /**
     * ExpectedException
     */
    @Rule
    public final ExpectedException expectedException = ExpectedException.none();

    @InjectMocks
    @Autowired
    private MountFlowListener listener;

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

    @Mock
    private ScheduleRestApi scheduleRestApi;

    @Mock
    private LicenseValidateService licenseValidateService;

    @Mock
    private ProviderRegistry providerRegistry;

    @Mock
    private MemberClusterService memberClusterService;

    @Mock
    private JobService jobService;

    /**
     * 测试挂载准备步骤
     * Mount Is Not Null
     */
    @Test
    public void testRequestLiveMountExecuteMountIsNotNull() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        JSONObject data = new JSONObject();
        data.set(REQUEST_ID, "request_id");
        data.set(LIVE_MOUNT_DEBUTS, false);
        data.set(LIVE_MOUNT, new LiveMountEntity());
        data.set(SOURCE_COPY, new Copy());
        data.set(MOUNTED_COPY, new Copy());
        testRequestLiveMountExecuteSame(data);
        org.junit.Assert.assertNotNull(data);
    }

    /**
     * 测试挂载准备步骤
     * Mount Is Null
     */
    @Test
    public void testRequestLiveMountExecute() {
        JSONObject data = new JSONObject();
        data.set(REQUEST_ID, "request_id");
        data.set(LIVE_MOUNT_DEBUTS, false);
        data.set(LIVE_MOUNT, new LiveMountEntity());
        data.set(SOURCE_COPY, new Copy());
        testRequestLiveMountExecuteSame(data);
        org.junit.Assert.assertNotNull(data);
    }

    /**
     * testRequestLiveMountExecute Same Code
     *
     * @param data JSONObject
     */
    private void testRequestLiveMountExecuteSame(JSONObject data) {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(map.put(any(), any())).thenReturn(null);
        PowerMockito.when(
            redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.doNothing().when(liveMountService).checkTargetEnvironmentStatus(any());
        PowerMockito.doNothing().when(liveMountService).updateLiveMountStatus(any(), any());
        PowerMockito.doNothing().when(copyRestApi).updateCopyStatus(any(), any());
        PowerMockito.doNothing().when(jobCenterRestApi).updateJob(any(), any());
        PowerMockito.when(messageTemplate.send(ArgumentMatchers.anyString(), any(JSONObject.class))).thenReturn(null);
        listener.requestLiveMountExecute(data.toString(), getAcknowledgment());
    }

    /**
     * unmountBeforeExecute
     */
    @Test
    public void unmountBeforeExecute() {
        JSONObject jsonObject = new JSONObject();
        Copy copy = new Copy();
        jsonObject.set(REQUEST_ID, "1");
        jsonObject.set(JOB_ID, "1");
        jsonObject.set(MOUNTED_COPY, JSONObject.fromObject(copy).toString());
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        jsonObject.set(LIVE_MOUNT, JSONObject.fromObject(liveMountEntity).toString());
        jsonObject.set(POLICY, "1");
        jsonObject.set(MOUNTED_COPY, JSONObject.fromObject(copy).toString());
        listener.unmountBeforeExecute(jsonObject.toString(), getAcknowledgment());
        org.junit.Assert.assertNotNull(listener);
    }

    /**
     * processLiveMountUnmountBeforeExecute  jobStatus 不是 success
     */
    @Test
    public void processLiveMountUnmountBeforeExecuteFail() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(JOB_STATUS, JobStatusEnum.RUNNING);
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("unmount failed");
        PowerMockito.when(jobService.isJobPresent(isNull())).thenReturn(true);
        listener.processLiveMountUnmountBeforeExecute(jsonObject.toString(), getAcknowledgment());
    }

    /**
     * 用例名称：验证修改即时挂载前置任务成功。<br/>
     * 前置条件：参数输入正常。<br/>
     * check点：执行完毕无异常抛出。
     */
    @Test
    public void test_live_mount_file_system_name_changed_success() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(JOB_STATUS, JobStatusEnum.SUCCESS);
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setResourceSubType("Fileset");
        List<LiveMountFileSystemShareInfo> liveMountFileSystemShareInfos = new ArrayList<>();
        LiveMountFileSystemShareInfo systemShareInfo = new LiveMountFileSystemShareInfo();
        systemShareInfo.setFileSystemName("fileset_123");
        liveMountFileSystemShareInfos.add(systemShareInfo);
        liveMountEntity.setFileSystemShareInfo(JSONArray.fromObject(liveMountFileSystemShareInfos).toString());
        jsonObject.set("live_mount", liveMountEntity);
        Copy copy = new Copy();
        copy.setUuid("uuid");
        copy.setResourceSubType("Fileset");

        jsonObject.set("mounted_copy", copy);
        PowerMockito.doNothing().when(copyRestApi).updateCopyStatus(anyString(), any());
        PowerMockito.doNothing().when(liveMountService).cleanMountedCopyInfo(any());
        jsonObject.set("source_copy", copy);

        listener.processLiveMountUnmountBeforeExecute(jsonObject.toString(), getAcknowledgment());

        LiveMountEntity afterLiveMountEntity = JSONObject.fromObject(jsonObject.get("live_mount"))
            .toBean(LiveMountEntity.class);
        JSONArray jsonArray = JSONArray.fromObject(afterLiveMountEntity.getFileSystemShareInfo());
        List<LiveMountFileSystemShareInfo> fileSystemShareInfo = JSONArray.toCollection(jsonArray,
            LiveMountFileSystemShareInfo.class);
        Assert.assertTrue(!fileSystemShareInfo.get(0).getFileSystemName().contains("mount_"));
    }

    /**
     * processLiveMountUnmountBeforeExecute  jobStatus 是 success
     */
    @Test
    public void processLiveMountUnmountBeforeExecute() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(JOB_STATUS, JobStatusEnum.SUCCESS);
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setResourceSubType("1");
        jsonObject.set(LIVE_MOUNT, JSONObject.fromObject(liveMountEntity).toString());
        Copy copy = new Copy();
        copy.setResourceType(ResourceSubTypeEnum.ORACLE.getType());
        jsonObject.set(MOUNTED_COPY, JSONObject.fromObject(copy).toString());
        LiveMountPolicyEntity liveMountPolicyEntity = new LiveMountPolicyEntity();
        jsonObject.set(POLICY, liveMountPolicyEntity);
        jsonObject.set(REQUEST_ID, "1");
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(providerRegistry.findProvider(any(), any())).thenReturn(liveMountFlowService);
        PowerMockito.when(liveMountClientRestApi.refreshTargetResource(any())).thenReturn(Arrays.asList("1"));
        Assert.assertNotNull(jsonObject);
    }

    /**
     * processUnmountBeforeExecuteFailed
     */
    @Test
    public void processUnmountBeforeExecuteFailed() {
        JSONObject jsonObject = new JSONObject();
        Copy copy = new Copy();
        jsonObject.set(SOURCE_COPY, JSONObject.fromObject(copy).toString());
        MessageObject messageObject = listener.processUnmountBeforeExecuteFailed(jsonObject.toString(), getAcknowledgment());
        Assert.assertEquals(messageObject.getString("topic"), TopicConstants.LIVE_MOUNT_COMPLETE_PROCESS);
        Assert.assertEquals(messageObject.getString("message"), jsonObject.toString());
    }

    /**
     * 测试挂载准备步骤
     */
    @Test
    public void testProcessLiveMountCopyClone() {
        JSONObject data = new JSONObject();
        data.set("request_id", "request_id");
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        Copy cloneCopy = MountFlowListenerTestData.getCloneCopy();
        PowerMockito.when(liveMountClientRestApi.cloneCopy(any())).thenReturn(cloneCopy);
        PowerMockito.when(copyRestApi.saveCopy(any())).thenReturn(MountFlowListenerTestData.getSaveCopyUuidObject());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MountFlowListenerTestData.getSavedCloneCopy());
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(liveMountFlowService.getCloneCopyFeatureByResourceSubType(anyInt())).thenReturn(1);
        PowerMockito.when(providerRegistry.findProvider(any(), any())).thenReturn(liveMountFlowService);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
            redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
        listener.processLiveMountCopyClone(data.toString(), getAcknowledgment());
        org.junit.Assert.assertEquals("Unsupport", cloneCopy.getIndexed());
    }

    /**
     * 下发挂载任务
     */
    @Test
    public void processLiveMountExecute() {
        JSONObject jsonObject = new JSONObject();
        Copy copy = new Copy();
        copy.setResourceSubType("1");
        jsonObject.set(SOURCE_COPY, JSONObject.fromObject(copy).toString());
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        jsonObject.set(LIVE_MOUNT, JSONObject.fromObject(liveMountEntity).toString());
        jsonObject.set(POLICY, "1");
        jsonObject.set(MOUNTED_COPY, JSONObject.fromObject(copy).toString());
        listener.processLiveMountExecute(jsonObject.toString(), getAcknowledgment());
        Assert.assertNotNull(listener);
    }

    /**
     * 下发挂载任务（非克隆副本）
     */
    @Test
    public void processLiveMountExecuteWithoutCloneCopy() {
        JSONObject jsonObject = new JSONObject();
        Copy copy = new Copy();
        copy.setResourceSubType("1");
        jsonObject.set(SOURCE_COPY, JSONObject.fromObject(copy).toString());
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        jsonObject.set(LIVE_MOUNT, JSONObject.fromObject(liveMountEntity).toString());
        jsonObject.set(POLICY, "1");
        jsonObject.set(MOUNTED_COPY, JSONObject.fromObject(copy).toString());
        listener.processLiveMountExecute(jsonObject.toString(), getAcknowledgment());
        Assert.assertNotNull(listener);
    }

    /**
     * 测试挂载，资源被锁，挂载失败
     */
    @Test
    public void test_mount_resource_lock_fail() {
        JSONObject data = new JSONObject();
        data.set("request_id", UUIDGenerator.getUUID());
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        data.set("job_status", "SUCCESS");

        RMap rMap = mock(RMap.class);
        rMap.put("lock", new JSONObject().set("status", "fail"));
        PowerMockito.when(redissonClient.getMap(any(), (Codec) any())).thenReturn(rMap);

        PowerMockito.doNothing().when(liveMountService).updateLiveMountStatus(any(), any());
        org.junit.Assert.assertThrows(NullPointerException.class,
            () -> listener.completeLiveMountExecute(data.toString(), getAcknowledgment()));
    }

    /**
     * scheduleLiveMount
     * hasActive is false
     */
    @Test
    public void scheduleLiveMountFalse() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(LIVE_MOUNT_ID, "1");
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setEnableStatus("1");
        when(liveMountEntityDao.selectById(any())).thenReturn(liveMountEntity);
        when(liveMountService.checkHasActive("1", false)).thenReturn(false);
        listener.scheduleLiveMount(jsonObject.toString(), getAcknowledgment());
        Mockito.verify(liveMountEntityDao, times(1)).selectById(any());
        Mockito.verify(liveMountService, times(1)).checkHasActive("1", false);
    }

    /**
     * scheduleLiveMount
     * hasActive is true
     * policyId is null
     */
    @Test
    public void scheduleLiveMountPolicyIsNull() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(LIVE_MOUNT_ID, "1");
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setEnableStatus("1");
        when(liveMountEntityDao.selectById(any())).thenReturn(liveMountEntity);
        when(liveMountService.checkHasActive("1", false)).thenReturn(true);
        listener.scheduleLiveMount(jsonObject.toString(), getAcknowledgment());
        Mockito.verify(liveMountEntityDao, times(1)).selectById(any());
        Mockito.verify(liveMountService, times(1)).checkHasActive("1", false);
    }

    /**
     * scheduleLiveMount policyId is null
     */
    @Test
    public void scheduleLiveMount() {
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(LIVE_MOUNT_ID, "1");
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setEnableStatus("1");
        liveMountEntity.setPolicyId("1");
        when(liveMountEntityDao.selectById(any())).thenReturn(liveMountEntity);
        when(liveMountService.checkHasActive("1", false)).thenReturn(true);
        listener.scheduleLiveMount(jsonObject.toString(), getAcknowledgment());
        Mockito.verify(liveMountEntityDao, times(1)).selectById(any());
        Mockito.verify(liveMountService, times(1)).checkHasActive("1", false);
    }

    /**
     * processProtectionBackupDone
     * copyIds is null
     */
    @Test
    public void processProtectionBackupDoneCopyIdsIsNull() {
        listener.processProtectionBackupDone(new JSONObject().toString(), getAcknowledgment());
        Assert.assertNotNull(listener);
    }

    /**
     * processProtectionBackupDone
     * copyIds is ""
     */
    @Test
    public void processProtectionBackupDoneCopyIdsEmpty() {
        Copy copy = new Copy();
        copy.setResourceId("1");
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        listener.processProtectionBackupDone(new JSONObject().set(COPY_IDS, "").toString(), getAcknowledgment());
        Assert.assertNotNull(listener);
    }

    /**
     * processProtectionBackupDone
     * copyIds is "1"
     * hasActive is false
     */
    @Test
    public void processProtectionBackupDoneCopyIdsIsOneHasNoActive() {
        Copy copy = new Copy();
        copy.setResourceId("1");
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setEnableStatus("1");
        when(liveMountEntityDao.queryAutoUpdateWhenBackupDoneLiveMounts("1")).thenReturn(
            Arrays.asList(liveMountEntity));
        when(liveMountService.checkHasActive(liveMountEntity.getEnableStatus(), false)).thenReturn(true);
        listener.processProtectionBackupDone(new JSONObject().set(COPY_IDS, "1").toString(), getAcknowledgment());
        Mockito.verify(copyRestApi, times(1)).queryCopyByID(any());
    }

    /**
     * processProtectionBackupDone
     * copyIds is "1"
     * hasActive is true
     * PolicyId is null
     */
    @Test
    public void processProtectionBackupDoneCopyIdsIsOnePolicyIdIsNull() {
        Copy copy = new Copy();
        copy.setResourceId("1");
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setEnableStatus("1");
        when(liveMountEntityDao.queryAutoUpdateWhenBackupDoneLiveMounts("1")).thenReturn(
            Arrays.asList(liveMountEntity));
        when(liveMountService.checkHasActive(liveMountEntity.getEnableStatus(), false)).thenReturn(true);
        listener.processProtectionBackupDone(new JSONObject().set(COPY_IDS, "1").toString(), getAcknowledgment());
        Mockito.verify(copyRestApi, times(1)).queryCopyByID(any());
    }

    /**
     * processProtectionBackupDone
     * copyIds is "1"
     * hasActive is true
     * PolicyId is not null
     */
    @Test
    public void processProtectionBackupDone() {
        Copy copy = new Copy();
        copy.setResourceId("1");
        when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setEnableStatus("1");
        liveMountEntity.setPolicyId("1");
        when(liveMountEntityDao.queryAutoUpdateWhenBackupDoneLiveMounts("1")).thenReturn(
            Arrays.asList(liveMountEntity));
        when(liveMountService.checkHasActive(liveMountEntity.getEnableStatus(), false)).thenReturn(true);
        listener.processProtectionBackupDone(new JSONObject().set(COPY_IDS, "1").toString(), getAcknowledgment());
        Mockito.verify(liveMountEntityDao, times(1)).queryAutoUpdateWhenBackupDoneLiveMounts("1");
        Mockito.verify(liveMountService, times(1)).checkHasActive(liveMountEntity.getEnableStatus(), false);
    }

    /**
     * getAcknowledgment
     *
     * @return Acknowledgment
     */
    private Acknowledgment getAcknowledgment() {
        Acknowledgment acknowledgment = PowerMockito.mock(Acknowledgment.class);
        acknowledgment.acknowledge();
        PowerMockito.doNothing().when(acknowledgment).acknowledge();
        return acknowledgment;
    }

    /**
     * clean copy by policy only live mount success
     *
     * @throws InvocationTargetException InvocationTargetException
     * @throws IllegalAccessException IllegalAccessException
     */
    @Test
    public void cleanCopyByPolicyOnlyMountSuccess() throws InvocationTargetException, IllegalAccessException {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setPolicyId("3434343");

        LiveMountPolicyEntity policy = new LiveMountPolicyEntity();
        policy.setRetentionPolicy(RetentionType.FIXED_TIME.getName());
        policy.setRetentionUnit(RetentionUnit.DAY.getName());
        policy.setRetentionValue(3);

        JSONObject json = new JSONObject();
        json.set("request_id", UUIDGenerator.getUUID());

        Copy lastedCopy = new Copy();
        Copy mountedCopy = new Copy();
        mountedCopy.setResourceId("34343434");
        mountedCopy.setChainId("34343");

        BasePage<Copy> basePage = new BasePage<>();
        basePage.setItems(Collections.singletonList(lastedCopy));

        JobStatusEnum status = JobStatusEnum.SUCCESS;

        when(copyRestApi.queryCopies(anyInt(), anyInt(), anyMap())).thenReturn(basePage);
        when(liveMountPolicyEntityDao.selectPolicy(anyString())).thenReturn(policy);
        when(copyRestApi.updateCopyRetention(anyString(), any(CopyRetentionPolicy.class))).thenReturn(true);
        Method method = PowerMockito.method(MountFlowListener.class, "cleanCopyByPolicyOnlyMountSuccess",
            JSONObject.class, JobStatusEnum.class, LiveMountEntity.class, Copy.class);
        method.invoke(listener, json, status, liveMountEntity, mountedCopy);
        Assert.assertNotNull(mountedCopy);
    }

    /**
     * test initial schedule
     *
     * @throws InvocationTargetException InvocationTargetException
     * @throws IllegalAccessException IllegalAccessException
     */
    @Test
    public void test_initial_schedule_success() throws InvocationTargetException, IllegalAccessException {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setPolicyId("3434343");

        LiveMountPolicyEntity policy = new LiveMountPolicyEntity();
        policy.setRetentionPolicy(RetentionType.FIXED_TIME.getName());
        policy.setRetentionUnit(RetentionUnit.DAY.getName());
        policy.setRetentionValue(3);

        String requestId = UUIDGenerator.getUUID();

        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
            redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);

        Method method = PowerMockito.method(MountFlowListener.class, "initialSchedule", LiveMountEntity.class,
            String.class);
        method.invoke(listener, liveMountEntity, requestId);
        Mockito.verify(redissonClient, times(1)).getMap(ArgumentMatchers.anyString(),
                ArgumentMatchers.eq(StringCodec.INSTANCE));
    }

    @Test
    public void should_setDeviceEsnSuccess_when_clone_copy_given_resourceSubTypeNotDws() {
        JSONObject data = new JSONObject();
        data.set("request_id", "request_id");
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        Copy cloneCopy = MountFlowListenerTestData.getCloneCopy();
        cloneCopy.setResourceSubType(ResourceSubTypeEnum.ORACLE.getType());
        PowerMockito.when(liveMountClientRestApi.cloneCopy(any())).thenReturn(cloneCopy);
        PowerMockito.when(copyRestApi.saveCopy(any())).thenReturn(MountFlowListenerTestData.getSaveCopyUuidObject());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MountFlowListenerTestData.getSavedCloneCopy());
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(liveMountFlowService.getCloneCopyFeatureByResourceSubType(anyInt())).thenReturn(1);
        PowerMockito.when(providerRegistry.findProvider(any(), any())).thenReturn(liveMountFlowService);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.mockStatic(BackupClusterConfigUtil.class);
        PowerMockito.when(BackupClusterConfigUtil.getBackupClusterRole())
                .thenReturn(ClusterEnum.BackupRoleTypeEnum.PRIMARY.getBackupRoleType());
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("123");
        PowerMockito.when(jobService.isJobPresent(anyString())).thenReturn(true);
        listener.processLiveMountCopyClone(data.toString(), getAcknowledgment());
        assertThat(cloneCopy.getDeviceEsn()).isEqualTo("123");
    }

    @Test
    public void should_skipSetDeviceEsnSuccess_when_clone_copy_given_resourceSubTypeIsDws() {
        JSONObject data = new JSONObject();
        data.set("request_id", "request_id");
        data.set("live_mount", new LiveMountEntity());
        data.set("source_copy", new Copy());
        data.set("mounted_copy", new Copy());
        data.set("live_mount.debuts", false);
        Copy cloneCopy = MountFlowListenerTestData.getCloneCopy();
        cloneCopy.setResourceSubType(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType());
        PowerMockito.when(liveMountClientRestApi.cloneCopy(any())).thenReturn(cloneCopy);
        PowerMockito.when(copyRestApi.saveCopy(any())).thenReturn(MountFlowListenerTestData.getSaveCopyUuidObject());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(MountFlowListenerTestData.getSavedCloneCopy());
        LiveMountFlowService liveMountFlowService = PowerMockito.mock(LiveMountFlowService.class);
        PowerMockito.when(liveMountFlowService.getCloneCopyFeatureByResourceSubType(anyInt())).thenReturn(1);
        PowerMockito.when(providerRegistry.findProvider(any(), any())).thenReturn(liveMountFlowService);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(
                        redissonClient.getMap(ArgumentMatchers.anyString(), eq(StringCodec.INSTANCE)))
                .thenReturn(map);
        PowerMockito.mockStatic(BackupClusterConfigUtil.class);
        PowerMockito.when(BackupClusterConfigUtil.getBackupClusterRole())
                .thenReturn(ClusterEnum.BackupRoleTypeEnum.PRIMARY.getBackupRoleType());
        PowerMockito.when(BackupClusterConfigUtil.getBackupClusterEsn()).thenReturn("123");
        listener.processLiveMountCopyClone(data.toString(), getAcknowledgment());
        assertThat(cloneCopy.getDeviceEsn()).isNull();
    }
}