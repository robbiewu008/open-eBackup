/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.data.access.framework.protection.service.replication;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.isNull;

import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterQueryService;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponseError;
import openbackup.data.access.client.sdk.api.framework.dmc.DmcCopyService;
import openbackup.data.access.client.sdk.api.framework.dmc.model.CopyDetail;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.DmeReplicationRestApi;
import openbackup.data.access.client.sdk.api.framework.dme.replicate.model.DmeReplicateRequest;
import openbackup.data.access.framework.copy.mng.constant.CopyPropertiesKeyConstant;
import openbackup.data.access.framework.protection.listener.v1.replication.ReplicateContext;
import openbackup.data.access.framework.protection.service.replication.AdvanceReplicationProvider;
import openbackup.data.access.framework.protection.service.repository.TaskRepositoryManager;
import openbackup.data.protection.access.provider.sdk.backup.BackupObject;
import openbackup.data.protection.access.provider.sdk.backup.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.backup.v2.StorageRepositoryCreateService;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.data.protection.access.provider.sdk.copy.CopyReplicationImport;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import com.huawei.oceanprotect.sla.infrastructure.repository.SlaRepositoryImpl;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.enums.RetentionTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.accesspoint.model.DmeRemoteDevice;
import openbackup.system.base.sdk.auth.api.AuthNativeApi;
import openbackup.system.base.sdk.cluster.ClusterInternalApi;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.cluster.model.ClusterDetailInfo;
import openbackup.system.base.sdk.cluster.model.SourceClustersParams;
import openbackup.system.base.sdk.cluster.model.StorageSystemInfo;
import openbackup.system.base.sdk.cluster.model.StorageUnitVo;
import openbackup.system.base.sdk.cluster.model.TargetClusterVo;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.protection.QosCommonRestApi;
import openbackup.system.base.sdk.protection.constants.SlaConstant;
import openbackup.system.base.sdk.protection.emuns.SlaPolicyTypeEnum;
import openbackup.system.base.sdk.protection.model.PolicyBo;
import openbackup.system.base.sdk.protection.model.QosBo;
import openbackup.system.base.sdk.protection.model.ScheduleBo;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.BackupClusterVo;
import openbackup.system.base.sdk.repository.model.BackupUnitVo;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import com.huawei.oceanprotect.system.sdk.dto.SystemSwitchDto;
import com.huawei.oceanprotect.system.sdk.enums.SwitchNameEnum;
import com.huawei.oceanprotect.system.sdk.enums.SwitchStatusEnum;
import com.huawei.oceanprotect.system.sdk.service.SystemSwitchInternalService;

import com.fasterxml.jackson.core.JsonProcessingException;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.node.TextNode;

import org.apache.commons.collections.map.ListOrderedMap;
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
import org.powermock.reflect.Whitebox;
import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.redisson.client.codec.StringCodec;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;

import java.sql.Timestamp;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;

/**
 * OracleBackupProviderTest LLT
 *
 * @author m00576658
 * @since 2021-04-17
 */
@RunWith(PowerMockRunner.class)
@PrepareForTest( {AdvanceReplicationProvider.class, ListOrderedMap.class})
@AutoConfigureMockMvc
public class AdvanceReplicationProviderTest {
    @InjectMocks
    private AdvanceReplicationProvider advanceReplicationProvider;

    @Mock
    private DmcCopyService dmcCopyService;

    @Mock
    private DmeReplicationRestApi dmeReplicationRestApi;

    @Mock
    private ClusterInternalApi clusterInternalApi;

    @Mock
    private QosCommonRestApi qosCommonRestApi;

    @Mock
    private JobCenterRestApi jobCenterRestApi;

    @Mock
    private RedissonClient redissonClient;

    @Mock
    private CopyRestApi copyRestApi;

    @Mock
    private SlaQueryService slaQueryService;

    @Mock
    private TaskRepositoryManager taskRepositoryManager;

    @Mock
    private SystemSwitchInternalService systemSwitchInternalService;

    @Mock
    private SlaRepositoryImpl slaRepository;

    @Mock
    private ClusterNativeApi clusterNativeApi;

    @Mock
    private ClusterQueryService clusterQueryService;

    @Mock
    private AuthNativeApi authNativeApi;

    @Mock
    private BackupStorageApi backupStorageApi;

    @Mock
    private StorageRepositoryCreateService storageRepositoryCreateService;

    @Mock
    private ArrayTargetClusterService arrayTargetClusterService;

    @Before
    public void init() {
        PowerMockito.when(dmeReplicationRestApi.replicate(any())).thenReturn(new DmeResponse<>());
    }

