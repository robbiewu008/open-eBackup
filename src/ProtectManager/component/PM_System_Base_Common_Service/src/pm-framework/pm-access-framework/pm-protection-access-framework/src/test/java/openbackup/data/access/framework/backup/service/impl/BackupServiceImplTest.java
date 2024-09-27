package openbackup.data.access.framework.backup.service.impl;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.StorageUnitService;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.backup.constant.BackupConstant;
import openbackup.data.access.framework.backup.service.impl.BackupServiceImpl;
import openbackup.data.access.framework.copy.mng.service.DeeCopyService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.service.SanClientService;
import openbackup.data.access.framework.protection.service.repository.RepositoryStrategyManager;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.access.framework.protection.service.repository.strategies.RepositoryStrategy;
import openbackup.data.access.framework.servitization.util.OpServiceHelper;
import openbackup.data.protection.access.provider.sdk.agent.CommonAgentService;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.resource.NextBackupParams;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.SelectorManager;
import openbackup.data.protection.access.provider.sdk.sla.Policy;
import openbackup.data.protection.access.provider.sdk.sla.Sla;
import com.huawei.oceanprotect.job.sdk.JobService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.protection.model.QosBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.service.AvailableAgentManagementDomainService;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import org.junit.Assert;
import org.junit.BeforeClass;
import org.junit.Test;
import org.mockito.ArgumentMatchers;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.reflect.Whitebox;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.mock.mockito.MockBean;

import java.net.URI;
import java.net.URISyntaxException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * 备份服务LLT
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-03
 */
public class BackupServiceImplTest {
    private static final String RESOURCE_ID = "266ea41d-adf5-480b-af50-15b940c2b846";
    private static final String ENV_ID = "266ea41d-adf5-480b-af50-15b940c2b850";
    private static final String REQUEST_ID = "266ea41d-adf5-480b-af50-15b940c2b860";
    private static final String HDFS_FILESET = "HDFSFileset";
    private static final String NAME = "testfs";
    private final TaskRepositoryManager taskRepositoryManager = Mockito.mock(TaskRepositoryManager.class);
    private final SystemSwitchInternalService systemSwitchInternalService = PowerMockito.mock(SystemSwitchInternalService.class);
    private DeployTypeService deployTypeService = Mockito.mock(DeployTypeService.class);

