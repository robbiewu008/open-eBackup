/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.service.archive;

import static org.assertj.core.api.BDDAssertions.then;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;
import static org.mockito.Mockito.doNothing;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.powermock.api.mockito.PowerMockito.whenNew;

import com.huawei.LocalRedisConfiguration;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.archive.ArchiveUnifiedRestApi;
import openbackup.data.access.framework.core.common.constants.CopyInfoConstants;
import openbackup.data.access.framework.core.common.constants.TopicConstants;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.mocks.CopyMocker;
import openbackup.data.access.framework.protection.mocks.RepositoryMocker;
import openbackup.data.access.framework.protection.mocks.SlaMock;

import openbackup.data.access.framework.protection.service.context.ContextManager;
import openbackup.data.access.framework.protection.service.job.InternalApiHub;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveObject;
import openbackup.data.protection.access.provider.sdk.archive.v2.ArchiveTask;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFeatureEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.repository.s3.service.S3StorageService;

import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.protection.model.QosBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentCaptor;
import org.mockito.Mockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.modules.junit4.PowerMockRunnerDelegate;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.redisson.spring.starter.RedissonAutoConfiguration;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.boot.test.mock.mockito.MockBeans;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

/**
 * ArchiveTaskManager单元测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/15
 **/
@RunWith(PowerMockRunner.class)
@PrepareForTest(value = {ArchiveTaskManager.class, ArchiveTaskService.class})
@PowerMockRunnerDelegate(SpringRunner.class)
@SpringBootTest(classes = {
    LocalRedisConfiguration.class, RedissonAutoConfiguration.class, ContextManager.class, RedissonClient.class,
    ArchiveTaskService.class, ArchiveUnifiedRestApi.class, ArchiveTaskManager.class, ArchiveRepositoryService.class,
    UserQuotaManager.class
})
@MockBeans(@MockBean(classes = {S3StorageService.class}))
public class ArchiveTaskManagerTest {

    @Autowired
    private ArchiveTaskManager archiveTaskManager;

    @Autowired
    private RedissonClient redissonClient;

    @MockBean
    private ArchiveUnifiedRestApi archiveUnifiedRestApi;

    @MockBean
    private ClusterNativeApi clusterNativeApi;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private QosCommonRestApi qosCommonRestApi;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private NotifyManager notifyManager;

    @MockBean
    private JobService jobService;

    @MockBean
    private RepositoryStrategyManager repositoryStrategyManager;

    @MockBean
    private ProviderManager providerManager;

    @MockBean
    private EncryptorService encryptorService;

    @MockBean
    private BackupStorageApi backupStorageApi;

    @MockBean
    private InternalApiHub internalApiHub;

    @MockBean
    private UserQuotaManager userQuotaManager;

    @MockBean
    private ClusterQueryService clusterQueryService;

    @Before
    public void mockRepository() {
        RepositoryStrategy s3Strategy = mock(RepositoryStrategy.class);
        given(repositoryStrategyManager.getStrategy(eq(RepositoryProtocolEnum.S3))).willReturn(s3Strategy);
        given(s3Strategy.getRepository(any())).willReturn(RepositoryMocker.mockS3Repository());

        RepositoryStrategy tapeStrategy = mock(RepositoryStrategy.class);
        given(repositoryStrategyManager.getStrategy(eq(RepositoryProtocolEnum.TAPE))).willReturn(tapeStrategy);
        given(tapeStrategy.getRepository(any())).willReturn(RepositoryMocker.mockTapRepository());

        RepositoryStrategy nativeNfsStrategy = mock(RepositoryStrategy.class);
        given(repositoryStrategyManager.getStrategy(eq(RepositoryProtocolEnum.NATIVE_NFS))).willReturn(
            nativeNfsStrategy);
        given(nativeNfsStrategy.getAuthentication(any())).willReturn(
            RepositoryMocker.mockNativeNfsRepository().getAuth());
    }

