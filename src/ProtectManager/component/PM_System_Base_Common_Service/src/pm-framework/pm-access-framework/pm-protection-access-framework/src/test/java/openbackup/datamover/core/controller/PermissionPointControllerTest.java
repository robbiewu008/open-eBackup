/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.datamover.core.controller;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import openbackup.data.access.client.sdk.api.framework.dme.DmeCopyInfo;
import openbackup.data.access.client.sdk.api.framework.dme.DmeUnifiedRestApi;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.controller.AccessPointController;
import openbackup.data.access.framework.protection.dto.ArchiveRequestDto;
import openbackup.data.access.framework.protection.dto.CopyReplicationMetadata;
import openbackup.data.access.framework.protection.service.archive.ArchiveJobService;
import openbackup.data.access.framework.protection.service.quota.UserQuotaManager;
import openbackup.data.access.framework.protection.service.replication.ReplicationService;
import openbackup.data.access.framework.restore.dto.RestoreRequestDto;
import openbackup.data.protection.access.provider.sdk.archive.ArchiveProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;
import openbackup.data.protection.access.provider.sdk.copy.ReplicationOriginCopyDuration;
import openbackup.data.protection.access.provider.sdk.replication.ReplicationProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreProvider;
import openbackup.data.protection.access.provider.sdk.restore.RestoreRequest;
import com.huawei.oceanprotect.functionswitch.enums.FunctionTypeEnum;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.enums.TimeUnitEnum;
import openbackup.system.base.common.enums.UserTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.anti.model.AntiRansomwareScheduleRes;
import openbackup.system.base.sdk.auth.AuthRestApi;
import openbackup.system.base.sdk.common.model.UuidObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.protection.constants.SlaConstant;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.RetentionBo;
import openbackup.system.base.sdk.protection.model.SlaBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.Filter;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import com.huawei.oceanprotect.system.base.user.service.UserService;
import openbackup.system.base.util.MessageTemplate;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.ArgumentMatchers;
import org.mockito.InjectMocks;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.UUID;

import static org.assertj.core.api.Assertions.assertThat;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.powermock.api.mockito.PowerMockito.whenNew;

