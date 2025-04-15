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
package openbackup.ndmp.protection.access.sla;

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.assertThatNoException;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import com.fasterxml.jackson.databind.JsonNode;

import openbackup.ndmp.protection.access.sla.NdmpVailderSlaProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.repository.api.BackupStorageApi;
import openbackup.system.base.sdk.repository.model.NasDistributionStorageDetail;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * sla 测试类
 *
 */
public class NdmpVailderSlaTest {
    SlaBase slaBase = new SlaBase();

    JSONObject jsonObject = new JSONObject();

    private final BackupStorageApi backupStorageApi = Mockito.mock(BackupStorageApi.class);

    private final NdmpVailderSlaProvider validator = new NdmpVailderSlaProvider(backupStorageApi);

    /**
     * 用例名称：获取provider成功<br/>
     * 前置条件：无<br/>
     * check点：能够获取到通用sla配置<br/>
     */
    @Test
    public void applicable() {
        Assert.assertTrue(validator.applicable(ResourceSubTypeEnum.NDMP_BACKUPSET.getType()));
    }

    /**
     * 用例名称：获取provider成功<br/>
     * 前置条件：无<br/>
     * check点：能够获取到通用sla配置<br/>
     */
    @Test
    public void test_get_config() {
        SlaValidateConfig config = validator.getConfig();
        List<PolicyLimitConfig> policyLimitConfigs = Arrays.asList(
            PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT),
            PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT)

        );
        assertThat(validator.applicable(ResourceSubTypeEnum.NDMP_BACKUPSET.getType())).isTrue();
        assertThat(config).isNotNull();
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).hasSize(5);
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).usingRecursiveComparison()
            .isEqualTo(policyLimitConfigs);
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