    /**
     * 用例名称：验证参数正确时，执行开始归档任务成功<br/>
     * 前置条件：无<br/>
     * check点：1.调用dme_archive接口一次 2.archiveTask对象字段与期望一致<br/>
     */
    @Test
    public void should_success_when_test_archive_start_given_param_correct() {
        // Given
        final String copyId = UUID.randomUUID().toString();
        final String requestId = UUID.randomUUID().toString();
        // ***** 构造归档任务对象 ****
        ArchiveObject archiveObject = new ArchiveObject();
        archiveObject.setRequestId(requestId);
        archiveObject.setJobId(requestId);
        archiveObject.setCopyId(copyId);
        archiveObject.setObjectType(ResourceSubTypeEnum.NAS_SHARE.getType());
        archiveObject.setPolicy(SlaMock.getArchiveTapePolicy());
        // ***** Mock ArchiveTask 构造函数 ****
        ArchiveTask archiveTask = new ArchiveTask();
        try {
            whenNew(ArchiveTask.class).withNoArguments().thenReturn(archiveTask);
        } catch (Exception e) {
            e.printStackTrace();
        }
        doNothing().when(archiveUnifiedRestApi).createArchiveTask(any());
        // ***** Mock Qos信息 ****
        String qosId = "a24420e1-f6ca-4afc-acee-36e26a7a3d0d";
        final QosBo qosBo = new QosBo();
        qosBo.setUuid(qosId);
        qosBo.setName("test");
        qosBo.setSpeedLimit(10);
        given(internalApiHub.getQosCommonRestApi()).willReturn(qosCommonRestApi);
        given(internalApiHub.getJobService()).willReturn(jobService);
        given(qosCommonRestApi.queryQos(eq(qosId))).willReturn(qosBo);
        given(internalApiHub.getCopyRestApi()).willReturn(copyRestApi);
        given(copyRestApi.queryCopyByID(any(), eq(true))).willReturn(CopyMocker.mockNasCopy());
        RepositoryStrategy nativeNfsStrategy = mock(RepositoryStrategy.class);
        given(repositoryStrategyManager.getStrategy(eq(RepositoryProtocolEnum.NATIVE_NFS))).willReturn(
            nativeNfsStrategy);
        Authentication authentication = new Authentication();
        given(nativeNfsStrategy.getAuthentication(any())).willReturn(authentication);
        // ***** Mock 本地存储 ****
        final ClusterDetailInfo localCluster = RepositoryMocker.mockLocalClusterInfo();
        given(clusterNativeApi.queryDecryptCurrentGroupClusterDetails()).willReturn(localCluster);
        // When
        archiveTaskManager.start(archiveObject);
        // Then
        Mockito.verify(archiveUnifiedRestApi, times(1)).createArchiveTask(eq(archiveTask));
        then(archiveTask.getRequestId()).isNotNull().isEqualTo(requestId);
        then(archiveTask.getTaskId()).isNotNull().isEqualTo(requestId);
        then(archiveTask.getOriginCopyId()).isNotNull().isEqualTo(copyId);
        then(archiveTask.getDataLayout()).isNull();
        then(archiveTask.getQos()).isNotNull().hasFieldOrPropertyWithValue("bandwidth", 10);

        then(archiveTask.getRepositories().get(1)).isNotNull()
            .matches(item -> item.getExtendInfo().get("esn").equals(localCluster.getStorageSystem().getStorageEsn()));
        then(archiveTask.getRepositories()).isNotEmpty()
            .hasSize(2)
            .anyMatch(
                repository -> repository.getProtocol() == 7 && !repository.getLocal() && repository.getAuth() != null
                    && repository.getEndpoint() != null && repository.getId()
                    .equals("f8b38f61-9c09-4c05-8caa-03b38b881242"));
    }

    /**
     * 用例名称：验证参数正确时，执行执行归档任务成功处理成功<br/>
     * 前置条件：无<br/>
     * check点：1.生成的归档副本信息与期望一致 2.调用发送归档完成kafka消息一次<br/>
     */
    @Test
    public void should_send_archive_done_message_success_when_archiveSuccess_given_status_success() {
        // Given
        final String requestId = UUID.randomUUID().toString();
        final Integer protocol = getRandomProtocol();
        mockArchiveSuccessContext(requestId, protocol);
        given(internalApiHub.getCopyRestApi()).willReturn(copyRestApi);
        given(internalApiHub.getJobService()).willReturn(jobService);
        given(copyRestApi.queryCopyByID(eq("test_original_132323232"), eq(true))).willReturn(CopyMocker.mockNasCopy());
        given(copyRestApi.saveCopy(any())).willReturn(new UuidObject("test_archive_132323232"));
        CopyInfo copyInfo = new CopyInfo();
        try {
            whenNew(CopyInfo.class).withNoArguments().thenReturn(copyInfo);
        } catch (Exception e) {
            e.printStackTrace();
        }
        doNothing().when(notifyManager).send(eq(TopicConstants.TASK_ARCHIVE_DONE_TOPIC), any());
        doNothing().when(jobService).updateJobStatus(any(), any());
        // When
        archiveTaskManager.archiveSuccess(requestId, DmcJobStatus.SUCCESS, new HashMap());
        // Then
        Mockito.verify(notifyManager, times(1)).send(eq(TopicConstants.TASK_ARCHIVE_DONE_TOPIC), any());
        then(copyInfo.getUuid()).isEqualTo("test_archive_132323232");
        then(copyInfo.getChainId()).isNotBlank();
        then(copyInfo.getParentCopyUuid()).isEqualTo("test_original_132323232");
        then(copyInfo.getStatus()).isEqualTo(CopyStatus.NORMAL.getValue());
        then(copyInfo.getIsReplicated()).isFalse();
        then(copyInfo.getIsArchived()).isTrue();
        then(copyInfo.getDisplayTimestamp()).isNotBlank();
        then(copyInfo.getGeneratedTime()).isNotBlank();
        then(copyInfo.getIndexed()).isEqualTo(CopyInfoConstants.COPY_INIT_INDEXED);
        then(copyInfo.getFeatures()).isEqualTo(
            CopyFeatureEnum.setAndGetFeatures(Lists.newArrayList(CopyFeatureEnum.RESTORE)));
        then(copyInfo.getSlaName()).isEqualTo("testaq");
        then(copyInfo.getSlaProperties()).isEqualTo(SlaMock.getSla());
        then(copyInfo.getRetentionType()).isNotNull();
        then(copyInfo.getGeneratedBy()).isNotNull().isEqualTo(selectCopyGenerateType(protocol).value());
    }