    private CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);

    private SanClientService sanClientService = PowerMockito.mock(SanClientService.class);

    private static StorageUnitService storageUnitService = PowerMockito.mock(StorageUnitService.class);

    private static MemberClusterService memberClusterService = PowerMockito.mock(MemberClusterService.class);

    @MockBean
    private RedissonClient redissonClient;

    @BeforeClass
    public static void init() {


    }

    /**
     * 用例场景：正常下发备份命令，冒烟测试
     * 前置条件：备份参数设置正确
     * 检查点：无异常抛出
     */
    @Test
    public void smoke_test() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setRequestId(UUIDGenerator.getUUID());
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("storage_info", "");
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);

        CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
        backupService.setCopyRestApi(copyRestApi);
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        backupService.setOpServiceHelper(opServiceHelper);

        BackupTask task = backupService.backup(backupObject);
        assertEquals(task.getAdvanceParams().get("test_param"), "test_value");
        assertEquals(task.getAdvanceParams().get("file_scan_channel_number"), "20");
        assertEquals(task.getAdvanceParams().get("read_and_send_channel_number"), "1");
        assertNull(task.getAdvanceParams().get("before_protect_script"));
        assertEquals(task.getRepositories().size(), 1);
        assertEquals(task.getBackupType(), "fullBackup");
        assertFalse(task.isParameterChanged("encryption"));
        assertTrue(task.getDataLayout().isSrcDeduption());
        assertTrue(!task.isParameterChanged("encryption"));
        // 备份链路加密开关
        assertFalse(task.getDataLayout().isLinkEncryption());

        // fc配置开关
        assertEquals(Optional.of(1), Optional.of(task.getDataLayout().getClientProtocolType()));

        // 差异备份
        policy.setAction("cumulative_increment");
        backupObject.setBackupType(policy.getAction());
        task = backupService.backup(backupObject);
        assertEquals(task.getBackupType(), "diffBackup");

        // 增量备份
        policy.setAction("difference_increment");
        backupObject.setBackupType(policy.getAction());
        task = backupService.backup(backupObject);
        assertEquals(task.getBackupType(), "incrementBackup");

        // 永久增量备份
        policy.setAction("permanent_increment");
        backupObject.setBackupType(policy.getAction());
        task = backupService.backup(backupObject);
        assertEquals(task.getBackupType(), "foreverIncrementBackup");

        // 日志备份
        policy.setAction("log");
        backupObject.setBackupType(policy.getAction());
        task = backupService.backup(backupObject);
        assertEquals(task.getBackupType(), "logBackup");

        // 开启sanClient，首次备份转全量
        PowerMockito.when(sanClientService.getAgentsNotConfiguredSanclient(any())).thenReturn(null);
        policy.setAction("difference_increment");
        backupObject.setBackupType(policy.getAction());
        Copy copy1 = new Copy();
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy1);
        task = backupService.backup(backupObject);
        assertEquals(task.getBackupType(), "fullBackup");

        // 关闭sanClient且前一次副本为sanClient副本，首次备份转全量
        PowerMockito.when(sanClientService.getAgentsNotConfiguredSanclient(any())).thenReturn(new String[] {"1.1.1.1"});
        Copy copy2 = new Copy();
        copy2.setProperties("{isSanClient: true}");
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy2);
        policy.setAction("difference_increment");
        backupObject.setBackupType(policy.getAction());
        task = backupService.backup(backupObject);
        assertEquals(task.getBackupType(), "fullBackup");

        ObjectNode objectNode = (ObjectNode) jsonNode;
        objectNode.put(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY, true);
        ObjectMapper objectMapper = JSONObject.createObjectMapper();
        ObjectNode temp = objectMapper.createObjectNode();
        temp.put(BackupConstant.BACKUP_EXT_PARAM_STORAGE_TYPE_KEY, BackupConstant.BACKUP_EXT_PARAM_STORAGE_UNIT_GROUP_VALUE);
        temp.put(BackupConstant.BACKUP_EXT_PARAM_STORAGE_ID_KEY, "aaaaa");
        objectNode.put(BackupConstant.BACKUP_EXT_PARAM_STORAGE_INFO_KEY, temp);
        task = backupService.backup(backupObject);
        assertEquals(task.getRepositories().size(), 1);

    }
    /**
     * 用例场景：正常下发备份命令时，没有可用的agent
     * 前置条件：没有可用的agent
     * 检查点：异常抛出
     */
    @Test
    public void should_exception_if_no_agents() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(false);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("encryption", true);
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
        backupService.setCopyRestApi(copyRestApi);
    }

    /**
     * 测试场景：备份策略未设置数据布局，则返回默认的数据布局
     * 前置条件：备份策略未设置数据布局
     * 检查点：数据布局使用默认的
     */
    @Test
    public void should_return_default_value_if_data_layout_is_not_set() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("storage_info", "");
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        backupService.setOpServiceHelper(opServiceHelper);
        BackupTask task = backupService.backup(backupObject);
        assertFalse(task.getDataLayout().isDstEncryption());
        assertTrue(task.getDataLayout().isDstDeduption());
        assertTrue(task.getDataLayout().isDstCompression());
        assertFalse(task.getDataLayout().isSrcEncryption());
        assertTrue(task.getDataLayout().isSrcDeduption());
        assertFalse(task.getDataLayout().isSrcCompression());
    }

    /**
     * 测试场景：备份策略未设置Qos，则备份任务的Qos为空
     * 前置条件：备份策略中未设置Qos
     * 检查点：Qos为Null
     */
    @Test
    public void should_return_null_value_if_qos_is_not_set() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        BackupObject backupObject = mockBackupObject();
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        backupService.setOpServiceHelper(opServiceHelper);
        backupObject.setBackupType("full");
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        JsonNode jsonNode1 = backupObject.getPolicy().getExtParameters();
        if (jsonNode1 instanceof ObjectNode) {
            ((ObjectNode)jsonNode1).put("storage_info", "");
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        BackupTask task = backupService.backup(backupObject);

        // QosId设置为空字符串
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("qos_id", "");
        }

        task = backupService.backup(backupObject);
        assertNull(task.getQos());

        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("qos_id", "test_id");
        }
        backupService.backup(backupObject);
    }

    /**
     * 测试场景：备份存储使用备份策略中设置的存储及类型（S3）
     * 前置条件：策略中设置了对应的备份存储
     * 检查点：备份任务中的存储类型为S3
     */
    @Test
    public void should_return_s3_repository_if_storage_is_set_s3() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("storageId", "storageId");
            ((ObjectNode)jsonNode).put("storageType", RepositoryProtocolEnum.S3.getProtocol());
            ((ObjectNode)jsonNode).put("storage_info", "");
        }
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        backupService.setOpServiceHelper(opServiceHelper);
        BackupTask task = backupService.backup(backupObject);
        assertEquals(task.getRepositories().size(), 1);
        assertEquals(task.getRepositories().get(0).getProtocol().intValue(), RepositoryProtocolEnum.S3.getProtocol());
    }

    /**
     * 测试场景： 备份参数改变的情况
     * 前置条件： 上一次备份和当前备份参数不一样
     * 检查点：检查备份变化的备份参数
     */
    @Test
    public void should_return_full_backup_type_if_datalayout_changed() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);

        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("encryption", Boolean.TRUE);
            ((ObjectNode)jsonNode).put("storage_info", "");
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        CopyRestApi copyRestApi = PowerMockito.mock(CopyRestApi.class);
        String copyJson
            = "{\"uuid\":\"386ebbec-8dea-4870-8a13-68cf856b4c27\",\"chain_id\":\"83cb05fc-8ad4-4452-85eb-b7b878f92c69\",\"timestamp\":\"1640591852097581\",\"display_timestamp\":\"2021-12-27T07:57:32.097000\",\"deletable\":false,\"status\":\"Normal\",\"location\":\"Local\",\"backup_type\":2,\"generated_by\":\"Backup\",\"generated_time\":\"2021-12-27T07:57:32.097000\",\"features\":2,\"indexed\":\"Indexed\",\"generation\":1,\"parent_copy_uuid\":null,\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"MO\",\"expiration_time\":\"2022-01-27T07:57:32.097000\",\"properties\":\"{\\\"backup_id\\\":\\\"1e3fff275a5711ecafc03a72c20070ab\\\",\\\"backup_type\\\":\\\"difference_increment\\\"}\",\"resource_id\":\"dc80b3c2-47db-4136-975d-793f3839447d\",\"resource_name\":\"test2\",\"resource_type\":\"Fileset\",\"resource_sub_type\":\"Fileset\",\"resource_location\":\"localhost.localdomain\",\"resource_status\":\"EXIST\",\"resource_properties\":\"{\\\"uuid\\\":\\\"dc80b3c2-47db-4136-975d-793f3839447d\\\",\\\"name\\\":\\\"test2\\\",\\\"type\\\":\\\"Fileset\\\",\\\"path\\\":\\\"localhost.localdomain\\\",\\\"version\\\":\\\"\\\",\\\"paths\\\":[\\\"/opt\\\"],\\\"filters\\\":[],\\\"sub_type\\\":\\\"Fileset\\\",\\\"created_time\\\":\\\"2021-12-11T07:50:31.404758\\\",\\\"ext_parameters\\\":{},\\\"parent_name\\\":\\\"\\\",\\\"parent_uuid\\\":\\\"\\\",\\\"root_uuid\\\":\\\"00da1134d4a0c211ba24969400b86d96\\\",\\\"environment_name\\\":\\\"localhost.localdomain\\\",\\\"environment_uuid\\\":\\\"00da1134d4a0c211ba24969400b86d96\\\",\\\"environment_endpoint\\\":\\\"192.168.100.231\\\",\\\"environment_os_type\\\":\\\"linux\\\",\\\"environment_type\\\":\\\"Host\\\",\\\"environment_sub_type\\\":\\\"ABBackupClient\\\",\\\"environment_is_cluster\\\":false,\\\"environment_os_name\\\":\\\"Linux el7\\\",\\\"children_uuids\\\":[],\\\"authorized_user\\\":\\\"\\\",\\\"user_id\\\":\\\"\\\",\\\"sla_id\\\":\\\"8556bb41-abe6-4821-870d-a0252f304dfc\\\",\\\"sla_name\\\":\\\"Gold\\\",\\\"sla_status\\\":\\\"true\\\",\\\"sla_compliance\\\":\\\"true\\\"}\",\"resource_environment_name\":\"localhost.localdomain\",\"resource_environment_ip\":\"192.168.100.231\",\"sla_name\":\"Gold\",\"sla_properties\":\"{\\\"name\\\": \\\"Gold\\\", \\\"type\\\": 1, \\\"application\\\": \\\"Common\\\", \\\"created_time\\\": \\\"9999-01-09T20:21:32.842417\\\", \\\"uuid\\\": \\\"8556bb41-abe6-4821-870d-a0252f304dfc\\\", \\\"is_global\\\": true, \\\"policy_list\\\": [{\\\"uuid\\\": \\\"3bd9e448-816d-43b1-9b16-1feca34ece65\\\", \\\"name\\\": \\\"full\\\", \\\"action\\\": \\\"full\\\", \\\"ext_parameters\\\": {}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"MO\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 1, \\\"interval_unit\\\": \\\"d\\\", \\\"start_time\\\": \\\"2021-04-20T00:30:00\\\", \\\"window_start\\\": \\\"00:30:00\\\", \\\"window_end\\\": \\\"00:30:00\\\"}, \\\"type\\\": \\\"backup\\\"}, {\\\"uuid\\\": \\\"9b17382f-7164-4f5b-8d77-2910a0be348c\\\", \\\"name\\\": \\\"difference_increment\\\", \\\"action\\\": \\\"difference_increment\\\", \\\"ext_parameters\\\": {}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"MO\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 4, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2021-04-20T01:00:00\\\", \\\"window_start\\\": \\\"00:30:00\\\", \\\"window_end\\\": \\\"00:30:00\\\"}, \\\"type\\\": \\\"backup\\\"}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null}\",\"user_id\":\"\",\"is_archived\":false,\"is_replicated\":false,\"gn\":468,\"prev_copy_id\":\"ec89e74a-e2c0-4cfa-9754-4e2fd386a707\",\"next_copy_id\":null,\"prev_copy_gn\":462,\"next_copy_gn\":null}";
        Copy copy = JSONObject.fromObject(copyJson).toBean(Copy.class);
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy);
        backupService.setCopyRestApi(copyRestApi);
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        backupService.setOpServiceHelper(opServiceHelper);
        BackupTask task = backupService.backup(backupObject);
        assertTrue(task.isParameterChanged("encryption"));
        assertTrue(task.isParameterAdded("encryption"));
        assertFalse(task.isParameterDeleted("encryption"));

        // 上一次备份开启开启加密
        String copyJsonWithEncryption
            = "{\"uuid\":\"386ebbec-8dea-4870-8a13-68cf856b4c27\",\"chain_id\":\"83cb05fc-8ad4-4452-85eb-b7b878f92c69\",\"timestamp\":\"1640591852097581\",\"display_timestamp\":\"2021-12-27T07:57:32.097000\",\"deletable\":false,\"status\":\"Normal\",\"location\":\"Local\",\"backup_type\":2,\"generated_by\":\"Backup\",\"generated_time\":\"2021-12-27T07:57:32.097000\",\"features\":2,\"indexed\":\"Indexed\",\"generation\":1,\"parent_copy_uuid\":null,\"retention_type\":2,\"retention_duration\":1,\"duration_unit\":\"MO\",\"expiration_time\":\"2022-01-27T07:57:32.097000\",\"properties\":\"{\\\"backup_id\\\":\\\"1e3fff275a5711ecafc03a72c20070ab\\\",\\\"backup_type\\\":\\\"difference_increment\\\"}\",\"resource_id\":\"dc80b3c2-47db-4136-975d-793f3839447d\",\"resource_name\":\"test2\",\"resource_type\":\"Fileset\",\"resource_sub_type\":\"Fileset\",\"resource_location\":\"localhost.localdomain\",\"resource_status\":\"EXIST\",\"resource_properties\":\"{\\\"uuid\\\":\\\"dc80b3c2-47db-4136-975d-793f3839447d\\\",\\\"name\\\":\\\"test2\\\",\\\"type\\\":\\\"Fileset\\\",\\\"path\\\":\\\"localhost.localdomain\\\",\\\"version\\\":\\\"\\\",\\\"paths\\\":[\\\"/opt\\\"],\\\"filters\\\":[],\\\"sub_type\\\":\\\"Fileset\\\",\\\"created_time\\\":\\\"2021-12-11T07:50:31.404758\\\",\\\"ext_parameters\\\":{\\\"encryption\\\":true},\\\"parent_name\\\":\\\"\\\",\\\"parent_uuid\\\":\\\"\\\",\\\"root_uuid\\\":\\\"00da1134d4a0c211ba24969400b86d96\\\",\\\"environment_name\\\":\\\"localhost.localdomain\\\",\\\"environment_uuid\\\":\\\"00da1134d4a0c211ba24969400b86d96\\\",\\\"environment_endpoint\\\":\\\"192.168.100.231\\\",\\\"environment_os_type\\\":\\\"linux\\\",\\\"environment_type\\\":\\\"Host\\\",\\\"environment_sub_type\\\":\\\"ABBackupClient\\\",\\\"environment_is_cluster\\\":false,\\\"environment_os_name\\\":\\\"Linux el7\\\",\\\"children_uuids\\\":[],\\\"authorized_user\\\":\\\"\\\",\\\"user_id\\\":\\\"\\\",\\\"sla_id\\\":\\\"8556bb41-abe6-4821-870d-a0252f304dfc\\\",\\\"sla_name\\\":\\\"Gold\\\",\\\"sla_status\\\":\\\"true\\\",\\\"sla_compliance\\\":\\\"true\\\"}\",\"resource_environment_name\":\"localhost.localdomain\",\"resource_environment_ip\":\"192.168.100.231\",\"sla_name\":\"Gold\",\"sla_properties\":\"{\\\"name\\\": \\\"Gold\\\", \\\"type\\\": 1, \\\"application\\\": \\\"Common\\\", \\\"created_time\\\": \\\"9999-01-09T20:21:32.842417\\\", \\\"uuid\\\": \\\"8556bb41-abe6-4821-870d-a0252f304dfc\\\", \\\"is_global\\\": true, \\\"policy_list\\\": [{\\\"uuid\\\": \\\"3bd9e448-816d-43b1-9b16-1feca34ece65\\\", \\\"name\\\": \\\"full\\\", \\\"action\\\": \\\"full\\\", \\\"ext_parameters\\\": {\\\"encryption\\\":true}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"MO\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 1, \\\"interval_unit\\\": \\\"d\\\", \\\"start_time\\\": \\\"2021-04-20T00:30:00\\\", \\\"window_start\\\": \\\"00:30:00\\\", \\\"window_end\\\": \\\"00:30:00\\\"}, \\\"type\\\": \\\"backup\\\"}, {\\\"uuid\\\": \\\"9b17382f-7164-4f5b-8d77-2910a0be348c\\\", \\\"name\\\": \\\"difference_increment\\\", \\\"action\\\": \\\"difference_increment\\\", \\\"ext_parameters\\\": {\\\"encryption\\\":true}, \\\"retention\\\": {\\\"retention_type\\\": 2, \\\"duration_unit\\\": \\\"MO\\\", \\\"retention_duration\\\": 1}, \\\"schedule\\\": {\\\"trigger\\\": 1, \\\"interval\\\": 4, \\\"interval_unit\\\": \\\"h\\\", \\\"start_time\\\": \\\"2021-04-20T01:00:00\\\", \\\"window_start\\\": \\\"00:30:00\\\", \\\"window_end\\\": \\\"00:30:00\\\"}, \\\"type\\\": \\\"backup\\\"}], \\\"resource_count\\\": null, \\\"archival_count\\\": null, \\\"replication_count\\\": null}\",\"user_id\":\"\",\"is_archived\":false,\"is_replicated\":false,\"gn\":468,\"prev_copy_id\":\"ec89e74a-e2c0-4cfa-9754-4e2fd386a707\",\"next_copy_id\":null,\"prev_copy_gn\":462,\"next_copy_gn\":null}";
        copy = JSONObject.fromObject(copyJsonWithEncryption).toBean(Copy.class);
        PowerMockito.when(copyRestApi.queryLatestBackupCopy(any(), any(), any())).thenReturn(copy);

        // 本次备份关闭加密
        jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).remove("encryption");
        }

        task = backupService.backup(backupObject);
        assertTrue(task.isParameterChanged("encryption"));
        assertFalse(task.isParameterAdded("encryption"));
        assertTrue(task.isParameterDeleted("encryption"));
    }

    /**
     * 测试场景： 根据RetentionBo给定的保留参数，计算保留天数
     * 前置条件： 无
     * 检查点： 检查计算保留天数正确
     */
    @Test
    public void testCalRetentionDay() {
        RetentionBo retentionWeek = new RetentionBo();
        retentionWeek.setRetentionDuration(1);
        retentionWeek.setDurationUnit("w");
        Assert.assertEquals(DeeCopyService.getRetentionDay(retentionWeek), 7L);

        RetentionBo retentionYear = new RetentionBo();
        retentionYear.setRetentionDuration(1);
        retentionYear.setDurationUnit("y");
        // 考虑闰年
        Assert.assertTrue(DeeCopyService.getRetentionDay(retentionYear) - 365 <= 1);
        Assert.assertTrue(DeeCopyService.getRetentionDay(retentionYear) - 365 >= 0);

        RetentionBo retentionMonth = new RetentionBo();
        retentionMonth.setRetentionDuration(1);
        retentionMonth.setDurationUnit("MO");
        Assert.assertTrue(DeeCopyService.getRetentionDay(retentionMonth) - 28 <= 3);
        Assert.assertTrue(DeeCopyService.getRetentionDay(retentionMonth) - 28 >= 0);
    }

    /**
     * 测试场景：备份任务不下发sanClient备份
     * 前置条件：代理主机未配置sanClient
     * 检查点：备份任务中高级参数中下发sanClient备份标志为false
     *
     * @throws IllegalAccessException 抛出异常
     */
    @Test
    public void should_return_false_if_agents_not_config_sanClient() throws IllegalAccessException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        SanClientService sanClientService = PowerMockito.mock(SanClientService.class);
        PowerMockito.when(sanClientService.getAgentsNotConfiguredSanclient(any()))
                .thenReturn(new String[] {"9527"});
        backupService.setSanClientService(sanClientService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("storage_info", "");
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        OpServiceHelper opServiceHelper = PowerMockito.mock(OpServiceHelper.class);
        backupService.setOpServiceHelper(opServiceHelper);
        BackupTask task = backupService.backup(backupObject);
        task.getAdvanceParams().get(SanClientService.IS_SANCLIENT);
        assertEquals(task.getAdvanceParams().get(SanClientService.IS_SANCLIENT), Boolean.FALSE.toString());
    }

    /**
     * 测试场景：备份任务下发sanClient备份抛出异常
     * 前置条件：代理主机部分配置sanClient
     * 检查点：抛出LegoCheckedException
     *
     * @throws IllegalAccessException 抛出异常
     */
    @Test(expected = LegoCheckedException.class)
    public void should_throw_LegoCheckedException_if_agents_no_sanClient() throws IllegalAccessException, JsonProcessingException {
        BackupServiceImpl backupService = mockBackupService(true);
        Whitebox.setInternalState(backupService, "storageUnitService", storageUnitService);
        Whitebox.setInternalState(backupService, "memberClusterService", memberClusterService);
        SanClientService sanClientService = PowerMockito.mock(SanClientService.class);
        PowerMockito.when(sanClientService.getAgentsNotConfiguredSanclient(any()))
                .thenReturn(new String[]{"9527", "9527a"});
        backupService.setSanClientService(sanClientService);
        BackupObject backupObject = mockBackupObject();
        Policy policy = mockPolicy();
        backupObject.setBackupType(policy.getAction());
        backupObject.setPolicy(policy);
        JsonNode jsonNode = backupObject.getPolicy().getExtParameters();
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("storage_id", "id");
        ObjectMapper mapper=new ObjectMapper();
        JsonNode jsonNode1 = mapper.readTree("{\"storage_id\":\"123456\",\"storage_type\":\"pacific\"}");
        if (jsonNode instanceof ObjectNode) {
            ((ObjectNode)jsonNode).put("storage_info",jsonNode1 );
        }
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        StorageUnitVo storageUnitVo = new StorageUnitVo();
        storageUnitVo.setDeviceId("deviceId");
        storageUnitVo.setPoolId("poolId");
        response.setRecords(Collections.singletonList(storageUnitVo));
        PowerMockito.when(memberClusterService.getCurrentClusterEsn()).thenReturn("esn");
        PowerMockito.when(storageUnitService.pageQueryStorageUnits(any(), anyInt(), anyInt())).thenReturn(response);
        BackupTask task = backupService.backup(backupObject);
    }

    private BackupServiceImpl mockBackupService(Boolean isAgents) throws IllegalAccessException {
        ProviderManager manager = PowerMockito.mock(ProviderManager.class);
        ResourceService resourceService = PowerMockito.mock(ResourceService.class);
        QosCommonRestApi qosCommonRestApi = PowerMockito.mock(QosCommonRestApi.class);
        ProtectAgentSelector selector = PowerMockito.mock(ProtectAgentSelector.class);
        RedissonClient redissonClient = PowerMockito.mock(RedissonClient.class);
        MemberClusterService memberClusterService = PowerMockito.mock(MemberClusterService.class);
        AvailableAgentManagementDomainService domainService = PowerMockito.mock(AvailableAgentManagementDomainService.class);
        CommonAgentService commonAgentService = Mockito.mock(CommonAgentService.class);
        SelectorManager selectorManager = Mockito.mock(SelectorManager.class);
        OpServiceHelper opServiceHelper = Mockito.mock(OpServiceHelper.class);

        List<Endpoint> endpointList = new ArrayList<>();
        if(isAgents) {
            Endpoint endpoint = new Endpoint();
            endpoint.setIp("1.1.1.1");
            endpointList.add(endpoint);
        }
        ProtectedEnvironmentService protectedEnvironmentService = PowerMockito.mock(ProtectedEnvironmentService.class);

        DmeUnifiedRestApi unifiedRestApi = PowerMockito.mock(DmeUnifiedRestApi.class);

        RepositoryStrategyManager repositoryStrategyManager = mockRepositoryStrategyManager();

        PowerMockito.when(resourceService.getLanFreeConfig(any(), any())).thenReturn(mockAgentFcConfigMap());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockProtectedResource());
        NextBackupParams nextBackupParams = new NextBackupParams("test", "test");
        PowerMockito.when(resourceService.queryNextBackupTypeAndCause(anyString())).thenReturn(nextBackupParams);
        PowerMockito.when(protectedEnvironmentService.getEnvironmentById(any())).thenReturn(mockProtectedEnv());
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ProtectAgentSelector.class), any(), any())).thenReturn(PowerMockito.mock(ProtectAgentSelector.class));
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(BackupInterceptorProvider.class), any(), any())).thenReturn(PowerMockito.mock(BackupInterceptorProvider.class));
        PowerMockito.when(qosCommonRestApi.queryQos(any())).thenReturn(new QosBo()).thenReturn(null);
        PowerMockito.when(taskRepositoryManager.buildTargetRepositories(any(),any())).thenReturn(new ArrayList<>());
        PowerMockito.when(systemSwitchInternalService.queryByName(any())).thenReturn(mockDefaultSwitch(SwitchNameEnum.BACKUP_LINK_ENCRYPTION));
        PowerMockito.when(selectorManager.selectAgentByResource(any(), any(), any())).thenReturn(endpointList);
        PowerMockito.doNothing().when(opServiceHelper).injectVpcInfo(any());
        PowerMockito.when(resourceService.getResourceById(any())).thenReturn(mockProtectedResource());
        PowerMockito.when(domainService.getUrlByAgents(any())).thenReturn(mockUri());
        PowerMockito.when(selector.select(any(), any())).thenReturn(endpointList);
        PowerMockito.when(sanClientService.getAgentsNotConfiguredSanclient(any()))
            .thenReturn(endpointList.stream().map(Endpoint::getIp).toArray(String[]::new));
        PowerMockito.doNothing().when(sanClientService).fillAgentParams(any());

        PowerMockito.when(selector.select(any(),any())).thenReturn(endpointList);
        PowerMockito.when(resourceService.queryNextBackupTypeAndCause(anyString())).thenReturn(new NextBackupParams("123"));
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.anyString(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        BackupServiceImpl backupService = new BackupServiceImpl(manager, resourceService, protectedEnvironmentService, qosCommonRestApi, unifiedRestApi);
        backupService.setCommonAgentService(commonAgentService);

        MemberModifier.field(BackupServiceImpl.class, "taskRepositoryManager")
            .set(backupService, taskRepositoryManager);
        MemberModifier.field(BackupServiceImpl.class, "systemSwitchInternalService")
            .set(backupService, systemSwitchInternalService);
        backupService.setMemberClusterService(memberClusterService);
        backupService.setRepositoryStrategyManager(repositoryStrategyManager);
        backupService.setCopyRestApi(copyRestApi);
        backupService.setDeployTypeService(deployTypeService);
        backupService.setAvailableAgentManagementDomainService(domainService);
        PowerMockito.when(manager.findProvider(ArgumentMatchers.eq(ProtectAgentSelector.class), any(), any())).thenReturn(selector);
        JobBo jobBo = new JobBo();
        jobBo.setUserId(UUID.randomUUID().toString());
        JobService jobService = Mockito.mock(JobService.class);
        Mockito.when(jobService.queryJob(anyString())).thenReturn(jobBo);
        backupService.setJobService(jobService);

        UserService userService = Mockito.mock(UserService.class);
        UserInnerResponse userInnerResponse = new UserInnerResponse();
        Mockito.when(userService.getUserInfoByUserId(eq(jobBo.getUserId()))).thenReturn(userInnerResponse);
        backupService.setUserService(userService);
        backupService.setRedissonClient(redissonClient);
        backupService.setDeployTypeService(deployTypeService);
        backupService.setSanClientService(sanClientService);
        backupService.setAvailableAgentManagementDomainService(domainService);
        backupService.setSelectorManager(selectorManager);
        backupService.setOpServiceHelper(opServiceHelper);
        return backupService;
    }

    private Map<String, String> mockAgentFcConfigMap() {
        HashMap map = new HashMap();
        map.put("123", "true");
        return map;
    }

    private SystemSwitchDto mockDefaultSwitch(SwitchNameEnum name) {
        SystemSwitchDto systemSwitch = new SystemSwitchDto();
        systemSwitch.setName(name);
        systemSwitch.setStatus(SwitchStatusEnum.OFF);
        return systemSwitch;
    }

    private RepositoryStrategyManager mockRepositoryStrategyManager() {
        Map<String, RepositoryStrategy> map = new HashMap<>();
        RepositoryStrategy s3Strategy = PowerMockito.mock(RepositoryStrategy.class);
        RepositoryStrategy nfsStrategy = PowerMockito.mock(RepositoryStrategy.class);
        PowerMockito.when(s3Strategy.getRepository(any())).thenReturn(mockRepository(RepositoryProtocolEnum.S3.getProtocol()));
        PowerMockito.when(nfsStrategy.getRepository(any())).thenReturn(mockRepository(RepositoryProtocolEnum.NFS.getProtocol()));
        map.put("nativeNfsRepositoryStrategy", nfsStrategy);
        map.put("s3RepositoryStrategy", s3Strategy);
        return new RepositoryStrategyManager(map);
    }

    private StorageRepository mockRepository(int protocol) {
        StorageRepository repository = new StorageRepository();
        repository.setProtocol(protocol);
        return repository;
    }

    private BackupObject mockBackupObject() {
        BackupObject backupObject = new BackupObject();
        backupObject.setRequestId(REQUEST_ID);
        backupObject.setTaskId(REQUEST_ID);
        String protectedObjectJson
            = "{\"name\":\"small_o1\",\"type\":\"Fileset\",\"sub_type\":\"HDFSFileset\",\"resource_id\":\"266ea41d-adf5-480b-af50-15b940c2b846\",\"ext_parameters\":{\"before_protect_script\":null,\"after_protect_script\":null,\"protect_failed_script\":null, \"test_param\":\"test_value\"},\"sla_id\":\"8110c749-4eb9-488f-921d-9210374bd7af\",\"sla_name\":\"gtest\",\"path\":\"localhost.localdomain\",\"env_id\":\"3f38568b0342bbf4557d2edc0261b565\",\"env_type\":\"BigData\",\"status\":1}";
        ProtectedObject protectedObject = JSONObject.fromObject(protectedObjectJson).toBean(ProtectedObject.class);
        backupObject.setProtectedObject(protectedObject);
        Sla sla = new Sla();
        sla.setPolicyList(Collections.singletonList(mockPolicy()));
        backupObject.setSla(sla);
        return backupObject;
    }

    private Policy mockPolicy() {
        String policyJson
            = "{\"uuid\":\"3bd9e448-816d-43b1-9b16-1feca34ece65\",\"name\":\"full\",\"action\":\"full\",\"ext_parameters\":{\"qos_id\":\"qos_id\", \"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5, \"file_scan_channel_number\":20, \"read_and_send_channel_number\":1, \"source_deduplication\":true},\"retention\":{\"retention_type\":2,\"duration_unit\":\"MO\",\"retention_duration\":1},\"schedule\":{\"trigger\":1,\"interval\":1,\"interval_unit\":\"d\",\"start_time\":\"2021-04-20T00:30:00\",\"window_start\":\"00:30:00\",\"window_end\":\"00:30:00\",\"days_of_month\":null,\"days_of_year\":null,\"trigger_action\":null,\"days_of_week\":null},\"type\":\"backup\"}";
        return JSONObject.fromObject(policyJson).toBean(Policy.class);
    }
    private ProtectedEnvironment mockProtectedEnv() {
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        protectedEnvironment.setName("HDFS");
        protectedEnvironment.setType("BigData");
        protectedEnvironment.setSubType("HDFS");
        protectedEnvironment.setUuid(ENV_ID);
        return protectedEnvironment;
    }

    private Optional<ProtectedResource> mockProtectedResource() {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setName(NAME);
        protectedResource.setSubType(HDFS_FILESET);
        protectedResource.setUuid(RESOURCE_ID);
        return Optional.of(protectedResource);
    }

    private URI mockUri() {
        try {
            return new URI("test");
        } catch (URISyntaxException e) {
            throw new IllegalStateException(e.getMessage());
        }
    }
}