    @Test
    public void test_replicate() throws JsonProcessingException {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.get("resource"))
            .thenReturn(
                "{\"name\":\"test_upgrade\",\"path\":\"192.168.120.113\",\"root_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"parent_name\":null,\"parent_uuid\":\"82548066-bdc5-3ecb-8e3c-dfd152f80213\",\"children_uuids\":null,\"type\":\"Fileset\",\"sub_type\":\"Fileset\",\"uuid\":\"f7bfca983429460fa28210a73cefddfe\",\"created_time\":\"2023-03-16T09:45:33.557000\",\"ext_parameters\":{\"next_backup_change_cause\":null,\"next_backup_type\":null,\"consistent_backup\":false,\"cross_file_system\":false,\"backup_nfs\":false,\"backup_smb\":false,\"sparse_file_detection\":false,\"backup_continue_with_files_backup_failed\":false,\"small_file_aggregation\":true,\"aggregation_file_size\":128,\"aggregation_file_max_size\":128,\"pre_script\":null,\"post_script\":null,\"failed_script\":null},\"authorized_user\":null,\"user_id\":null,\"version\":null,\"sla_id\":\"40d9f4c2-75d4-457c-9f6d-a5a0fe35c5b2\",\"sla_name\":\"复制\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"environment_name\":\"dws-agent-2\",\"environment_endpoint\":\"192.168.120.113\",\"environment_os_type\":\"linux\",\"environment_type\":\"Host\",\"environment_sub_type\":\"UBackupAgent\",\"environment_is_cluster\":\"False\",\"environment_os_name\":\"linux\",\"extendInfo\":{\"templateName\":\"\",\"paths\":\"[{\\\"name\\\":\\\"/l30015744_test\\\"}]\",\"filters\":\"[]\",\"templateId\":\"\"}}");
        commonMock(map);
        // extracted mock值
        SlaDto slaDto = new SlaDto();
        List<PolicyDto> policyList = new ArrayList<>();
        PolicyDto policyDto = new PolicyDto();
        JsonNode node = PowerMockito.mock(JsonNode.class);
        TextNode node2 = PowerMockito.mock(TextNode.class);
        policyDto.setExtParameters(node);
        policyList.add(policyDto);
        slaDto.setPolicyList(policyList);
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(slaDto);
        PowerMockito.when(node.get(anyString())).thenReturn(node2);
        PowerMockito.when(node2.asText()).thenReturn("node2");
        // mock copies
        List<Copy> copies = getCopies();
        PowerMockito.when(copyRestApi.queryCopiesByResourceId(any())).thenReturn(copies);
        List<StorageRepository> storageRepositories = new ArrayList<>();
        PowerMockito.when(storageRepositoryCreateService.createRepositoryByStorageUnitGroup(anyString())).thenReturn(storageRepositories);
        PageListResponse<StorageUnitVo> response = new PageListResponse<>();
        List<StorageUnitVo> list = Collections.singletonList(new StorageUnitVo());
        response.setRecords(list);
        Map<String, String> queryParam = new HashMap<>();
        TargetCluster targetCluster = new TargetCluster();
        PowerMockito.when(clusterQueryService.getTargetClusterById(anyInt())).thenReturn(targetCluster);
        PowerMockito.when(arrayTargetClusterService.getStorageUnitInfo(targetCluster, queryParam, 0, 1)).thenReturn(response);
        advanceReplicationProvider.replicate(mockReplicateContext());
        Mockito.verify(clusterNativeApi, Mockito.times(1)).getCurrentEsn();
    }

    @Test
    public void test_replicate_clean_password() throws JsonProcessingException {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        ClusterDetailInfo clusterDetailInfo = mockClusterDetailInfo();
        PowerMockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(clusterDetailInfo);
        PowerMockito.when(map.get("job_id")).thenReturn(UUID.randomUUID().toString());
        QosBo qosBo = new QosBo();
        PowerMockito.when(qosCommonRestApi.queryQos(ArgumentMatchers.anyString())).thenReturn(qosBo);
        PowerMockito.when(systemSwitchInternalService.queryByName(SwitchNameEnum.REPLICATION_LINK_ENCRYPTION))
            .thenReturn(mockDefaultSwitch(SwitchNameEnum.REPLICATION_LINK_ENCRYPTION));
        qosBo.setSpeedLimit(1024);
        // extracted mock值
        SlaDto slaDto = new SlaDto();
        List<PolicyDto> policyList = new ArrayList<>();
        PolicyDto policyDto = new PolicyDto();
        JsonNode node = PowerMockito.mock(JsonNode.class);
        TextNode node2 = PowerMockito.mock(TextNode.class);
        policyDto.setExtParameters(node);
        policyList.add(policyDto);
        slaDto.setPolicyList(policyList);
        PowerMockito.when(clusterNativeApi.getCurrentEsn()).thenReturn("123");
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(slaDto);
        PowerMockito.when(node.get(anyString())).thenReturn(node2);
        PowerMockito.when(node2.asText()).thenReturn("node2");
        advanceReplicationProvider.replicate(mockReplicateContext());
        Assert.assertNotSame(new String("abc123"), clusterDetailInfo.getStorageSystem().getPassword());
    }

    @Test
    public void test_build_copy_properties() {
        CopyInfoBo copyInfo = new CopyInfoBo();
        CopyReplicationImport importParam = new CopyReplicationImport();
        JSONObject properties = new JSONObject();
        properties.set("backup_id", UUID.randomUUID().toString());
        properties.set("backup_type", 1);
        importParam.setTimestamp(System.currentTimeMillis());
        importParam.setProperties(properties);
        CopyDetail copyDetail = new CopyDetail();
        copyInfo.setResourceSubType(ResourceSubTypeEnum.ORACLE.getType());
        DmeResponse<CopyDetail> copyDetailDmeResponse = new DmeResponse<>();
        copyDetailDmeResponse.setData(copyDetail);
        PowerMockito.when(dmcCopyService.queryCopyById(ArgumentMatchers.anyString(), ArgumentMatchers.anyString()))
            .thenReturn(copyDetailDmeResponse);
        advanceReplicationProvider.buildCopyProperties(copyInfo, importParam);
        Assert.assertNull(copyInfo.getResourceType());
        copyInfo.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        advanceReplicationProvider.buildCopyProperties(copyInfo, importParam);
        Assert.assertNull(copyInfo.getResourceType());
        copyInfo.setResourceSubType(ResourceSubTypeEnum.IMPORT_COPY.getType());
        advanceReplicationProvider.buildCopyProperties(copyInfo, importParam);
        Assert.assertNull(copyInfo.getResourceType());
    }

    @Test
    public void test_applicable() {
        Assert.assertFalse(advanceReplicationProvider.applicable(null));
    }

    /**
     * 用例场景：设置反向复制副本过期类型
     * 前置条件：反向复制副本保留策略不为永久保留
     * 检查点：反向复制副本过期类不为永久保留
     */
    @Test
    public void test_reverse_replication_retention_type_success_when_retention_type_is_temporary() throws Exception {
        CopyInfoBo copyInfo = new CopyInfoBo();
        CopyReplicationImport importParam = new CopyReplicationImport();
        copyInfo.setGeneratedBy(CopyGeneratedByEnum.BY_REVERSE_REPLICATION.value());

        JSONObject importParamProperties = new JSONObject();
        importParamProperties.put("backup_type", BackupTypeEnum.FULL.getAbbreviation());
        importParam.setProperties(importParamProperties);

        copyInfo.setResourceSubType(ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType());
        CopyDetail copyDetail = new CopyDetail();
        DmeResponse<CopyDetail> copyDetailDmeResponse = new DmeResponse<>();
        copyDetailDmeResponse.setData(copyDetail);
        PowerMockito.when(dmcCopyService.queryCopyById(ArgumentMatchers.anyString(), ArgumentMatchers.anyString()))
            .thenReturn(copyDetailDmeResponse);

        ListOrderedMap copyProperties = new ListOrderedMap();
        copyProperties.put(CopyPropertiesKeyConstant.KEY_FORMAT, CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        PowerMockito.whenNew(ListOrderedMap.class).withNoArguments().thenReturn(copyProperties);

        copyInfo.setRetentionType(RetentionTypeEnum.TEMPORARY.getType());

        advanceReplicationProvider.buildCopyProperties(copyInfo, importParam);
        Assert.assertEquals(copyInfo.getRetentionType(), RetentionTypeEnum.TEMPORARY.getType().intValue());
    }

    private SystemSwitchDto mockDefaultSwitch(SwitchNameEnum name) {
        SystemSwitchDto systemSwitch = new SystemSwitchDto();
        systemSwitch.setName(name);
        systemSwitch.setStatus(SwitchStatusEnum.OFF);
        return systemSwitch;
    }

    private ClusterDetailInfo mockClusterDetailInfo() {
        ClusterDetailInfo clusterDetailInfo = new ClusterDetailInfo();
        SourceClustersParams sourceClusters = new SourceClustersParams();
        sourceClusters.setMgrIpList(Collections.singletonList("127.0.0.1"));
        StorageSystemInfo storageSystemInfo = new StorageSystemInfo();
        storageSystemInfo.setPassword("abc123");
        storageSystemInfo.setUsername("******");
        storageSystemInfo.setStoragePort(8080);
        clusterDetailInfo.setStorageSystem(storageSystemInfo);
        clusterDetailInfo.setSourceClusters(sourceClusters);

        List<ClusterDetailInfo> detailInfos = new ArrayList<>();
        ClusterDetailInfo detailInfo1 = new ClusterDetailInfo();

        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setPort(8080);
        targetClusterVo.setEsn(UUID.randomUUID().toString().replace("-", ""));
        targetClusterVo.setUsername("******");
        targetClusterVo.setPassword("abc123");
        targetClusterVo.setMgrIpList(Arrays.asList("8.40.102.34"));
        detailInfo1.setTargetClusterVo(targetClusterVo);
        StorageSystemInfo systemInfo = new StorageSystemInfo();
        systemInfo.setPassword("abc123");
        systemInfo.setUsername("******");
        systemInfo.setStoragePort(8080);
        storageSystemInfo.setStorageEsn("2102353GTD10L9000005");
        detailInfo1.setStorageSystem(storageSystemInfo);
        detailInfos.add(detailInfo1);

        clusterDetailInfo.setAllMemberClustersDetail(detailInfos);
        return clusterDetailInfo;
    }

    private ReplicateContext mockReplicateContext() throws JsonProcessingException {
        RMap context = PowerMockito.mock(RMap.class);
        PowerMockito.when(context.get("job_id")).thenReturn(UUID.randomUUID().toString());

        return ReplicateContext.builder()
            .resourceEntity(mockResourceEntity())
            .policy(mockPolicy())
            .backupObject(mockBackupObject())
            .targetCluster(mockTargetClusterVo())
            .context(context)
            .build();
    }

    private TargetClusterVo mockTargetClusterVo() {
        TargetClusterVo targetClusterVo = new TargetClusterVo();
        targetClusterVo.setClusterId("1");
        targetClusterVo.setPort(8080);
        targetClusterVo.setEsn(UUID.randomUUID().toString().replace("-", ""));
        targetClusterVo.setUsername("******");
        targetClusterVo.setPassword("abc123");
        targetClusterVo.setClusterDetailInfo(mockClusterDetailInfo());
        return targetClusterVo;
    }

    private ResourceEntity mockResourceEntity() {
        ResourceEntity resourceEntity = new ResourceEntity();
        resourceEntity.setUuid(UUID.randomUUID().toString());
        resourceEntity.setSubType(ResourceSubTypeEnum.IMPORT_COPY.getType());
        resourceEntity.setUserId("e18b85f4-a0a0-4524-9ff6-10db6c55ebf0");
        return resourceEntity;
    }

    private ProtectedObject mockProtectedObject() {
        ProtectedObject object = new ProtectedObject();
        object.setSlaId("sla_id");
        return object;
    }

    private BackupObject mockBackupObject() {
        BackupObject backupObject = new BackupObject();
        backupObject.setRequestId(UUID.randomUUID().toString());
        backupObject.setTaskId(UUID.randomUUID().toString());
        backupObject.setProtectedObject(mockProtectedObject());

        return backupObject;
    }

    private PolicyBo mockPolicy() throws JsonProcessingException {
        PolicyBo policyBo = new PolicyBo();
        policyBo.setType(SlaPolicyTypeEnum.BACKUP.getName());
        policyBo.setAction("full");
        ScheduleBo scheduleBo = new ScheduleBo();
        scheduleBo.setStartTime(new Timestamp(System.currentTimeMillis()));
        policyBo.setSchedule(scheduleBo);
        JSONObject jsonObject = new JSONObject();
        jsonObject.set(SlaConstant.QOS_ID, "1");
        jsonObject.set("channel_number", 4);
        jsonObject.set("encryption", true);
        jsonObject.set("link_deduplication", true);
        jsonObject.set("external_system_id", "12");
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node = mapper.readTree(jsonObject.toString());
        policyBo.setExtParameters(node);
        return policyBo;
    }

    @Test
    public void test_replicate_update_sla_when_update_and_retention_is_temprary() throws JsonProcessingException {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.get("resource"))
            .thenReturn(
                "{\"name\":\"test_upgrade\",\"path\":\"192.168.120.113\",\"root_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"parent_name\":null,\"parent_uuid\":\"82548066-bdc5-3ecb-8e3c-dfd152f80213\",\"children_uuids\":null,\"type\":\"Fileset\",\"sub_type\":\"Fileset\",\"uuid\":\"f7bfca983429460fa28210a73cefddfe\",\"created_time\":\"2023-03-16T09:45:33.557000\",\"ext_parameters\":{\"next_backup_change_cause\":null,\"next_backup_type\":null,\"consistent_backup\":false,\"cross_file_system\":false,\"backup_nfs\":false,\"backup_smb\":false,\"sparse_file_detection\":false,\"backup_continue_with_files_backup_failed\":false,\"small_file_aggregation\":true,\"aggregation_file_size\":128,\"aggregation_file_max_size\":128,\"pre_script\":null,\"post_script\":null,\"failed_script\":null},\"authorized_user\":null,\"user_id\":null,\"version\":null,\"sla_id\":\"40d9f4c2-75d4-457c-9f6d-a5a0fe35c5b2\",\"sla_name\":\"复制\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"environment_name\":\"dws-agent-2\",\"environment_endpoint\":\"192.168.120.113\",\"environment_os_type\":\"linux\",\"environment_type\":\"Host\",\"environment_sub_type\":\"UBackupAgent\",\"environment_is_cluster\":\"False\",\"environment_os_name\":\"linux\",\"extendInfo\":{\"templateName\":\"\",\"paths\":\"[{\\\"name\\\":\\\"/l30015744_test\\\"}]\",\"filters\":\"[]\",\"templateId\":\"\"}}");
        PowerMockito.when(map.get("sla"))
            .thenReturn(
                "{\"uuid\":\"c6f771db-e097-42b3-81cb-b6f32f8aa000\",\"name\":\"复制_更新前\",\"created_time\":1679020683849,\"type\":1,\"application\":\"OpenGauss\",\"policy_list\":[{\"uuid\":\"9a6f9cc6-c66f-44a3-9d7d-be161abb83d7\",\"name\":\"全量01\",\"type\":\"backup\",\"action\":\"full\",\"retention\":{\"retention_type\":2,\"retention_duration\":6,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":5,\"interval_unit\":\"h\",\"start_time\":\"2023-03-17T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\"},\"ext_parameters\":{\"deduplication\":true,\"qos_id\":\"\",\"auto_index\":false,\"alarm_after_failure\":true,\"source_deduplication\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5},\"active\":true,\"is_active\":true},{\"uuid\":\"a2fa3c01-febe-47ab-a12e-0e67acffcd88\",\"name\":\"全量02\",\"type\":\"backup\",\"action\":\"full\",\"retention\":{\"retention_type\":2,\"retention_duration\":9,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":4,\"interval_unit\":\"h\",\"start_time\":\"2023-03-17T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\"},\"ext_parameters\":{\"deduplication\":true,\"qos_id\":\"\",\"auto_index\":false,\"alarm_after_failure\":true,\"source_deduplication\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5},\"active\":true,\"is_active\":true},{\"uuid\":\"d9617b80-072b-47ff-af25-3b001f01c91d\",\"name\":\"策略0\",\"type\":\"replication\",\"action\":\"replication\",\"retention\":{\"retention_type\":2,\"retention_duration\":2,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":2,\"interval_unit\":\"m\",\"start_time\":\"2023-03-17T00:30:21\"},\"ext_parameters\":{\"replication_target_type\":1,\"qos_id\":\"\",\"external_system_id\":\"2\",\"link_deduplication\":true,\"link_compression\":true,\"alarm_after_failure\":true,\"start_replicate_time\":\"2023-03-17 00:30:16\"},\"active\":true,\"is_active\":true}],\"is_global\":false}");
        commonMock(map);
        // extracted mock值
        mockSla();

        // mock copies
        List<Copy> copies = getCopies();
        PowerMockito.when(copyRestApi.queryCopiesByResourceId(any())).thenReturn(copies);
        advanceReplicationProvider.replicate(mockReplicateContext());
        Mockito.verify(clusterNativeApi, Mockito.times(1)).getCurrentEsn();
    }

    @Test
    public void test_replicate_update_sla_when_update_and_retention_is_permanent() throws JsonProcessingException {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.get("resource"))
            .thenReturn(
                "{\"name\":\"test_upgrade\",\"path\":\"192.168.120.113\",\"root_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"parent_name\":null,\"parent_uuid\":\"82548066-bdc5-3ecb-8e3c-dfd152f80213\",\"children_uuids\":null,\"type\":\"Fileset\",\"sub_type\":\"Fileset\",\"uuid\":\"f7bfca983429460fa28210a73cefddfe\",\"created_time\":\"2023-03-16T09:45:33.557000\",\"ext_parameters\":{\"next_backup_change_cause\":null,\"next_backup_type\":null,\"consistent_backup\":false,\"cross_file_system\":false,\"backup_nfs\":false,\"backup_smb\":false,\"sparse_file_detection\":false,\"backup_continue_with_files_backup_failed\":false,\"small_file_aggregation\":true,\"aggregation_file_size\":128,\"aggregation_file_max_size\":128,\"pre_script\":null,\"post_script\":null,\"failed_script\":null},\"authorized_user\":null,\"user_id\":null,\"version\":null,\"sla_id\":\"40d9f4c2-75d4-457c-9f6d-a5a0fe35c5b2\",\"sla_name\":\"复制\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"environment_name\":\"dws-agent-2\",\"environment_endpoint\":\"192.168.120.113\",\"environment_os_type\":\"linux\",\"environment_type\":\"Host\",\"environment_sub_type\":\"UBackupAgent\",\"environment_is_cluster\":\"False\",\"environment_os_name\":\"linux\",\"extendInfo\":{\"templateName\":\"\",\"paths\":\"[{\\\"name\\\":\\\"/l30015744_test\\\"}]\",\"filters\":\"[]\",\"templateId\":\"\"}}");
        PowerMockito.when(map.get("sla"))
            .thenReturn(
                "{\"uuid\":\"c6f771db-e097-42b3-81cb-b6f32f8aa000\",\"name\":\"复制_更新前\",\"created_time\":1679020683849,\"type\":1,\"application\":\"OpenGauss\",\"policy_list\":[{\"uuid\":\"9a6f9cc6-c66f-44a3-9d7d-be161abb83d7\",\"name\":\"全量01\",\"type\":\"backup\",\"action\":\"full\",\"retention\":{\"retention_type\":1,\"retention_duration\":6,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":5,\"interval_unit\":\"h\",\"start_time\":\"2023-03-17T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\"},\"ext_parameters\":{\"deduplication\":true,\"qos_id\":\"\",\"auto_index\":false,\"alarm_after_failure\":true,\"source_deduplication\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5},\"active\":true,\"is_active\":true},{\"uuid\":\"a2fa3c01-febe-47ab-a12e-0e67acffcd88\",\"name\":\"全量02\",\"type\":\"backup\",\"action\":\"full\",\"retention\":{\"retention_type\":2,\"retention_duration\":9,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":4,\"interval_unit\":\"h\",\"start_time\":\"2023-03-17T00:00:00\",\"window_start\":\"00:00:00\",\"window_end\":\"00:00:00\"},\"ext_parameters\":{\"deduplication\":true,\"qos_id\":\"\",\"auto_index\":false,\"alarm_after_failure\":true,\"source_deduplication\":false,\"auto_retry\":true,\"auto_retry_times\":3,\"auto_retry_wait_minutes\":5},\"active\":true,\"is_active\":true},{\"uuid\":\"d9617b80-072b-47ff-af25-3b001f01c91d\",\"name\":\"策略0\",\"type\":\"replication\",\"action\":\"replication\",\"retention\":{\"retention_type\":2,\"retention_duration\":2,\"duration_unit\":\"d\"},\"schedule\":{\"trigger\":1,\"interval\":2,\"interval_unit\":\"m\",\"start_time\":\"2023-03-17T00:30:21\"},\"ext_parameters\":{\"replication_target_type\":1,\"qos_id\":\"\",\"external_system_id\":\"2\",\"link_deduplication\":true,\"link_compression\":true,\"alarm_after_failure\":true,\"start_replicate_time\":\"2023-03-17 00:30:16\"},\"active\":true,\"is_active\":true}],\"is_global\":false}");
        commonMock(map);
        // extracted mock值
        mockSla();

        // mock copies
        List<Copy> copies = getCopies();
        PowerMockito.when(copyRestApi.queryCopiesByResourceId(any())).thenReturn(copies);
        advanceReplicationProvider.replicate(mockReplicateContext());
        Mockito.verify(clusterNativeApi, Mockito.times(1)).getCurrentEsn();
    }

    @Test
    public void test_reverse_replicate() throws Exception {
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.get("sla")).thenReturn(null);
        PowerMockito.when(map.put(anyString(), isNull()))
            .thenThrow(new NullPointerException("map value can't be null"));
        commonMock(map);
        // extracted mock值
        mockSla();
        // mock copies
        List<Copy> copies = getCopies();
        PowerMockito.when(copyRestApi.queryCopiesByResourceId(any())).thenReturn(copies);
        advanceReplicationProvider.replicate(mockReplicateContext());
    }

    private void commonMock(RMap map) {
        PowerMockito.when(map.put(ArgumentMatchers.any(), ArgumentMatchers.any())).thenReturn(null);
        PowerMockito.when(clusterInternalApi.queryClusterDetails()).thenReturn(mockClusterDetailInfo());
        PowerMockito.when(map.get("job_id")).thenReturn(UUID.randomUUID().toString());
        QosBo qosBo = new QosBo();
        PowerMockito.when(qosCommonRestApi.queryQos(ArgumentMatchers.anyString())).thenReturn(qosBo);
        PowerMockito.when(systemSwitchInternalService.queryByName(SwitchNameEnum.REPLICATION_LINK_ENCRYPTION))
            .thenReturn(mockDefaultSwitch(SwitchNameEnum.REPLICATION_LINK_ENCRYPTION));
        qosBo.setSpeedLimit(1024);
    }

    private List<Copy> getCopies() {
        Copy copy = new Copy();
        copy.setStorageId("123");
        Copy copy1 = new Copy();
        copy.setStorageId("123");
        List<Copy> copies = new ArrayList<>();
        copies.add(copy);
        copies.add(copy1);
        Copy copy2 = new Copy();
        copy.setUuid("123");
        copies.add(copy2);
        return copies;
    }

    private void mockSla() {
        SlaDto slaDto = new SlaDto();
        List<PolicyDto> policyList = new ArrayList<>();
        PolicyDto policyDto = new PolicyDto();
        JsonNode node = PowerMockito.mock(JsonNode.class);
        TextNode node2 = PowerMockito.mock(TextNode.class);
        policyDto.setExtParameters(node);
        policyList.add(policyDto);
        slaDto.setPolicyList(policyList);
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(slaDto);
        PowerMockito.when(node.get(anyString())).thenReturn(node2);
        PowerMockito.when(node2.asText()).thenReturn("node2");
    }

    @Test
    public void test_obtainRemoteDevice() throws Exception {
        TargetClusterVo targetClusterVo = mockTargetClusterVo();
        targetClusterVo.setDeployType(DeployTypeEnum.X3000.getValue());
        PolicyBo policyBo = mockPolicy();
        DmeRemoteDevice remoteDevice = Whitebox.invokeMethod(advanceReplicationProvider, "obtainRemoteDevice",
            targetClusterVo, policyBo);
        Assert.assertEquals(DeployTypeEnum.X3000.getValue(), remoteDevice.getDeployType());
    }

    @Test
    public void test_generate_intra_device_list() throws Exception {
        ClusterDetailInfo clusterDetailInfo = mockClusterDetailInfo();
        clusterDetailInfo.getAllMemberClustersDetail().get(0).getTargetClusterVo().setEsn("2102353GTD10L9000005");
        PowerMockito.when(clusterQueryService.getPrimaryGroupClustersDetail()).thenReturn(clusterDetailInfo);
        PowerMockito.when(backupStorageApi.getDetail(anyString())).thenReturn(getNasDistributionStorageDetail());
        PolicyBo policyBo = mockPolicy();
        JSONObject jsonObject = new JSONObject();
        jsonObject.put("replication_target_mode", "2");
        jsonObject.put("external_storage_id", "1");
        jsonObject.put("start_replicate_time", "2023-04-01 12:00:00");
        policyBo.setExtParameters(jsonObject.toBean(JsonNode.class));
        List<DmeRemoteDevice> generateIntraDeviceList = Whitebox.invokeMethod(advanceReplicationProvider,
            "generateIntraDeviceList", policyBo.getExtParameters());
        Assert.assertFalse(generateIntraDeviceList.isEmpty());

        jsonObject = new JSONObject();
        jsonObject.put("replication_target_mode", "2");
        jsonObject.put("start_replicate_time", "2023-04-01 12:00:00");
        policyBo.setExtParameters(jsonObject.toBean(JsonNode.class));
        generateIntraDeviceList = Whitebox.invokeMethod(advanceReplicationProvider, "generateIntraDeviceList",
            policyBo.getExtParameters());
        Assert.assertFalse(generateIntraDeviceList.isEmpty());
    }

    public static NasDistributionStorageDetail getNasDistributionStorageDetail() {
        NasDistributionStorageDetail nasDistributionStorageDetail = new NasDistributionStorageDetail();
        nasDistributionStorageDetail.setUnitList(new ArrayList<>());
        BackupUnitVo backupUnitVo = new BackupUnitVo();
        BackupClusterVo clustersInfoVo = new BackupClusterVo();
        clustersInfoVo.setIp("192.168.100.100");
        clustersInfoVo.setPort(9527);
        clustersInfoVo.setGeneratedType(1);
        clustersInfoVo.setClusterId(10000);
        clustersInfoVo.setStorageEsn("2102353GTD10L9000005");
        clustersInfoVo.setStatus(ClusterEnum.StatusEnum.ONLINE.getStatus());
        clustersInfoVo.setPmPort(25081);
        backupUnitVo.setBackupClusterVo(clustersInfoVo);
        nasDistributionStorageDetail.getUnitList().add(backupUnitVo);
        return nasDistributionStorageDetail;
    }

    /**
     * 用例场景：复制接口校验参数不合法，抛出异常
     * 前置条件：复制接口校验参数不合法
     * 检查点：【抛出异常
     */
    @Test
    public void test_replicate_when_param_error_then_throw_exception() {
        DmeResponseError error = new DmeResponseError();
        error.setCode(1073948954L);
        DmeResponse<String> response = new DmeResponse<>();
        response.setError(error);
        PowerMockito.when(dmeReplicationRestApi.replicate(any())).thenReturn(response);
        RMap map = PowerMockito.mock(RMap.class);
        PowerMockito.when(redissonClient.getMap(ArgumentMatchers.any(), ArgumentMatchers.eq(StringCodec.INSTANCE)))
            .thenReturn(map);
        PowerMockito.when(map.get("resource"))
            .thenReturn(
                "{\"name\":\"test_upgrade\",\"path\":\"192.168.120.113\",\"root_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"parent_name\":null,\"parent_uuid\":\"82548066-bdc5-3ecb-8e3c-dfd152f80213\",\"children_uuids\":null,\"type\":\"Fileset\",\"sub_type\":\"Fileset\",\"uuid\":\"f7bfca983429460fa28210a73cefddfe\",\"created_time\":\"2023-03-16T09:45:33.557000\",\"ext_parameters\":{\"next_backup_change_cause\":null,\"next_backup_type\":null,\"consistent_backup\":false,\"cross_file_system\":false,\"backup_nfs\":false,\"backup_smb\":false,\"sparse_file_detection\":false,\"backup_continue_with_files_backup_failed\":false,\"small_file_aggregation\":true,\"aggregation_file_size\":128,\"aggregation_file_max_size\":128,\"pre_script\":null,\"post_script\":null,\"failed_script\":null},\"authorized_user\":null,\"user_id\":null,\"version\":null,\"sla_id\":\"40d9f4c2-75d4-457c-9f6d-a5a0fe35c5b2\",\"sla_name\":\"复制\",\"sla_status\":true,\"sla_compliance\":null,\"protection_status\":1,\"environment_uuid\":\"662ee599-dedc-47e5-95ec-86380ae624ca\",\"environment_name\":\"dws-agent-2\",\"environment_endpoint\":\"192.168.120.113\",\"environment_os_type\":\"linux\",\"environment_type\":\"Host\",\"environment_sub_type\":\"UBackupAgent\",\"environment_is_cluster\":\"False\",\"environment_os_name\":\"linux\",\"extendInfo\":{\"templateName\":\"\",\"paths\":\"[{\\\"name\\\":\\\"/l30015744_test\\\"}]\",\"filters\":\"[]\",\"templateId\":\"\"}}");
        commonMock(map);
        // extracted mock值
        SlaDto slaDto = new SlaDto();
        List<PolicyDto> policyList = new ArrayList<>();
        PolicyDto policyDto = new PolicyDto();
        JsonNode node = PowerMockito.mock(JsonNode.class);
        TextNode node2 = PowerMockito.mock(TextNode.class);
        policyDto.setExtParameters(node);
        policyList.add(policyDto);
        slaDto.setPolicyList(policyList);
        PowerMockito.when(slaQueryService.querySlaById(anyString())).thenReturn(slaDto);
        PowerMockito.when(node.get(anyString())).thenReturn(node2);
        PowerMockito.when(node2.asText()).thenReturn("node2");
        // mock copies
        List<Copy> copies = getCopies();
        PowerMockito.when(copyRestApi.queryCopiesByResourceId(any())).thenReturn(copies);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> advanceReplicationProvider.replicate(mockReplicateContext()));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.ERR_PARAM);
    }

    @Test
    public void test_execute_replica() {
        DmeReplicateRequest request = new DmeReplicateRequest();
        DmeResponse<String> result = new DmeResponse<>();
        DmeResponseError error = new DmeResponseError();
        error.setDescription("test");
        error.setCode(CommonErrorCode.BACKUP_CLUSTER_NOT_ALL_ONLINE);
        result.setError(error);
        PowerMockito.when(dmeReplicationRestApi.replicate(any())).thenReturn(result);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
            () -> Whitebox.invokeMethod(advanceReplicationProvider, "executeReplica", request));
        Assert.assertEquals(exception.getErrorCode(), CommonErrorCode.BACKUP_CLUSTER_NOT_ALL_ONLINE);
    }
}
