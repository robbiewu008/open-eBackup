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
package openbackup.oracle.sla;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.UpdateSlaCommand;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyType;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.List;

import static org.assertj.core.api.Assertions.assertThat;

/**
 * 功能描述
 * {@link OracleSlaValidatorProvider}测试类
 *
 */
public class OracleSlaValidatorProviderTest {
    private final ProtectObjectRestApi protectObjectRestApi = Mockito.mock(ProtectObjectRestApi.class);

    private final OracleSlaValidatorProvider oracleSlaValidatorProvider = new OracleSlaValidatorProvider(
            protectObjectRestApi);

    /**
     * 用例场景：策略模式策略识别-Oracle
     * 前置条件：类型参数为Oracle
     * 检查点：识别成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(oracleSlaValidatorProvider.applicable(ResourceSubTypeEnum.ORACLE.getType()));
    }

    @Test
    public void should_return_correct_config_when_specify_oracle_config() throws Exception {
        SlaValidateConfig config = oracleSlaValidatorProvider.getConfig();
        assertThat(config).isNotNull();
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).hasSize(7);
        assertThat(config.getSpecificationConfig().getPoliciesConfig())
                .first()
                .usingRecursiveComparison()
                .isEqualTo(
                        PolicyLimitConfig.of(
                                PolicyAction.FULL,
                                SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(1))
                .usingRecursiveComparison()
                .isEqualTo(PolicyLimitConfig.of(
                        PolicyAction.DIFFERENCE_INCREMENT, SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(2))
                .usingRecursiveComparison()
                .isEqualTo(PolicyLimitConfig.of(
                        PolicyAction.CUMULATIVE_INCREMENT, SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(3))
                .usingRecursiveComparison()
                .isEqualTo(PolicyLimitConfig.of(
                        PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(4))
                .usingRecursiveComparison()
                .isEqualTo(PolicyLimitConfig.of(
                        PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(5))
                .usingRecursiveComparison()
                .isEqualTo(PolicyLimitConfig.of(
                        PolicyAction.REPLICATION_LOG, SlaConstants.REPLICATION_LOG_POLICY_COUNT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(6))
                .usingRecursiveComparison()
                .isEqualTo(PolicyLimitConfig.of(
                        PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
    }

    /**
     * 用例名称：sla所绑定的资源部支持sla修改的备份策略<br/>
     * 前置条件：<br/>
     * check点：不合法的参数成功校验<br/>
     */
    @Test
    public void should_throw_LegoCheckedException_when_resource_not_support_sla() {
        PowerMockito.when(
                        protectObjectRestApi.pageQueryProtectObject(Mockito.anyString(), Mockito.anyInt(), Mockito.anyInt()))
                .thenReturn(prepareBasePages());
        Assert.assertThrows(LegoCheckedException.class,
                () -> oracleSlaValidatorProvider.validateSLA(prepareUpdateSlaCommand()));
    }

    private BasePage<ProtectedObjectInfo> prepareBasePages() {
        ProtectedObjectInfo protectedObjectInfo1 = new ProtectedObjectInfo();
        protectedObjectInfo1.setSubType(ResourceSubTypeEnum.ORACLE_PDB.getType());
        protectedObjectInfo1.setResourceId("123");
        ProtectedObjectInfo protectedObjectInfo2 = new ProtectedObjectInfo();
        protectedObjectInfo2.setResourceId("456");
        List<ProtectedObjectInfo> items = new ArrayList<>();
        items.add(protectedObjectInfo1);
        items.add(protectedObjectInfo2);
        BasePage<ProtectedObjectInfo> basePages = new BasePage<>();
        basePages.setItems(items);
        return basePages;
    }

    private UpdateSlaCommand prepareUpdateSlaCommand() {
        UpdateSlaCommand updateSlaCommand = new UpdateSlaCommand();
        updateSlaCommand.setPolicyList(preparePolicyList());
        updateSlaCommand.setUuid("123456");
        return updateSlaCommand;
    }

    private List<PolicyDto> preparePolicyList() {
        PolicyDto policyDto1 = new PolicyDto();
        policyDto1.setType(PolicyType.REPLICATION);
        policyDto1.setAction(PolicyAction.REPLICATION);
        policyDto1.setSlaId("777");
        PolicyDto policyDto2 = new PolicyDto();
        policyDto2.setType(PolicyType.ARCHIVING);
        policyDto2.setAction(PolicyAction.ARCHIVING);
        policyDto2.setSlaId("888");
        PolicyDto policyDto3 = new PolicyDto();
        policyDto3.setType(PolicyType.REPLICATION);
        policyDto3.setAction(PolicyAction.REPLICATION_LOG);
        policyDto3.setSlaId("999");
        List<PolicyDto> policyDtoList = new ArrayList<>();
        policyDtoList.add(policyDto1);
        policyDtoList.add(policyDto2);
        policyDtoList.add(policyDto3);
        return policyDtoList;
    }
}
