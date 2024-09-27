/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdbdws.protection.access.sla;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatNoException;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;

import com.huawei.oceanprotect.base.cluster.sdk.service.ArrayTargetClusterService;
import com.huawei.oceanprotect.base.cluster.repository.ClusterRepository;
import com.huawei.oceanprotect.base.cluster.sdk.entity.TargetCluster;
import openbackup.data.protection.access.provider.sdk.protection.ProtectionQueryService;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.fasterxml.jackson.databind.JsonNode;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * {@link DwsValidator}测试类
 *
 * @author mwx1009754
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-30
 */

public class DwsValidatorTest {
    private final BackupStorageApi backupStorageApi = Mockito.mock(BackupStorageApi.class);

    private final ProtectionQueryService protectionQueryService = Mockito.mock(ProtectionQueryService.class);

    private final ArrayTargetClusterService arrayTargetClusterService = Mockito.mock(ArrayTargetClusterService.class);

    private final ClusterRepository clusterRepository = Mockito.mock(ClusterRepository.class);

    private final DwsValidator validator = new DwsValidator(backupStorageApi, protectionQueryService,
        arrayTargetClusterService, clusterRepository);

    SlaBase slaBase = new SlaBase();

    JSONObject jsonObject = new JSONObject();

    /**
     * 用例名称：获取provider成功<br/>
     * 前置条件：无<br/>
     * check点：能够获取到通用sla配置<br/>
     */
    @Test
    public void should_return_correct_config_when_specify_oracle_config() {
        SlaValidateConfig config = validator.getConfig();
        List<PolicyLimitConfig> policyLimitConfigs = Arrays.asList(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                        SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                        SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT),
                PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT)

        );
        assertThat(validator.applicable(ResourceSubTypeEnum.GAUSSDB_DWS.getType())).isTrue();
        assertThat(config).isNotNull();
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).hasSize(5);
        assertThat(config.getSpecificationConfig().getPoliciesConfig())
                .usingRecursiveComparison()
                .isEqualTo(policyLimitConfigs);
    }

    /**
     * 用例名称：创建sla时，存储库校验不传，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_storage_id_not_exists() {
        PolicyDto policyDto = new PolicyDto();
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.REPLICATION);
        PolicyDto backupPolicy = new PolicyDto();
        backupPolicy.setExtParameters(jsonObject.toBean(JsonNode.class));
        backupPolicy.setType(PolicyType.BACKUP);
        List<PolicyDto> policyList = new ArrayList<>();
        policyList.add(policyDto);
        policyList.add(backupPolicy);
        slaBase.setPolicyList(policyList);
        assertThatNoException().isThrownBy(() -> validator.validateSLA(slaBase));
    }

    /**
     * 用例名称：创建sla时，存储库校验不传，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void check_application_is_support_action_success() {
        PolicyDto policyDto = new PolicyDto();
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.BACKUP);
        policyDto.setAction(PolicyAction.CUMULATIVE_INCREMENT);
        slaBase.setPolicyList(Collections.singletonList(policyDto));
        validator.validateSLA(slaBase);
    }

    /**
     * 用例名称：创建sla时，存储库校验不传，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_application_is_support_action_if_check_schema() {
        PolicyDto policyDto = new PolicyDto();
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.BACKUP);
        policyDto.setAction(PolicyAction.CUMULATIVE_INCREMENT);
        slaBase.setPolicyList(Collections.singletonList(policyDto));
        slaBase.setName("111");
        slaBase.setUserId("222");
        PowerMockito.when(protectionQueryService.countBySubTypeAndSlaName("111", null,
            ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType())).thenReturn(2);
        Assert.assertThrows(LegoCheckedException.class, () -> validator.validateSLA(slaBase));
    }

    /**
     * 用例名称：创建sla时，存储库校验不传，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_check_application_is_support_action_if_check_table() {
        PolicyDto policyDto = new PolicyDto();
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.REPLICATION);
        policyDto.setAction(PolicyAction.CUMULATIVE_INCREMENT);
        slaBase.setPolicyList(Collections.singletonList(policyDto));
        slaBase.setName("111");
        slaBase.setUserId("222");
        PowerMockito.when(protectionQueryService.countBySubTypeAndSlaName("111", null,
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType())).thenReturn(2);
        Assert.assertThrows(LegoCheckedException.class, () -> validator.validateSLA(slaBase));
    }

    /**
     * 用例名称：创建sla时，存储库校验不存在，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_storage_info() {
        PolicyDto policyDto = new PolicyDto();
        jsonObject.set("storage_id", "1");
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.BACKUP);
        slaBase.setPolicyList(Collections.singletonList(policyDto));
        NasDistributionStorageDetail nasDistributionStorageDetail = new NasDistributionStorageDetail();
        nasDistributionStorageDetail.setUuid("1");
        Mockito.when(backupStorageApi.getDetail("1")).thenReturn(nasDistributionStorageDetail);
        assertThatNoException().isThrownBy(() -> validator.validateSLA(slaBase));
    }

    /**
     * 用例名称：创建sla时，存储库校验不存在，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_storage_info_not_exists() {
        PolicyDto policyDto = new PolicyDto();
        jsonObject.set("storage_id", "1");
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.BACKUP);
        slaBase.setPolicyList(Collections.singletonList(policyDto));

        Mockito.when(backupStorageApi.getDetail("1")).thenReturn(new NasDistributionStorageDetail());
        LegoCheckedException legoCheckedException =
                Assert.assertThrows(LegoCheckedException.class, () -> validator.validateSLA(slaBase));
        assertThat(legoCheckedException.getErrorCode()).isEqualTo(CommonErrorCode.ILLEGAL_PARAM);
    }

    /**
     * 用例名称：创建sla时，存储库校验不存在，则抛出异常<br/>
     * 前置条件：无<br/>
     * check点：存储库校验不存在<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_storage_info_exception() {
        PolicyDto policyDto = new PolicyDto();
        jsonObject.set("storage_id", "1");
        policyDto.setExtParameters(jsonObject.toBean(JsonNode.class));
        policyDto.setType(PolicyType.BACKUP);
        slaBase.setPolicyList(Collections.singletonList(policyDto));

        Mockito.when(backupStorageApi.getDetail("1")).thenThrow(LegoCheckedException.class);
        LegoCheckedException legoCheckedException =
                Assert.assertThrows(LegoCheckedException.class, () -> validator.validateSLA(slaBase));
        assertThat(legoCheckedException.getErrorCode()).isEqualTo(CommonErrorCode.ILLEGAL_PARAM);
    }

    /**
     * 用例名称：创建sla时，备份策略未配置备份存储单元组，复制策略配置备份存储单元组
     * 前置条件：备份策略未配置备份存储单元组，复制策略配置备份存储单元组
     * check点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_backup_not_has_storage_group() {
        PolicyDto policyDto = new PolicyDto();
        // 复制策略配置备份存储单元组
        JSONObject replicationExt = new JSONObject();
        replicationExt.put("external_storage_id", "33998cec-808b-496a-935a-522b8f66511f");
        JsonNode replicationExtParam = replicationExt.toBean(JsonNode.class);
        policyDto.setExtParameters(replicationExtParam);
        policyDto.setType(PolicyType.REPLICATION);
        PolicyDto backupPolicy = new PolicyDto();
        // 备份策略未配置备份存储单元组
        JSONObject backupExt = new JSONObject();
        JSONObject storageInfo = new JSONObject();
        storageInfo.put("storage_type", "storage_unit_group");
        backupExt.put("storage_info", storageInfo.toBean(JsonNode.class));
        JsonNode backupExtParam = backupExt.toBean(JsonNode.class);
        backupPolicy.setExtParameters(backupExtParam);
        backupPolicy.setType(PolicyType.BACKUP);
        List<PolicyDto> policyList = new ArrayList<>();
        policyList.add(policyDto);
        policyList.add(backupPolicy);
        slaBase.setPolicyList(policyList);
        Assert.assertThrows(LegoCheckedException.class, () -> validator.validateSLA(slaBase));
    }

    /**
     * 用例名称：创建sla时，备份策略配置并行备份存储单元组，复制策略配置非并行备份存储单元组
     * 前置条件：备份策略配置并行备份存储单元组，复制策略配置非并行备份存储单元组
     * check点：抛出异常
     */
    @Test
    public void should_throw_LegoCheckedException_when_backup_has_parallel_storage_group() {
        PolicyDto policyDto = new PolicyDto();
        // 复制策略配置备份存储单元组
        JSONObject replicationExt = new JSONObject();
        replicationExt.put("external_storage_id", "33998cec-808b-496a-935a-522b8f66511f");
        replicationExt.put("external_system_id", "4");
        JsonNode replicationExtParam = replicationExt.toBean(JsonNode.class);
        policyDto.setExtParameters(replicationExtParam);
        policyDto.setType(PolicyType.REPLICATION);
        PolicyDto backupPolicy = new PolicyDto();
        // 备份策略未配置备份存储单元组
        JSONObject backupExt = new JSONObject();
        JSONObject storageInfo = new JSONObject();
        storageInfo.put("storage_type", "storage_unit_group");
        storageInfo.put("storage_id", "97221370-b106-4a07-81b9-db080a63b096");
        backupExt.put("storage_info", storageInfo.toBean(JsonNode.class));
        JsonNode backupExtParam = backupExt.toBean(JsonNode.class);
        backupPolicy.setExtParameters(backupExtParam);
        backupPolicy.setType(PolicyType.BACKUP);
        List<PolicyDto> policyList = new ArrayList<>();
        policyList.add(policyDto);
        policyList.add(backupPolicy);
        slaBase.setPolicyList(policyList);
        NasDistributionStorageDetail detail = new NasDistributionStorageDetail();
        detail.setHasEnableParallelStorage(true);
        PowerMockito.when(backupStorageApi.getDetail(eq("97221370-b106-4a07-81b9-db080a63b096"))).thenReturn(detail);
        TargetCluster targetCluster = new TargetCluster();
        PowerMockito.when(clusterRepository.getTargetClusterById(anyInt())).thenReturn(targetCluster);
        NasDistributionStorageDetail targetDistribution = new NasDistributionStorageDetail();
        targetDistribution.setHasEnableParallelStorage(false);
        PowerMockito.when(arrayTargetClusterService.getTargetDistribution(any(), any()))
            .thenReturn(Optional.of(targetDistribution));
        Assert.assertThrows(LegoCheckedException.class, () -> validator.validateSLA(slaBase));
    }
}
