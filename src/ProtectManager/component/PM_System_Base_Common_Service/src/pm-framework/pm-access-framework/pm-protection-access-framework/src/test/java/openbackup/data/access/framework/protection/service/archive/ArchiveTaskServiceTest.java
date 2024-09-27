package openbackup.data.access.framework.protection.service.archive;

import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.BDDMockito.given;

import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.protection.common.constants.ArchivePolicyKeyConstant;
import openbackup.data.access.framework.protection.mocks.RepositoryMocker;

import openbackup.data.access.framework.protection.service.context.ContextManager;
import openbackup.data.access.framework.protection.service.job.InternalApiHub;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.protection.access.provider.sdk.base.Qos;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.sla.common.constants.ExtParamsConstants;

import openbackup.system.base.common.msg.NotifyManager;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.protection.model.QosBo;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.api.support.membermodification.MemberModifier;
import org.powermock.modules.junit4.PowerMockRunner;
import org.powermock.reflect.Whitebox;

import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

/**
 * ArchiveTaskService 的单元测试用例集合
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2022/1/14
 **/
@RunWith(PowerMockRunner.class)
public class ArchiveTaskServiceTest {
    @InjectMocks
    private ArchiveTaskService archiveTaskService;
    @Mock
    private ArchiveRepositoryService archiveRepositoryService;
    @Mock
    private ContextManager contextManager;
    @Mock
    private NotifyManager notifyManager;
    @Mock
    private InternalApiHub internalApiHub;
    @Mock
    private UserQuotaManager userQuotaManager;
    @Mock
    private JobService jobService;
    @Mock
    private QosCommonRestApi commonRestApi;
    @Mock
    private ClusterNativeApi clusterNativeApi;

    @Before
    public void init() throws IllegalAccessException {
        given(internalApiHub.getQosCommonRestApi()).willReturn(commonRestApi);
        MemberModifier.field(ArchiveTaskService.class, "clusterNativeApi")
            .set(archiveTaskService, clusterNativeApi);
    }

    /**
     * 用例名称：验证根据归档策略高级参数获取磁带库存储库信息成功<br/>
     * 前置条件：无<br/>
     * check点：1.返回存储库对象非空 2.字段值与期望一致<br/>
     */
    @Test
    public void should_return_repository_when_getRepositoryFromPolicyExtParameters_given_ext_params_contains_tape_id() {
        // Given
        final RepositoryProtocolEnum protocol = RepositoryProtocolEnum.TAPE;
        String mediaSetId = UUID.randomUUID().toString();
        final JSONObject extParams = new JSONObject();
        extParams.set(ArchivePolicyKeyConstant.STORAGE_ID_KEY, mediaSetId);
        extParams.set(ArchivePolicyKeyConstant.PROTOCOL_KEY, protocol.getProtocol());
        given(archiveRepositoryService.queryRepository(eq(mediaSetId), eq(protocol))).willReturn(
            RepositoryMocker.mockTapRepository());
        // When
        final StorageRepository storageRepository = archiveTaskService.getRepositoryFromPolicyExtParameters(extParams,
            false);
        // Then
        Assert.assertNotNull(storageRepository.getId());
        Assert.assertEquals(Integer.valueOf(RepositoryProtocolEnum.TAPE.getProtocol()),
            storageRepository.getProtocol());
        Assert.assertEquals(Integer.valueOf(RepositoryTypeEnum.DATA.getType()), storageRepository.getType());
        Assert.assertEquals("mediaSetName", storageRepository.getPath());
        Assert.assertFalse(storageRepository.getLocal());
        Assert.assertNotNull(storageRepository.getEndpoint());
        Assert.assertEquals("Local", storageRepository.getEndpoint().getIp());
        Assert.assertNull(storageRepository.getExtendInfo());
    }

