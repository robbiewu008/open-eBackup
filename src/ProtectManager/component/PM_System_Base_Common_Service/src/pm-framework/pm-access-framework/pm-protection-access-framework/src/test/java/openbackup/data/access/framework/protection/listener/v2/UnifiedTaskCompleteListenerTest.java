/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.protection.listener.v2;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.backup.handler.v2.UnifiedBackupTaskCompleteHandler;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.common.enums.DmcJobStatus;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.dto.TaskCompleteMessageDto;
import openbackup.data.access.framework.protection.handler.TaskCompleteHandler;
import openbackup.data.access.framework.protection.listener.v2.UnifiedTaskCompleteListener;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.service.RedissonService;
import openbackup.system.base.util.ProviderRegistry;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.redisson.api.RMap;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.ContextConfiguration;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

/**
 * UnifiedTaskCompleteListener LLT
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-10
 */
@SpringBootTest
@RunWith(SpringRunner.class)
@ContextConfiguration(
        classes = {UnifiedBackupTaskCompleteHandler.class, UnifiedTaskCompleteListener.class, ProviderRegistry.class})
@MockBean({JobService.class, ProviderManager.class})
public class UnifiedTaskCompleteListenerTest {
    /**
     * RES_JSON
     */
    public static final String RES_JSON =
            "{\"name\":\"test_mzx\",\"template_id\":null,\"template_name\":null,\"paths\":[\"/home/ctt/.bash_logout\"],\"filters\":[],\"path\":\"localhost.localdomain\",\"root_uuid\":\"3f38568b0342bbf4557d2edc0261b565\",\"parent_name\":null,\"parent_uuid\":null,\"children_uuids\":null,\"type\":\"Fileset\",\"sub_type\":\"Fileset\",\"uuid\":\"e7e2191e-7220-4107-a0ca-5d06526387b4\",\"created_time\":\"2021-12-09T10:01:19.283983\",\"ext_parameters\":{\"before_protect_script\":\"/test.sh\",\"after_protect_script\":\"/test.sh\",\"protect_failed_script\":\"/test.sh\"},\"authorized_user\":null,\"user_id\":null,\"version\":null,\"sla_id\":\"8556bb41-abe6-4821-870d-a0252f304dfc\",\"sla_name\":\"Gold\",\"sla_status\":true,\"sla_compliance\":false,\"protection_status\":1,\"environment_uuid\":\"3f38568b0342bbf4557d2edc0261b565\",\"environment_name\":\"localhost.localdomain\",\"environment_endpoint\":\"192.168.97.37\",\"environment_os_type\":\"linux\",\"environment_type\":\"Host\",\"environment_sub_type\":\"ABBackupClient\",\"environment_is_cluster\":\"False\",\"environment_os_name\":\"Linux"
                + " el7\"}";

    /**
     * SLA_JSON
     */
    public static final String SLA_JSON =
            "{\"name\":\"Gold\",\"type\":1,\"application\":\"Common\",\"created_time\":\"9999-01-09T20:21:32.842417\",\"uuid\":\"8556bb41-abe6-4821-870d-a0252f304dfc\",\"is_global\":true,\"policy_list\":[{\"uuid\":\"3bd9e448-816d-43b1-9b16-1feca34ece65\",\"name\":\"full\",\"action\":\"full\",\"ext_parameters\":{},\"retention\":{\"retention_type\":2,\"duration_unit\":\"MO\",\"retention_duration\":1},\"schedule\":{\"trigger\":1,\"interval\":1,\"interval_unit\":\"d\",\"start_time\":\"2021-04-20T00:30:00\",\"window_start\":\"00:30:00\",\"window_end\":\"00:30:00\"},\"type\":\"backup\"},{\"uuid\":\"9b17382f-7164-4f5b-8d77-2910a0be348c\",\"name\":\"difference_increment\",\"action\":\"difference_increment\",\"ext_parameters\":{},\"retention\":{\"retention_type\":2,\"duration_unit\":\"MO\",\"retention_duration\":1},\"schedule\":{\"trigger\":1,\"interval\":4,\"interval_unit\":\"h\",\"start_time\":\"2021-04-20T01:00:00\",\"window_start\":\"00:30:00\",\"window_end\":\"00:30:00\"},\"type\":\"backup\"}],\"resource_count\":2,\"archival_count\":0,\"replication_count\":0}";

