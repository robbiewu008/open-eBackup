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
package openbackup.saphana.protection.access.provider.sla;

import static org.assertj.core.api.Assertions.assertThat;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.saphana.protection.access.provider.sla.SapHanaSlaValidatorProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * {@link SapHanaSlaValidatorProvider Test}
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-06-09
 */
public class SapHanaSlaValidatorProviderTest {
    private final SapHanaSlaValidatorProvider hanaSlaProvider = new SapHanaSlaValidatorProvider();

    /**
     * 用例名称：获取SAPHANA的sla配置信息
     * 前置条件：无
     * check点：获取到SAPHANA的sla配置正确
     */
    @Test
    public void should_return_backup_archive_replication_cfg_when_create_db2_sla() {
        SlaValidateConfig slaConfig = hanaSlaProvider.getConfig();
        assertThat(slaConfig).isNotNull();
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig()).hasSize(6);
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig()).first()
            .usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig().get(1)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig().get(2)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig().get(3)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig().get(4)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        assertThat(slaConfig.getSpecificationConfig().getPoliciesConfig().get(5)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        assertThat(slaConfig.getBackupConfig().shouldCheckBackupPoliciesExtParamsIdentical());
    }

    /**
     * 用例场景：SAPHANA的SAL适配器
     * 前置条件：输入子资源类型
     * 检查点：SAPHANA-database返回true
     */
    @Test
    public void applicable_saphana_database_provider_success() {
        Assert.assertTrue(hanaSlaProvider.applicable(ResourceSubTypeEnum.SAPHANA_DATABASE.getType()));
    }

    /**
     * 用例场景：SAPHANA的SAL适配器
     * 前置条件：输入子资源类型
     * 检查点：SAPHANA-instance返回false
     */
    @Test
    public void applicable_saphana_instance_provider_fail() {
        Assert.assertFalse(hanaSlaProvider.applicable(ResourceSubTypeEnum.SAPHANA_INSTANCE.getType()));
    }
}