    /**
     * 用例名称：验证根据归档策略高级参数获取磁带库存储库信息成功<br/>
     * 前置条件：无<br/>
     * check点：1.返回存储库对象非空 2.字段值与期望一致<br/>
     */
    @Test
    public void should_return_repository_when_getRepositoryFromPolicyExtParameters_given_ext_params_contains_tape_id_from_storage_list() {
        // Given
        final RepositoryProtocolEnum protocol = RepositoryProtocolEnum.TAPE;
        String mediaSetId = "storageId111";
        final JSONObject extParams = new JSONObject();
        extParams.set(ArchivePolicyKeyConstant.STORAGE_ID_KEY, mediaSetId);
        extParams.set(ArchivePolicyKeyConstant.PROTOCOL_KEY, protocol.getProtocol());
        extParams.set(ArchivePolicyKeyConstant.STORAGE_LIST_KEY, createStorageListWithOneStorage());
        given(clusterNativeApi.getCurrentEsn()).willReturn("esn111");
        given(archiveRepositoryService.queryRepository(eq(mediaSetId), eq(protocol))).willReturn(
            RepositoryMocker.mockTapRepository());
        // When
        final StorageRepository storageRepository = archiveTaskService.getRepositoryFromPolicyExtParameters(extParams,
            false);
        // Then
        Assert.assertNotNull(storageRepository.getId());
        Assert.assertEquals(Integer.valueOf(RepositoryProtocolEnum.TAPE.getProtocol()),
            storageRepository.getProtocol());
        Assert.assertEquals(Integer.valueOf(RepositoryTypeEnum.DATA.getType()), storageRepository.getType());
        Assert.assertEquals("mediaSetName", storageRepository.getPath());
        Assert.assertFalse(storageRepository.getLocal());
        Assert.assertNotNull(storageRepository.getEndpoint());
        Assert.assertEquals("Local", storageRepository.getEndpoint().getIp());
        Assert.assertNull(storageRepository.getExtendInfo());
    }

    /**
     * 用例名称：验证id存在时查询Qos信息成功<br/>
     * 前置条件：无<br/>
     * check点：1.返回qos象非空 2.字段值与期望一致<br/>
     */
    @Test
    public void should_return_qos_when_queryQos_given_id_exist() {
        // Given
        String qosId = UUID.randomUUID().toString();
        final QosBo qosBo = new QosBo();
        qosBo.setUuid(qosId);
        qosBo.setName("test");
        qosBo.setSpeedLimit(10);
        given(commonRestApi.queryQos(eq(qosId))).willReturn(qosBo);
        // When
        final Optional<Qos> optionalQos = archiveTaskService.queryQos(qosId);
        // Then
        Assert.assertTrue(optionalQos.isPresent());
        final Qos qos = optionalQos.get();
        Assert.assertEquals(10, qos.getBandwidth());
    }

    /**
     * 用例名称：验证id为空时查询Qos信息成功<br/>
     * 前置条件：无<br/>
     * check点：1.返回qos对象为空<br/>
     */
    @Test
    public void should_return_qos_when_queryQos_given_id_empty() {
        // When
        final Optional<Qos> optionalQos = archiveTaskService.queryQos("");
        // Then
        Assert.assertFalse(optionalQos.isPresent());
    }

    /**
     * 用例名称：验证id不存在时查询Qos信息成功<br/>
     * 前置条件：无<br/>
     * check点：1.返回qos对象为空<br/>
     */
    @Test
    public void should_return_qos_when_queryQos_given_id_not_exist() {
        // Given
        String qosId = UUID.randomUUID().toString();
        given(commonRestApi.queryQos(eq(qosId))).willReturn(null);
        // When
        final Optional<Qos> optionalQos = archiveTaskService.queryQos(qosId);
        // Then
        Assert.assertFalse(optionalQos.isPresent());
    }

    /**
     * 用例名称：获得副本归档类型
     * 前置条件：无<br/>
     * check点：1.返回副本生成类型是否是归档类型
     */
    @Test
    public void select_copy_generate_archive_type_success() throws Exception {
        CopyGeneratedByEnum result = Whitebox.invokeMethod(archiveTaskService, "selectCopyGenerateType", 7);
        Assert.assertEquals(CopyGeneratedByEnum.BY_TAPE_ARCHIVE, result);
    }

