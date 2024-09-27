/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.gaussdb.protection.access.sla;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatNoException;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.gaussdb.protection.access.sla.GaussDBValidatorProvider;
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

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * {@link GaussDBValidatorProvider}测试类
 *
 * @author t30021437
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-02-15
 */

public class GaussDBValidatorProviderTest {
    private final BackupStorageApi backupStorageApi = Mockito.mock(BackupStorageApi.class);

    private final GaussDBValidatorProvider validator = new GaussDBValidatorProvider(backupStorageApi);

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
        List<PolicyLimitConfig> policyLimitConfigs = Arrays.asList(
            PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT)

        );
        assertThat(validator.applicable(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType())).isTrue();
        assertThat(config).isNotNull();
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).hasSize(5);
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).usingRecursiveComparison()
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
        slaBase.setPolicyList(Collections.singletonList(policyDto));
        assertThatNoException().isThrownBy(() -> validator.validateSLA(slaBase));
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
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> validator.validateSLA(slaBase));
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
        LegoCheckedException legoCheckedException = Assert.assertThrows(LegoCheckedException.class,
            () -> validator.validateSLA(slaBase));
        assertThat(legoCheckedException.getErrorCode()).isEqualTo(CommonErrorCode.ILLEGAL_PARAM);
    }
}