/**
 * JobControllerTest LLT
 *
 * @author m00576658
 * @since 2021-03-05
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest(AccessPointController.class)
public class PermissionPointControllerTest {
    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private BackupStorageApi backupStorageApi;

    @Mock
    private DmeUnifiedRestApi dmeUnifiedRestApi;

    @Mock
    private ResourceService resourceService;

    @Mock
    private ProviderManager registry;

    @Mock
    private AuthRestApi authRestApi;

    @Mock
    private MessageTemplate<?> messageTemplate;

    @Mock
    private ArchiveJobService archiveJobService;

    @Mock
    private UserQuotaManager userQuotaManager;

    @Mock
    private UserService userService;

    @InjectMocks
    private AccessPointController accessPointController;

    @Mock
    private ReplicationService replicationService;

    @Before
    public void init() {
        Date date = PowerMockito.mock(Date.class);
        PowerMockito.when(replicationService.getGenerateTime(anyLong())).thenReturn(date);
        AntiRansomwareScheduleRes scheduleRes = PowerMockito.mock(AntiRansomwareScheduleRes.class);
        PowerMockito.when(replicationService.getAntiRansomwareSchedule(anyString())).thenReturn(scheduleRes);
    }

    /**
     * 用例名称：验证创建恢复接口中，对象参数转换正常<br/>
     * 前置条件：无<br/>
     * check点：1.没有发生异常  2.对象中的属性符合预期<br/>
     *
     * @throws Exception Exception
     */
    @Test
    public void should_has_same_properties_when_testCreateRestoreTask_given_correct_param() throws Exception {
        // Given
        RestoreRequestDto restoreRequestDto = new RestoreRequestDto();
        restoreRequestDto.setCopyId(UUID.randomUUID().toString().replace("-", ""));
        restoreRequestDto.setObjectType("Oracle");
        JSONObject content = new JSONObject();
        content.put("key1", "value1");
        content.put("key2", "value2");
        final Filter filter = new Filter();
        filter.setFilterType(1);
        filter.setFilterMode(2);
        filter.setContent(content);
        restoreRequestDto.setFilters(Collections.singletonList(filter));
        Copy copy = PowerMockito.mock(Copy.class);
        PowerMockito.when(copyRestApi.queryCopyByID(ArgumentMatchers.anyString())).thenReturn(copy);
        RestoreProvider restoreProvider = PowerMockito.mock(RestoreProvider.class);
        PowerMockito.when(registry.findProvider(any(), any())).thenReturn(restoreProvider);
        RestoreRequest request = new RestoreRequest();
        whenNew(RestoreRequest.class).withNoArguments().thenReturn(request);
        // When
        accessPointController.createRestoreTask(restoreRequestDto);
        // Then
        assertThat(request).isEqualToIgnoringGivenFields(restoreRequestDto, "filters");
        assertThat(request.getFilters()).hasSize(1);
        assertThat(request.getFilters().get(0)).isEqualToComparingFieldByField(filter);
    }

    @Test
    public void test_import_copy() {
        CopyReplicationImport importParam = getCopyReplicationImport();

        UserInnerResponse userInnerResponse = new UserInnerResponse();
        userInnerResponse.setUserType(UserTypeEnum.SAML.getValue());
        PowerMockito.when(userService.getUserInfoByUserId(Mockito.any())).thenReturn(userInnerResponse);
        boolean result = accessPointController.importReplicationCopy(importParam);
        Assert.assertTrue(result);
    }

    /**
     * 用例名称：验证副本导入 指定副本归属hcs用户
     * 前置条件：无
     * check点：1.没有发生异常
     */
    @Test
    public void test_import_copy_hcs() {
        CopyReplicationImport importParam = getCopyReplicationImport();

        UserInnerResponse userInnerResponse = new UserInnerResponse();
        userInnerResponse.setUserType(UserTypeEnum.HCS.getValue());
        PowerMockito.when(userService.getUserInfoByUserId(Mockito.any())).thenReturn(userInnerResponse);
        boolean result = accessPointController.importReplicationCopy(importParam);
        Assert.assertTrue(result);
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

    @Test
    public void test_creat_archive_task() {
        ArchiveRequestDto archiveRequest = new ArchiveRequestDto();
        archiveRequest.setCopyId(UUID.randomUUID().toString());
        archiveRequest.setPolicy(JSONObject.fromObject(new PolicyBo()).toString());
        archiveRequest.setResourceSubType(ResourceSubTypeEnum.ORACLE.getType());
        archiveRequest.setResourceType(ResourceTypeEnum.HOST.getType());
        ArchiveProvider provider = PowerMockito.mock(ArchiveProvider.class);
        PowerMockito.when(registry.findProvider(any(), any())).thenReturn(provider);
        accessPointController.createArchiveTask(archiveRequest);
        Mockito.verify(archiveJobService, Mockito.times(1)).buildJobRequest(any());
    }

    private PolicyBo generatePolicy(RetentionTypeEnum retentionType, String type) {
        PolicyBo policyBo = new PolicyBo();
        policyBo.setType(type);
        RetentionBo retentionBo = new RetentionBo();
        retentionBo.setRetentionType(retentionType.getType());
        retentionBo.setRetentionDuration(5);
        retentionBo.setDurationUnit(TimeUnitEnum.DAYS.getUnit());
        policyBo.setRetention(retentionBo);
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(SlaConstant.QOS_ID, "1");
        jsonObject.set("channel_number", 4);
        jsonObject.set("encryption", true);
        jsonObject.set("link_deduplication", true);
        jsonObject.set("is_worm", true);
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node;
        try {
            node = mapper.readTree(jsonObject.toString());
        } catch (JsonProcessingException e) {
            throw new RuntimeException(e);
        }
        policyBo.setExtParameters(node);
        return policyBo;
    }

    private SlaBo generateSla() {
        SlaBo sla = new SlaBo();
        List<PolicyBo> policyList = new ArrayList<>();
        policyList.add(generatePolicy(RetentionTypeEnum.TEMPORARY, FunctionTypeEnum.BACKUP.value()));
        policyList.add(generatePolicy(RetentionTypeEnum.TEMPORARY, FunctionTypeEnum.REPLICATION.value()));
        sla.setPolicyList(policyList);
        return sla;
    }

    private CopyReplicationImport getCopyReplicationImport() {
        String properties
            = "{\"backup_id\":\"9191a1c7-ae44-4506-b484-4acd9f017ede\",\"oracle_metadata\":{\"ORACLEPARAM\":{\"db_name\":\"orcl\",\"is_asm\":0,\"is_cluster\":false,\"is_os_verify\":1,\"os_type\":\"linux\",\"verify_status\":\"true\",\"version\":\"18.0.0.0.0\"},\"PFILE\":\"{\\n   \\\"*.audit_file_dest\\\" : \\\"'/u01/app/oracle/admin/orcl/adump'\\\",\\n   \\\"*.audit_trail\\\" : \\\"'db'\\\",\\n   \\\"*.compatible\\\" : \\\"'18.0.0'\\\",\\n   \\\"*.control_files\\\" : \\\"'/u01/app/oracle/oradata/ORCL/controlfile/o1_mf_j1l9l0o6_.ctl','/u01/app/oracle/fast_recovery_area/ORCL/controlfile/o1_mf_j1l9l0p7_.ctl'\\\",\\n   \\\"*.db_block_size\\\" : \\\"8192\\\",\\n   \\\"*.db_create_file_dest\\\" : \\\"'/u01/app/oracle/oradata'\\\",\\n   \\\"*.db_name\\\" : \\\"'orcl'\\\",\\n   \\\"*.db_recovery_file_dest\\\" : \\\"'/u01/app/oracle/fast_recovery_area'\\\",\\n   \\\"*.db_recovery_file_dest_size\\\" : \\\"8106m\\\",\\n   \\\"*.diagnostic_dest\\\" : \\\"'/u01/app/oracle'\\\",\\n   \\\"*.dispatchers\\\" : \\\"'(PROTOCOL=TCP) (SERVICE=orclXDB)'\\\",\\n   \\\"*.open_cursors\\\" : \\\"300\\\",\\n   \\\"*.pga_aggregate_target\\\" : \\\"782m\\\",\\n   \\\"*.processes\\\" : \\\"320\\\",\\n   \\\"*.remote_login_passwordfile\\\" : \\\"'EXCLUSIVE'\\\",\\n   \\\"*.sga_target\\\" : \\\"2346m\\\",\\n   \\\"*.undo_tablespace\\\" : \\\"'UNDOTBS1'\\\",\\n   \\\"orcl.__data_transfer_cache_size\\\" : \\\"0\\\",\\n   \\\"orcl.__db_cache_size\\\" : \\\"1744830464\\\",\\n   \\\"orcl.__inmemory_ext_roarea\\\" : \\\"0\\\",\\n   \\\"orcl.__inmemory_ext_rwarea\\\" : \\\"0\\\",\\n   \\\"orcl.__java_pool_size\\\" : \\\"16777216\\\",\\n   \\\"orcl.__large_pool_size\\\" : \\\"33554432\\\",\\n   \\\"orcl.__oracle_base\\\" : \\\"'/u01/app/oracle'#ORACLE_BASE set from environment\\\",\\n   \\\"orcl.__pga_aggregate_target\\\" : \\\"822083584\\\",\\n   \\\"orcl.__sga_target\\\" : \\\"2466250752\\\",\\n   \\\"orcl.__shared_io_pool_size\\\" : \\\"134217728\\\",\\n   \\\"orcl.__shared_pool_size\\\" : \\\"520093696\\\",\\n   \\\"orcl.__streams_pool_size\\\" : \\\"0\\\"\\n}\\n\"}}";
        JSONObject jsonObject = JSONObject.fromObject(properties);
        jsonObject.put("origin_copy_time_stamp", System.currentTimeMillis() / 1000);
        CopyReplicationImport importParam = new CopyReplicationImport();
        importParam.setTimestamp(1615534657L);
        importParam.setGeneratedTime(1615534657L);
        importParam.setProperties(jsonObject);
        ReplicationOriginCopyDuration originCopyDuration = new ReplicationOriginCopyDuration();
        originCopyDuration.setRetentionType(RetentionTypeEnum.PERMANENT.getType());
        importParam.setOriginCopyDuration(originCopyDuration);
        CopyReplicationMetadata copyReplicationMetadata = new CopyReplicationMetadata();
        SlaBo sla = generateSla();
        copyReplicationMetadata.setSla(JSONObject.fromObject(sla));

        ResourceEntity resourceEntity = getResourceEntity();
        copyReplicationMetadata.setResource(JSONObject.fromObject(resourceEntity));

        PolicyBo policyBo = generatePolicy(RetentionTypeEnum.PERMANENT, FunctionTypeEnum.REPLICATION.value());
        copyReplicationMetadata.setReplicationPolicy(policyBo);

        importParam.setMetadata(JSONObject.fromObject(copyReplicationMetadata).toString());

        ReplicationProvider replicationProvider = PowerMockito.mock(ReplicationProvider.class);
        PowerMockito.when(registry.findProviderOrDefault(any(), any(), any())).thenReturn(replicationProvider);
        TokenBo.UserInfo userInfo = PowerMockito.mock(TokenBo.UserInfo.class);
        PowerMockito.when(authRestApi.queryUserInfoByName(ArgumentMatchers.anyString())).thenReturn(userInfo);
        DmeCopyInfo dmeCopyInfo = new DmeCopyInfo();
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put(CopyPropertiesKeyConstant.KEY_BACKUP_REPOSITORY_ID, "test");
        dmeCopyInfo.setExtendInfo(extendInfo);
        PowerMockito.when(dmeUnifiedRestApi.getCopyInfo(ArgumentMatchers.anyString())).thenReturn(dmeCopyInfo);
        NasDistributionStorageDetail nasDistributionStorageDetail = new NasDistributionStorageDetail();
        nasDistributionStorageDetail.setName("test");
        PowerMockito.when(backupStorageApi.getDetail(ArgumentMatchers.anyString()))
            .thenReturn(nasDistributionStorageDetail);

        PowerMockito.when(messageTemplate.send(anyString(), anyString())).thenReturn(null);
        UuidObject uuidObject = new UuidObject();
        uuidObject.setUuid(UUID.randomUUID().toString().replace("-", ""));
        PowerMockito.when(copyRestApi.saveCopy(any())).thenReturn(uuidObject);
        PowerMockito.when(resourceService.getResourceById(ArgumentMatchers.anyString()))
            .thenReturn(mockProtectedResource());
        return importParam;
    }

    private ResourceEntity getResourceEntity() {
        ResourceEntity resourceEntity = new ResourceEntity();
        resourceEntity.setSubType(ResourceSubTypeEnum.IMPORT_COPY.getType());
        resourceEntity.setUuid(UUID.randomUUID().toString());
        resourceEntity.setUserId("samlUserId");
        return resourceEntity;
    }

    private CopyReplicationMetadata getCopyReplicationMetadata() throws JsonProcessingException {
        CopyReplicationMetadata copyReplicationMetadata = new CopyReplicationMetadata();
        SlaBo sla = generateSla();
        copyReplicationMetadata.setSla(JSONObject.fromObject(sla));
        ResourceEntity resourceEntity = getResourceEntity();
        copyReplicationMetadata.setResource(JSONObject.fromObject(resourceEntity));
        PolicyBo policyBo = generatePolicy(RetentionTypeEnum.TEMPORARY, FunctionTypeEnum.REPLICATION.value());
        ObjectMapper mapper = new ObjectMapper();
        policyBo.setExtParameters(mapper.readTree(
            "{\"specified_scope\":[{\"copy_type\":\"year\",\"generate_time_range\":\"3\",\"retention_unit\":\"y\",\"retention_duration\":1},{\"copy_type\":\"month\",\"generate_time_range\":\"first\",\"retention_unit\":\"MO\",\"retention_duration\":2},{\"copy_type\":\"week\",\"generate_time_range\":\"fri\",\"retention_unit\":\"w\",\"retention_duration\":3}],\"qos_id\":\"\",\"protocol\":2,\"storage_id\":\"27ad6ac00b0048e195fa93f5ff39c1bc\",\"network_access\":true,\"auto_retry\":false,\"replication_target_type\":2,\"alarm_after_failure\":false}"));
        copyReplicationMetadata.setReplicationPolicy(policyBo);

        return copyReplicationMetadata;
    }

    @Test
    public void test_import_copy_specified_time_not_match() throws JsonProcessingException {
        String properties
            = "{\"backup_id\": \"b84e40c8-1eaa-403d-8e57-fcc2156dccd6\", \"backup_type\": 1, \"chain_id\": \"d3e315db607749879fea7bdb2df40fd9\", \"originStorageInfo\": \"2102353GTD10L9000001\", \"replicate_count\": 1, \"reverse_copy\": false, \"source_copy_type\": 1, \"verifyStatus\": 1, \"wormCopy\": 1}";
        CopyReplicationImport importParam = getCopyReplicationImport();
        importParam.setProperties(JSONObject.fromObject(properties));
        // 组装指定日期复制策略
        CopyReplicationMetadata copyReplicationMetadata = new CopyReplicationMetadata();
        SlaBo sla = generateSla();
        copyReplicationMetadata.setSla(JSONObject.fromObject(sla));
        ResourceEntity resourceEntity = getResourceEntity();
        copyReplicationMetadata.setResource(JSONObject.fromObject(resourceEntity));
        PolicyBo policyBo = generatePolicy(RetentionTypeEnum.TEMPORARY, FunctionTypeEnum.REPLICATION.value());

        ObjectMapper mapper = new ObjectMapper();
        JsonNode extParams = mapper.readTree(
            "{\"specified_scope\":[{\"copy_type\":\"year\",\"generate_time_range\":\"12\",\"retention_unit\":\"y\",\"retention_duration\":null},{\"copy_type\":\"month\",\"generate_time_range\":\"first\",\"retention_unit\":\"MO\",\"retention_duration\":5},{\"copy_type\":\"week\",\"generate_time_range\":\"mon\",\"retention_unit\":\"w\",\"retention_duration\":6}],\"replication_target_type\":2,\"qos_id\":\"\",\"external_system_id\":\"2\",\"link_deduplication\":true,\"link_compression\":true,\"alarm_after_failure\":true,\"start_replicate_time\":null}");
        policyBo.setExtParameters(extParams);
        copyReplicationMetadata.setReplicationPolicy(policyBo);
        importParam.setMetadata(JSONObject.fromObject(copyReplicationMetadata).toString());
        Assert.assertThrows(LegoCheckedException.class, () -> accessPointController.importReplicationCopy(importParam));
    }

    @Test
    public void test_import_copy_specified_time() throws JsonProcessingException {
        String properties
            = "{\"backup_id\": \"b84e40c8-1eaa-403d-8e57-fcc2156dccd6\", \"backup_type\": 1, \"chain_id\": \"d3e315db607749879fea7bdb2df40fd9\", \"originStorageInfo\": \"2102353GTD10L9000001\", \"replicate_count\": 1, \"reverse_copy\": false, \"source_copy_type\": 1, \"verifyStatus\": 1, \"wormCopy\": 1}";
        CopyReplicationImport importParam = getCopyReplicationImport();
        importParam.setProperties(JSONObject.fromObject(properties));
        // 组装指定日期复制策略
        CopyReplicationMetadata copyReplicationMetadata = getCopyReplicationMetadata();
        importParam.setMetadata(JSONObject.fromObject(copyReplicationMetadata).toString());
        accessPointController.importReplicationCopy(importParam);
        boolean result = accessPointController.importReplicationCopy(importParam);
        Assert.assertTrue(result);
    }

    @Test
    public void test_getWormFromSal_success()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        AccessPointController accessPointController1 = new AccessPointController();
        Class<AccessPointController> cal = AccessPointController.class;
        Method method = cal.getDeclaredMethod("getWormFromSla", new Class[] {new String().getClass()});
        method.setAccessible(true);
        boolean res = (boolean) method.invoke(accessPointController1, "");
        Assert.assertFalse(res);
    }

    @Test
    public void test_getWormFromSal_empty()
        throws NoSuchMethodException, InvocationTargetException, IllegalAccessException {
        AccessPointController accessPointController1 = new AccessPointController();
        Class<AccessPointController> cal = AccessPointController.class;
        Method method = cal.getDeclaredMethod("getWormFromSla", new Class[] {new String().getClass()});
        method.setAccessible(true);

        boolean result = (boolean) method.invoke(accessPointController1,
            "{\"policy_list\":[{\"ext_parameters\":{\"is_worm\":true}}]}");
        Assert.assertFalse(result);

        boolean res = (boolean) method.invoke(accessPointController1,
            "{\"policy_list\":[{\"type\":\"replication\",\"ext_parameters\":{\"link_deduplication\":true}}]}");
        Assert.assertFalse(res);

        boolean res1 = (boolean) method.invoke(accessPointController1,
            "{\"policy_list\":[{\"type\":\"replication\",\"ext_parameters\":{\"is_worm\":true}}]}");
        Assert.assertTrue(res1);
    }
}
