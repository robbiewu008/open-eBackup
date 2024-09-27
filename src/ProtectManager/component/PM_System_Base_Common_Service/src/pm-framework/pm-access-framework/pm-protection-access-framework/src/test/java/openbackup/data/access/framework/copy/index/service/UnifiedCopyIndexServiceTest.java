/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.copy.index.service;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.oceanprotect.base.cluster.sdk.dto.ClusterRequestInfo;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.dee.DeeUnifiedRestApi;
import openbackup.data.access.client.sdk.api.framework.dee.model.DeleteCopyIndexRequest;
import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.core.common.enums.CopyIndexStatus;
import openbackup.data.access.framework.core.copy.CopyManagerService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.copy.index.service.impl.UnifiedCopyIndexService;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.copy.CopyBo;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexProvider;
import openbackup.data.protection.access.provider.sdk.index.v2.CopyIndexTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.TargetClusterRestApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.protection.SlaRestApi;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import com.huawei.oceanprotect.system.base.user.service.UserService;

import org.junit.Assert;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.ExpectedException;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.reflect.Whitebox;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.boot.test.mock.mockito.MockBean;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Optional;
import java.util.UUID;

/**
 * 统一框架副本创建索引服务类的测试类
 *
 * @author lWX776769
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022-01-13
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {UnifiedCopyIndexService.class})
public class UnifiedCopyIndexServiceTest {
    private static String copyStr
        = "{\"deletable\":false,\"archived\":false,\"replicated\":false,\"resource_id\":\"94820f43-4cc9-3153-a0a0-45087a337277\",\"resource_name\":\"wgh\",\"resource_type\":\"storage\",\"resource_sub_type\":\"NasFileSystem\",\"resource_location\":\"OceanStor Dorado 6.1.3+\\\\dorado28\\\\System_vStore\\\\zhudan_anti_1G\",\"resource_status\":\"EXIST\",\"resource_properties\":\"{\\\"name\\\":\\\"zhudan_anti_1G\\\",\\\"path\\\":\\\"OceanStor Dorado 6.1.3+\\\\\\\\dorado28\\\\\\\\System_vStore\\\\\\\\zhudan_anti_1G\\\",\\\"root_uuid\\\":\\\"2102353GTH10L8000008\\\",\\\"parent_name\\\":\\\"dorado28\\\",\\\"parent_uuid\\\":\\\"2102353GTH10L8000008\\\",\\\"children_uuids\\\":null,\\\"type\\\":\\\"storage\\\",\\\"sub_type\\\":\\\"NasFileSystem\\\",\\\"uuid\\\":\\\"94820f43-4cc9-3153-a0a0-45087a337277\\\",\\\"created_time\\\":\\\"2022-01-07T22:03:12.312000\\\",\\\"ext_parameters\\\":{\\\"proxy_host_mode\\\":0,\\\"agents\\\":null},\\\"authorized_user\\\":null,\\\"user_id\\\":null,\\\"version\\\":null,\\\"sla_id\\\":\\\"2bcc99b3-1ec1-46c2-9a0f-d34fb08b7dc8\\\",\\\"sla_name\\\":\\\"NasPerHour\\\",\\\"sla_status\\\":true,\\\"sla_compliance\\\":true,\\\"protection_status\\\":1,\\\"environment_uuid\\\":\\\"2102353GTH10L8000008\\\",\\\"environment_name\\\":\\\"dorado28\\\",\\\"environment_endpoint\\\":\\\"8.40.102.28\\\",\\\"environment_os_type\\\":null,\\\"environment_type\\\":\\\"StorageEquipment\\\",\\\"environment_sub_type\\\":\\\"DoradoV6\\\",\\\"environment_is_cluster\\\":\\\"False\\\",\\\"environment_os_name\\\":null,\\\"extendInfo\\\":{\\\"tenantName\\\":\\\"System_vStore\\\",\\\"fileSystemId\\\":\\\"11\\\",\\\"usedCapacity\\\":\\\"0\\\",\\\"onlineStatus\\\":\\\"27\\\",\\\"tenantId\\\":\\\"0\\\",\\\"capacity\\\":\\\"2097152\\\"}}\",\"resource_environment_name\":\"dorado28\",\"resource_environment_ip\":\"8.40.102.28\",\"uuid\":\"bfed7b8e-f7ae-47c7-878a-1542438e4997\",\"chain_id\":\"38d99a91-cd62-4fb4-a397-7a7a3cbd41cf\",\"timestamp\":\"1641647102000000\",\"display_timestamp\":\"2022-01-11T12:52:39\",\"status\":\"Normal\",\"location\":\"Local\",\"backup_type\":5,\"generated_by\":\"Backup\",\"generated_time\":\"2022-01-11T12:53:21\",\"generation_type\":\"\",\"features\":2,\"indexed\":\"Index_fail\",\"generation\":1,\"parent_copy_uuid\":\"\",\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"d\",\"expiration_time\":1657544064000,\"properties\":\"{\\\" snapshots \\\":[{\\\" id \\\":\\\" 7 @ FilesystemSnapshot2201111308220 \\\",\\\" parentName \\\":\\\" wgh \\\"}],\\\" repositories \\\":[{\\\" type \\\":1,\\\" protocol \\\":5,\\\" extendInfo \\\":{\\\" fileSystemId \\\":\\\" 11 \\\",\\\" productEsn \\\":\\\" 2102353GTH10L8000008 \\\"}},{\\\" type \\\":0,\\\" protocol \\\":5,\\\" extendInfo \\\":{\\\" fileSystemId \\\":\\\" 11 \\\",\\\" productEsn \\\":\\\" 2102353GTH10L8000008 \\\"}}],\\\"isAggregation\\\":false}\",\"sla_name\":\"NasPerHour\",\"sla_properties\":\"{\\\"name\\\": \\\"NasPerHour\\\", \\\"type\\\": 1, \\\"application\\\": \\\"NasFileSystem\\\", \\\"created_time\\\": \\\"2022-01-07T22:03:39.897393\\\", \\\"uuid\\\": \\\"2bcc99b3-1ec1-46c2-9a0f-d34fb08b7dc8\\\", \\\"is_global\\\": false, \\\"policy_list\\\": [{\\\"uuid\\\": \\\"99283b94-84fc-4981-854a-4e178ce873d3\\\", \\\"name\\\": \\\"permanent_increment\\\", \\\"action\\\": \\\"permanent_increment\\\", \\\"ext_parameters\\\": {\\\"auto_retry\\\": true, \\\"auto_retry_times\\\": 3, \\\"auto_retry_wait_minutes\\\": 5, \\\"qos_id\\\": \\\"\\\", \\\"auto_index\\\": true}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"d\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 1, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2022-01-06T00:00:00\\\", \\\"window_start\\\": \\\"00:00:00\\\", \\\"window_end\\\": \\\"23:59:59\\\", \\\"days_of_month\\\": null, \\\"days_of_year\\\": null, \\\"trigger_action\\\": null, \\\"days_of_week\\\": null}, \\\"type\\\": \\\"backup\\\"}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null}\",\"job_type\":\"\",\"user_id\":\"\",\"is_archived\":false,\"is_replicated\":false,\"amount\":0,\"gn\":437,\"prev_copy_id\":\"\",\"next_copy_id\":\"\",\"prev_copy_gn\":0,\"next_copy_gn\":0,\"device_esn\":\"123\"}";

    private static final String SLA_POLICY_ID = "99283b94-84fc-4981-854a-4e178ce873d3";

    private static String slaStr = "{\"name\":\"nasshare_sla\",\"type\":1,\"application\":\"NasShare\",\"created_time\":\"2021-12-16T10:54:36.247471\",\"uuid\":\"80306dc2-166f-4c86-81d0-5146c3cb35ca\",\"is_global\":false,\"policy_list\":[{\"uuid\":\"b2c50562-667e-4485-9ab3-6770193a7620\",\"name\":\"difference_increment\",\"action\":\"difference_increment\",\"ext_parameters\":{\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"qos_id\":\"32024a72-e884-4335-86bd-58b117308a4e\",\"auto_index\":false},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":111},\"schedule\":{\"trigger\":1,\"interval\":11,\"interval_unit\":\"h\",\"start_time\":\"2021-12-01T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\",\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"backup\"}],\"resource_count\":null,\"archival_count\":null,\"replication_count\":null}";

    private static String slaStrAutoIndexEnabled = "{\"name\":\"nasshare_sla\",\"type\":1,\"application\":\"NasShare\",\"created_time\":\"2021-12-16T10:54:36.247471\",\"uuid\":\"80306dc2-166f-4c86-81d0-5146c3cb35ca\",\"is_global\":false,\"policy_list\":[{\"uuid\":\"b2c50562-667e-4485-9ab3-6770193a7620\",\"name\":\"difference_increment\",\"action\":\"difference_increment\",\"ext_parameters\":{\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"qos_id\":\"32024a72-e884-4335-86bd-58b117308a4e\",\"auto_index\":true},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":111},\"schedule\":{\"trigger\":1,\"interval\":11,\"interval_unit\":\"h\",\"start_time\":\"2021-12-01T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\",\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"backup\"}],\"resource_count\":null,\"archival_count\":null,\"replication_count\":null}";

    private static String protectedResourceStr = "{\"uuid\":\"5af0788b-6405-471d-b86a-9809021c9280\",\"name\":\"protectengine-0\",\"type\":\"Host\",\"subType\":\"UBackupAgent\",\"path\":null,\"createdTime\":\"2022-01-24 10:16:24.318\",\"parentName\":null,\"parentUuid\":null,\"rootUuid\":null,\"sourceType\":null,\"version\":\"2.0\",\"protectionStatus\":0,\"extendInfo\":{\"scenario\":\"1\"},\"userId\":null,\"authorizedUser\":null,\"auth\":null,\"protectedObject\":null,\"environment\":null,\"endpoint\":\"192.168.145.213\",\"port\":59529,\"linkStatus\":\"1\",\"username\":\"\",\"password\":null,\"location\":null,\"osType\":\"linux\",\"osName\":\"linux\",\"scanInterval\":3600,\"cluster\":false}";

    private static String archiveSla = "{\"name\":\"nasshare_arch\",\"type\":1,\"application\":\"NasShare\",\"created_time\":\"2022-03-03T15:28:00.168656\",\"uuid\":\"fa369c4f-8ffd-4ba4-8ff4-690f879b45fd\",\"is_global\":false,\"policy_list\":[{\"uuid\":\"a724b11f-773d-4a23-a3c4-368d3a027f9e\",\"name\":\"\\u7b56\\u75650\",\"action\":\"archiving\",\"ext_parameters\":{\"qos_id\":\"\",\"storage_id\":\"9b9d6244860c4e3486cc9695362e6d96\",\"archive_target_type\":1,\"archiving_scope\":\"latest\",\"specified_scope\":null,\"network_access\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5,\"delete_import_copy\":null,\"protocol\":2},\"retention\":{\"retention_type\":2,\"duration_unit\":\"d\",\"retention_duration\":111},\"schedule\":{\"trigger\":2,\"interval\":null,\"interval_unit\":null,\"start_time\":null,\"window_start\":null,\"window_end\":null,\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"archiving\"}}";

    @Rule
    public ExpectedException expectedException = ExpectedException.none();

    @InjectMocks
    @Autowired
    private UnifiedCopyIndexService unifiedCopyIndexService;

    @MockBean
    private CopyRestApi copyRestApi;

    @MockBean
    private DeeUnifiedRestApi deeUnifiedRestApi;

    @MockBean
    private ProviderManager manager;

    @MockBean
    private ResourceService resourceService;

    @MockBean
    private SlaRestApi slaRestApi;

    @MockBean
    private ProtectAgentSelector defaultSelector;

    @MockBean
    private UserService userService;

    @MockBean
    private CommonAgentService commonAgentService;

    @MockBean
    @Qualifier("memberClusterApiWithDmaProxyManagePort")
    private TargetClusterRestApi targetClusterRestApi;

    @MockBean
    private MemberClusterService memberClusterService;

    @MockBean
    private CopyManagerService copyManagerService;;

    @MockBean
    private DefaultProtectAgentSelector defaultProtectAgentSelector;
    /**
     * 用例场景：正常执行创建索引方法
     * 前置条件：索引参数
     * 检查点：设置的参数正确
     */
    @Test
    public void execute_create_index_task_success() {
        CopyBo copy = JSONObject.toBean(copyStr, CopyBo.class);
        String requestId = UUID.randomUUID().toString();
        String indexedMode = "auto";
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ResourceProvider.class), any(), any()))
            .thenReturn(null);
        CopyIndexProvider indexProvider = PowerMockito.mock(CopyIndexProvider.class);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(CopyIndexProvider.class), any(), any()))
            .thenReturn(indexProvider);
        PowerMockito.when(indexProvider.isSupportIndex()).thenReturn(true);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ProtectAgentSelector.class), any(), any()))
            .thenReturn(PowerMockito.mock(ProtectAgentSelector.class));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockEnvs());
        TaskResource taskResource = new TaskResource();
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        Mockito.when(copyManagerService.buildTaskResource(Mockito.any())).thenReturn(taskResource);
        Mockito.when(copyManagerService.buildTaskEnvironment(Mockito.any())).thenReturn(taskEnvironment);
        CopyIndexTask indexTask = unifiedCopyIndexService.createIndexTask(copy, requestId, indexedMode);
        Assert.assertEquals(indexedMode.toUpperCase(), indexTask.getTriggerMode());
        Assert.assertFalse(indexTask.getCopyInfo().isAggregation());
        Assert.assertFalse(indexTask.getCopyInfo().isIndexed());
    }

    private PageListResponse<ProtectedResource> mockAgents() {
        PageListResponse<ProtectedResource> agents = new PageListResponse<>();
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource = JSONObject.toBean(protectedResourceStr, ProtectedEnvironment.class);
        resources.add(protectedResource);
        agents.setRecords(resources);
        agents.setTotalCount(1);
        return agents;
    }

    private PageListResponse<ProtectedResource> mockEnvs() {
        PageListResponse<ProtectedResource> envRecords = new PageListResponse<>();
        ProtectedEnvironment hdfsEnv = new ProtectedEnvironment();
        hdfsEnv.setName("dorado28");
        hdfsEnv.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        List<ProtectedResource> resources = new ArrayList<>();
        resources.add(hdfsEnv);
        envRecords.setRecords(resources);
        envRecords.setTotalCount(1);
        return envRecords;
    }

    /**
     * 用例场景：当调用dee时抛出错误，创建索引失败
     * 前置条件：1、索引参数正常；2、DEE接口抛出错误
     * 检查点：副本状态为“未索引”
     */
    @Test
    public void execute_create_index_task_fail_when_dee_throw_exception() {
        CopyBo copy = JSONObject.toBean(copyStr, CopyBo.class);
        copy.setProperties(JSONObject.fromObject(copy.getProperties()).set("sla_policy_id", SLA_POLICY_ID).toString());
        String requestId = UUID.randomUUID().toString();
        String indexedMode = "auto";
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ResourceProvider.class), any(), any()))
            .thenReturn(null);
        CopyIndexProvider indexProvider = PowerMockito.mock(CopyIndexProvider.class);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(CopyIndexProvider.class), any(), any()))
            .thenReturn(indexProvider);
        PowerMockito.when(indexProvider.isSupportIndex()).thenReturn(true);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ProtectAgentSelector.class), any(), any()))
            .thenReturn(PowerMockito.mock(ProtectAgentSelector.class));
        PowerMockito.when(resourceService.query(anyInt(), anyInt(), any())).thenReturn(mockEnvs());
        DeeUnifiedRestApi mockDeeUnifiedRestApi = new DeeUnifiedRestApi() {
            @Override
            public void createIndexTask(final CopyIndexTask indexTask) {
                throw new LegoUncheckedException(-1);
            }

            @Override
            public void deleteIndexTask(final String requestId, final String resourceId, final String copyId,
                final String chainId, final String userId) {
            }

            @Override
            public void deleteIndexTask(DeleteCopyIndexRequest deleteCopyIndexRequest) {
            }
        };
        Whitebox.setInternalState(unifiedCopyIndexService, "deeUnifiedRestApi", mockDeeUnifiedRestApi);
        Mockito.when(copyManagerService.buildTaskResource(any())).thenReturn(new TaskResource());
        CopyIndexTask indexTask = unifiedCopyIndexService.createIndexTask(copy, requestId, indexedMode);
        Assert.assertEquals(indexedMode.toUpperCase(), indexTask.getTriggerMode());
        Assert.assertFalse(indexTask.getCopyInfo().isAggregation());
        Assert.assertFalse(indexTask.getCopyInfo().isIndexed());
        Mockito.verify(copyRestApi)
            .updateCopyIndexStatus(eq(copy.getUuid()), eq(CopyIndexStatus.INDEX_FAIL.getIndexStaus()),
                eq(CopyIndexStatus.INDEX_RESPONSE_ERROR_LABEL.getIndexStaus()));
        Whitebox.setInternalState(unifiedCopyIndexService, "deeUnifiedRestApi", deeUnifiedRestApi);
    }

    /**
     * 用例场景：当sla中的开关被关闭时，创建索引失败
     * 前置条件：1、索引参数中sla参数找不到对应策略；
     * 检查点：副本状态为“未索引”
     */
    @Test
    public void execute_create_index_task_fail_when_auto_index_is_close() {
        CopyBo copy = JSONObject.toBean(copyStr, CopyBo.class);
        copy.setSlaProperties(slaStr);
        String requestId = UUID.randomUUID().toString();
        String indexedMode = "auto";
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ResourceProvider.class), any(), any()))
            .thenReturn(null);
        CopyIndexProvider indexProvider = PowerMockito.mock(CopyIndexProvider.class);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(CopyIndexProvider.class), any(), any()))
            .thenReturn(indexProvider);
        PowerMockito.when(indexProvider.isSupportIndex()).thenReturn(true);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ProtectAgentSelector.class), any(), any()))
            .thenReturn(PowerMockito.mock(ProtectAgentSelector.class));
        CopyIndexTask indexTask = unifiedCopyIndexService.createIndexTask(copy, requestId, indexedMode);
        Assert.assertNull(indexTask.getTriggerMode());
        Assert.assertNull(indexTask.getCopyInfo());
    }

    /**
     * 用例场景：正常执行创建索引方法
     * 前置条件：索引参数
     * 检查点：设置的参数正确
     */
    @Test
    public void should_return_empty_if_not_support_index_when_create_index_task() {
        CopyBo copy = JSONObject.toBean(copyStr, CopyBo.class);
        String requestId = UUID.randomUUID().toString();
        String indexedMode = "auto";
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ResourceProvider.class), any(), any()))
            .thenReturn(PowerMockito.mock(ResourceProvider.class));
        CopyIndexTask indexTask = unifiedCopyIndexService.createIndexTask(copy, requestId, indexedMode);
        Assert.assertEquals(null, indexTask.getRequestId());
        Assert.assertEquals(null, indexTask.getTriggerMode());
        Assert.assertEquals(null, indexTask.getCopyInfo());
        Assert.assertEquals(null, indexTask.getAgents());
    }

    /**
     * 用例场景：正常执行删除资源索引
     * 前置条件：资源正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_delete_resource_index_task_success() {
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockProtectedResource());
        PowerMockito.when(copyRestApi.queryCopiesByResourceIdAndIndexStatus(any(), any()))
            .thenReturn(new ArrayList<>());
        SlaBo slaBo = JSONObject.toBean(slaStr, SlaBo.class);
        PowerMockito.when(slaRestApi.querySlaById(any())).thenReturn(slaBo);
        PowerMockito.when(copyRestApi.queryCopiesByResourceIdAndIndexStatus(any(), any()))
            .thenReturn(new ArrayList<>());
        unifiedCopyIndexService.deleteResourceIndexTask(UUID.randomUUID().toString(), "123456");
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopiesByResourceIdAndIndexStatus(any(), any());
    }

    private Optional<ProtectedResource> mockProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName("nas");
        protectedResource.setSubType("NasFileSystem");
        protectedResource.setUuid(UUID.randomUUID().toString());
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSlaId(UUID.randomUUID().toString());
        protectedResource.setProtectedObject(protectedObject);
        return Optional.of(protectedResource);
    }

    /**
     * 用例场景：执行删除资源索引
     * 前置条件：资源的sla策略的自动索引打开
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_sla_auto_index_enabled_when_execute_delete_resource_index_task() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("The sla auto index is enabled");
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockProtectedResource());
        SlaBo slaBo = JSONObject.toBean(slaStrAutoIndexEnabled, SlaBo.class);
        PowerMockito.when(slaRestApi.querySlaById(any())).thenReturn(slaBo);
        unifiedCopyIndexService.deleteResourceIndexTask(UUID.randomUUID().toString(), "123456");
    }

    /**
     * 用例场景：执行删除资源索引
     * 前置条件：副本索引状态是索引中
     * 检查点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_index_status_deleting_when_execute_delete_resource_index_task() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Have copy indexing.");
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockProtectedResource());
        SlaBo slaBo = JSONObject.toBean(slaStr, SlaBo.class);
        PowerMockito.when(slaRestApi.querySlaById(any())).thenReturn(slaBo);
        PowerMockito.when(copyRestApi.queryCopiesByResourceIdAndIndexStatus(any(), any())).thenReturn(mockCopyList());
        unifiedCopyIndexService.deleteResourceIndexTask(UUID.randomUUID().toString(), "123456");
    }

    /**
     * 用例场景：正常执行删除副本索引
     * 前置条件：资源正常
     * 检查点：无异常抛出
     */
    @Test
    public void execute_delete_copy_index_task_success() {
        Copy copy = new Copy();
        copy.setIndexed(CopyIndexStatus.INDEXED.getIndexStaus());
        PowerMockito.when(copyRestApi.queryCopyByID(any())).thenReturn(copy);
        unifiedCopyIndexService.deleteCopyIndex(UUID.randomUUID().toString(), UUID.randomUUID().toString());
        Mockito.verify(copyRestApi, Mockito.times(1)).queryCopyByID(any());
    }

    /**
     * 用例场景：执行删除副本索引
     * 前置条件：副本不存在
     * 检查点：抛出对象不存在异常
     */
    @Test
    public void should_throw_LegoCheckedException_if_copy_is_not_exists_when_execute_delete_copy_index_task() {
        expectedException.expect(LegoCheckedException.class);
        expectedException.expectMessage("Copy is not exists.");
        unifiedCopyIndexService.deleteCopyIndex(UUID.randomUUID().toString(), UUID.randomUUID().toString());
    }

    /**
     * 用例场景：当归档副本的sla中的自动索引被关闭时，不执行创建索引
     * 前置条件：索引参数中sla自动索引被关闭；
     * 检查点：不执行创建索引
     */
    @Test
    public void execute_create_index_task_fail_when_archived_copy_auto_index_is_close() {
        CopyBo copy = mockArchiveCopy();
        String requestId = UUID.randomUUID().toString();
        String indexedMode = "auto";
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ResourceProvider.class), any(), any()))
            .thenReturn(null);
        CopyIndexProvider indexProvider = PowerMockito.mock(CopyIndexProvider.class);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(CopyIndexProvider.class), any(), any()))
            .thenReturn(indexProvider);
        PowerMockito.when(indexProvider.isSupportIndex()).thenReturn(true);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ProtectAgentSelector.class), any(), any()))
            .thenReturn(PowerMockito.mock(ProtectAgentSelector.class));
        CopyIndexTask indexTask = unifiedCopyIndexService.createIndexTask(copy, requestId, indexedMode);
        Assert.assertNull(indexTask.getTriggerMode());
        Assert.assertNull(indexTask.getCopyInfo());
    }

    @Test
    public void should_forward_success() {
        ClusterRequestInfo clusterRequestInfo = new ClusterRequestInfo();
        clusterRequestInfo.setPort(123);
        clusterRequestInfo.setIp("123.123.123.123");
        clusterRequestInfo.setToken("123");
        PowerMockito.when(memberClusterService.getClusterRequestInfo(anyString())).thenReturn(clusterRequestInfo);
        unifiedCopyIndexService.forwardCreateIndex("123", "123");
        Mockito.verify(targetClusterRestApi, Mockito.times(1)).createIndex(any(), anyString(), anyString());
    }

    private CopyBo mockArchiveCopy() {
        CopyBo copy = new CopyBo();
        copy.setIsArchived(true);
        copy.setProperties(Collections.singletonMap("storage_id", "9b9d6244860c4e3486cc9695362e6d96").toString());
        copy.setSlaProperties(archiveSla);
        return copy;
    }

    private List<Copy> mockCopyList() {
        List<Copy> copys = new ArrayList<>();
        copys.add(new Copy());
        return copys;
    }
}