    /**
     * 用例名称：验证参数正确时，执行执行归档任务成功处理成功<br/>
     * 前置条件：无<br/>
     * check点：1.生成的归档副本信息与期望一致 2.调用发送归档完成kafka消息一次<br/>
     */
    @Test
    public void should_send_archive_done_message_success_when_archiveFailed_given_status_not_success() {
        // Given
        final String requestId = UUID.randomUUID().toString();
        final Integer protocol = getRandomProtocol();
        mockArchiveSuccessContext(requestId, protocol);
        // When
        archiveTaskManager.archiveFailed(requestId, DmcJobStatus.FAIL);
        // Then
        Mockito.verify(notifyManager, times(1)).send(eq(TopicConstants.TASK_ARCHIVE_DONE_TOPIC), any());
    }

    /**
     * 用例名称：执行执行归档任务成功处理成功,FC归档副本索引状态为“未索引”<br/>
     * 前置条件：无<br/>
     * check点：FC归档副本索引状态为“未索引”<br/>
     */
    @Test
    public void test_FC_archive_copy() {
        // Given
        final String requestId = UUID.randomUUID().toString();
        final Integer protocol = getRandomProtocol();
        mockArchiveSuccessContext(requestId, protocol);
        given(internalApiHub.getCopyRestApi()).willReturn(copyRestApi);
        given(internalApiHub.getJobService()).willReturn(jobService);
        Copy copy = CopyMocker.mockCommonCopy();
        copy.setResourceSubType(ResourceSubTypeEnum.FUSION_COMPUTE.getType());
        given(copyRestApi.queryCopyByID(eq("test_original_132323232"), eq(true))).willReturn(copy);
        // When
        archiveTaskManager.archiveSuccess(requestId, DmcJobStatus.SUCCESS, new HashMap());
        ArgumentCaptor<CopyInfo> argumentCaptor = ArgumentCaptor.forClass(CopyInfo.class);
        Mockito.verify(copyRestApi, Mockito.times(1)).saveCopy(argumentCaptor.capture());
        CopyInfo copyInfo = argumentCaptor.getValue();
        // Then
        Assert.assertEquals(copyInfo.getIndexed(), CopyIndexStatus.UNSUPPORT.getIndexStaus());
    }