    @MockBean
    private RedissonService redissonService;

    @MockBean
    private ProviderRegistry registry;

    @Autowired
    private UnifiedTaskCompleteListener listener;

    @MockBean
    private DmeUnifiedRestApi unifiedRestApi;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private JobCenterRestApi jobCenterRestApi;

    @MockBean
    private NotifyManager notifyManager;

    @MockBean
    private BackupStorageApi backupStorageApi;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private UserQuotaManager userQuotaManager;

    @MockBean
    private DeployTypeService deployTypeService;

    @MockBean
    private MemberClusterService memberClusterService;

    public static void mockQueryDmeCopyInfo(DmeUnifiedRestApi unifiedRestApi) {
        DmeCopyInfo copyInfo = new DmeCopyInfo();
        copyInfo.setProtectEnv(new TaskEnvironment());
        copyInfo.setType("full");
        copyInfo.setSourceCopyType("full");
        StorageRepository repository = new StorageRepository();
        repository.setProtocol(RepositoryProtocolEnum.NFS.getProtocol());
        repository.setLocal(true);
        copyInfo.setRepositories(Arrays.asList(repository));
        copyInfo.setProtectObject(new TaskResource());
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(CopyPropertiesKeyConstant.KEY_COPY_VERIFY_FILE, "true");
        copyInfo.setExtendInfo(extendInfo);
        PowerMockito.when(unifiedRestApi.getCopyInfo(any())).thenReturn(copyInfo);
    }

    public static void mockSaveCopy(CopyRestApi copyRestApi) {
        UuidObject uuidObject = new UuidObject();
        uuidObject.setUuid("3bd9e448-816d-43b1-9b16-1feca34ece65");
        PowerMockito.when(copyRestApi.saveCopy(any())).thenReturn(uuidObject);
    }

    /**
     * 用例场景：测试监听处理任务完成消息成功 <br/>
     * 前置条件：参数正确 <br/>
     * 检查点：消息正确处理无异常抛出
     */
    @Test
    public void test_listen_task_complete_success() {
        TaskCompleteMessageDto message = new TaskCompleteMessageDto();
        message.setJobType(JobTypeEnum.BACKUP.getValue());
        message.setJobRequestId(UUID.randomUUID().toString());
        message.setJobId(UUID.randomUUID().toString());
        message.setTaskId(UUID.randomUUID().toString());

        RMap<String, String> context = PowerMockito.mock(RMap.class);

        PowerMockito.when(context.get(eq("resource"))).thenReturn(RES_JSON);
        PowerMockito.when(context.get(eq("sla"))).thenReturn(SLA_JSON);
        PowerMockito.when(context.get(eq("chain_id"))).thenReturn("chain_id");
        PowerMockito.when(context.get(eq("copy_format"))).thenReturn("1");
        PowerMockito.when(redissonService.getMap(any())).thenReturn(context);
        mockQueryDmeCopyInfo(unifiedRestApi);
        mockSaveCopy(copyRestApi);
        TaskCompleteHandler mockHandler = Mockito.mock(TaskCompleteHandler.class);
        PowerMockito.when(registry.findProvider(any(), any(), any())).thenReturn(mockHandler);

        // 成功
        message.setJobStatus(DmcJobStatus.SUCCESS.getStatus());
        listener.taskComplete(message);
        Mockito.verify(mockHandler, Mockito.times(1)).onTaskCompleteSuccess(any());

        // 失败
        message.setJobStatus(DmcJobStatus.FAIL.getStatus());
        listener.taskComplete(message);
        Mockito.verify(mockHandler, Mockito.times(1)).onTaskCompleteFailed(any());
    }
}