    @Test
    public void update_date_before_reduction_test() throws Exception {
        Field jobService1 = archiveTaskService.getClass().getDeclaredField("jobService");
        jobService1.setAccessible(true);
        jobService1.set(archiveTaskService, jobService);
        String properties
            = "{\"metaPathSuffix\":\"\",\"dataPathSuffix\":\"\",\"isAggregation\":\"false\",\"dataAfterReduction\":39,\"format\":0,\"snapshots\":[{\"id\":\"702@aa551abb-1730-4e95-9375-e3adfedda8ff\",\"parentName\":\"Storage_7707428c5e5d4c4ba26f2db956bfb09f\"}],\"verifyStatus\":\"3\",\"repositories\":[{\"type\":2,\"protocol\":5,\"role\":0,\"remotePath\":[{\"type\":1,\"path\":\"/Storage_CacheDataRepository/7707428c5e5d4c4ba26f2db956bfb09f\",\"id\":\"39\"}],\"extendInfo\":{\"esn\":\"2102354DEY10MA000001\"}},{\"type\":1,\"protocol\":6,\"role\":0,\"remotePath\":[{\"type\":0,\"path\":\"/Storage_7707428c5e5d4c4ba26f2db956bfb09f/source_policy_7707428c5e5d4c4ba26f2db956bfb09f_Context_Global_MD\",\"id\":\"702\"},{\"type\":1,\"path\":\"/Storage_7707428c5e5d4c4ba26f2db956bfb09f/source_policy_7707428c5e5d4c4ba26f2db956bfb09f_Context\",\"id\":\"702\"}],\"extendInfo\":{\"esn\":\"2102354DEY10MA000001\",\"securityStyle\":\"2\",\"copy_format\":0}}],\"dataBeforeReduction\":616,\"maxSizeAfterAggregate\":\"0\",\"multiFileSystem\":\"false\",\"maxSizeToAggregate\":\"0\"}";
        JSONObject propertiesJson = JSONObject.fromObject(properties);
        propertiesJson.put("size", "616");
        archiveTaskService.updateDataBeforeReduction("job1", propertiesJson.toString());
        Assert.assertTrue(true);
    }

    @Test
    public void set_copy_property_test() throws Exception {
        Map extendsInfo = new HashMap<>();
        extendsInfo.put(JobExtendInfoKeys.DATA_BEFORE_REDUCTION, 5);
        ArchiveContext archiveContext = PowerMockito.mock(ArchiveContext.class);
        CopyInfo copyInfo = new CopyInfo();
        String properties
            = "{\"metaPathSuffix\":\"\",\"dataPathSuffix\":\"\",\"isAggregation\":\"false\",\"dataAfterReduction\":39,\"format\":0,\"snapshots\":[{\"id\":\"702@aa551abb-1730-4e95-9375-e3adfedda8ff\",\"parentName\":\"Storage_7707428c5e5d4c4ba26f2db956bfb09f\"}],\"verifyStatus\":\"3\",\"repositories\":[{\"type\":2,\"protocol\":5,\"role\":0,\"remotePath\":[{\"type\":1,\"path\":\"/Storage_CacheDataRepository/7707428c5e5d4c4ba26f2db956bfb09f\",\"id\":\"39\"}],\"extendInfo\":{\"esn\":\"2102354DEY10MA000001\"}},{\"type\":1,\"protocol\":6,\"role\":0,\"remotePath\":[{\"type\":0,\"path\":\"/Storage_7707428c5e5d4c4ba26f2db956bfb09f/source_policy_7707428c5e5d4c4ba26f2db956bfb09f_Context_Global_MD\",\"id\":\"702\"},{\"type\":1,\"path\":\"/Storage_7707428c5e5d4c4ba26f2db956bfb09f/source_policy_7707428c5e5d4c4ba26f2db956bfb09f_Context\",\"id\":\"702\"}],\"extendInfo\":{\"esn\":\"2102354DEY10MA000001\",\"securityStyle\":\"2\",\"copy_format\":0}}],\"dataBeforeReduction\":616,\"maxSizeAfterAggregate\":\"0\",\"multiFileSystem\":\"false\",\"maxSizeToAggregate\":\"0\"}";
        JSONObject propertiesJson = JSONObject.fromObject(properties);
        copyInfo.setProperties(propertiesJson.toString());
        StorageRepository archiveRepository = new StorageRepository();
        Whitebox.invokeMethod(archiveTaskService, "setCopyProperty", extendsInfo, archiveContext, copyInfo,
            archiveRepository);
        JSONObject copyProperty = JSONObject.fromObject(copyInfo.getProperties());
        Assert.assertEquals(5, copyProperty.get(CopyPropertiesKeyConstant.SIZE));
    }

    private JSONArray createStorageListWithOneStorage() {
        Map<String, Object> storageListItemMap = new HashMap<>();
        storageListItemMap.put(ExtParamsConstants.ESN, "esn111");
        storageListItemMap.put(ExtParamsConstants.STORAGE_ID, "storageId111");
        JSONObject storageListItemJsonObject = JSONObject.fromObject(storageListItemMap);

        JSONArray storageListJsonArray = new JSONArray();
        storageListJsonArray.add(storageListItemJsonObject);

        return storageListJsonArray;
    }
}