    /**
     * 用例名称：执行执行归档任务成功处理成功,归档副本保存原副本时间
     * 前置条件：无
     * check点：归档副本保存原副本时间
     */
    @Test
    public void test_archive_copy_set_origin_time() {
        // Given
        final String requestId = UUID.randomUUID().toString();
        final Integer protocol = getRandomProtocol();
        mockArchiveSuccessContext(requestId, protocol);
        given(internalApiHub.getCopyRestApi()).willReturn(copyRestApi);
        given(internalApiHub.getJobService()).willReturn(jobService);
        Copy copy = CopyMocker.mockCommonCopy();
        given(copyRestApi.queryCopyByID(eq("test_original_132323232"), eq(true))).willReturn(copy);
        // When
        archiveTaskManager.archiveSuccess(requestId, DmcJobStatus.SUCCESS, new HashMap());
        ArgumentCaptor<CopyInfo> argumentCaptor = ArgumentCaptor.forClass(CopyInfo.class);
        Mockito.verify(copyRestApi, Mockito.times(1)).saveCopy(argumentCaptor.capture());
        CopyInfo copyInfo = argumentCaptor.getValue();
        // Then
        Assert.assertEquals(copyInfo.getOriginCopyTimeStamp(), String.valueOf(Long.parseLong(copy.getTimestamp()) / 1000));

        copy.setOriginCopyTimeStamp("2023-10-20 15:44:56");
        given(copyRestApi.queryCopyByID(eq("test_original_132323232"), eq(true))).willReturn(copy);
        archiveTaskManager.archiveSuccess(requestId, DmcJobStatus.SUCCESS, new HashMap());
        argumentCaptor = ArgumentCaptor.forClass(CopyInfo.class);
        Mockito.verify(copyRestApi, Mockito.times(2)).saveCopy(argumentCaptor.capture());
        copyInfo = argumentCaptor.getValue();
        // Then
        StorageRepository repository = RepositoryMocker.mockS3Repository();
        Assert.assertEquals(copyInfo.getLocation(),
            repository.getPath().concat("(").concat(repository.getEndpoint().getIp()).concat(")"));
    }

    /**
     * 用例名称：执行执行归档任务成功处理成功,对象归档副本位置为：桶名称(endpoint)<br/>
     * 前置条件：无<br/>
     * check点：对象归档副本位置为：桶名称(endpoint)<br/>
     */
    @Test
    public void test_archive_copy_location() {
        // Given
        final String requestId = UUID.randomUUID().toString();
        final int protocol = RepositoryProtocolEnum.S3.getProtocol();
        mockArchiveSuccessContext(requestId, protocol);
        given(internalApiHub.getCopyRestApi()).willReturn(copyRestApi);
        given(internalApiHub.getJobService()).willReturn(jobService);
        Copy copy = CopyMocker.mockCommonCopy();
        // final S3Storage mockS3 = RepositoryMocker.mockS3StorageFullInfo();
        copy.setResourceSubType(ResourceSubTypeEnum.FUSION_COMPUTE.getType());
        given(copyRestApi.queryCopyByID(eq("test_original_132323232"), eq(true))).willReturn(copy);
        // When
        archiveTaskManager.archiveSuccess(requestId, DmcJobStatus.SUCCESS, new HashMap());
        ArgumentCaptor<CopyInfo> argumentCaptor = ArgumentCaptor.forClass(CopyInfo.class);
        Mockito.verify(copyRestApi, Mockito.times(1)).saveCopy(argumentCaptor.capture());
        CopyInfo copyInfo = argumentCaptor.getValue();
        // Then
        Assert.assertEquals(copyInfo.getIndexed(), CopyIndexStatus.UNSUPPORT.getIndexStaus());
    }

    private void mockArchiveSuccessContext(String requestId, int protocol) {
        final RMap<String, String> contextMap = redissonClient.getMap(requestId, StringCodec.INSTANCE);
        contextMap.put(ArchiveContext.ORIGINAL_COPY_ID_KEY, "test_original_132323232");
        contextMap.put(ArchiveContext.ARCHIVE_COPY_ID_KEY, "test_archive_132323232");
        if (RepositoryProtocolEnum.getByProtocol(protocol).equals(RepositoryProtocolEnum.S3)) {
            contextMap.put(ArchiveContext.ARCHIVE_POLICY_KEY, SlaMock.getArchiveS3Policy());
        } else {
            contextMap.put(ArchiveContext.ARCHIVE_POLICY_KEY, SlaMock.getArchiveTapePolicy());
        }
        contextMap.put(ArchiveContext.JOB_ID, "test_job_132323232");
        contextMap.put(ArchiveContext.AUTO_RETRY_TIMES, "5");
        contextMap.put(ArchiveContext.SLA_KEY, SlaMock.getSla());
    }

    private Integer getRandomProtocol() {
        List<Integer> protocolList = Arrays.asList(RepositoryProtocolEnum.S3.getProtocol(),
            RepositoryProtocolEnum.TAPE.getProtocol());
        return protocolList.get(Double.valueOf(Math.random() * protocolList.size()).intValue());
    }

    private CopyGeneratedByEnum selectCopyGenerateType(int protocol) {
        switch (RepositoryProtocolEnum.getByProtocol(protocol)) {
            case S3:
                return CopyGeneratedByEnum.BY_CLOUD_ARCHIVE;
            case TAPE:
                return CopyGeneratedByEnum.BY_TAPE_ARCHIVE;
            default:
                throw new IllegalArgumentException("function not support this protocol");
        }
    }
}
