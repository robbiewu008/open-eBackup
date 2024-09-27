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
package openbackup.db2.protection.access.provider.sla;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.db2.protection.access.provider.sla.Db2SlaValidatorProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

/**
 * {@link Db2SlaValidatorProvider} 测试类
 *
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-01-05
 */
public class Db2SlaValidatorProviderTest {
    private final SlaValidService slaValidService = Mockito.mock(SlaValidService.class);

    private final Db2SlaValidatorProvider db2SlaValidatorProvider = new Db2SlaValidatorProvider(slaValidService);

    /**
     * 用例名称：获取db2的sla配置信息
     * 前置条件：无
     * check点：能够获取到db2的sla配置
     */
    @Test
    public void should_return_backup_archive_replication_cfg_when_create_db2_sla() {
        SlaValidateConfig config = db2SlaValidatorProvider.getConfig();
        assertThat(config).isNotNull();
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).hasSize(6);
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).first()
            .usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.FULL, SlaConstants.FULL_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(1)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.DIFFERENCE_INCREMENT,
                SlaConstants.DIFFERENCE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(2)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.CUMULATIVE_INCREMENT,
                SlaConstants.CUMULATIVE_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(3)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(4)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(5)).usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        assertThat(config.getBackupConfig().shouldCheckBackupPoliciesExtParamsIdentical());
    }

    /**
     * 用例场景：db2的sla适配器
     * 前置条件：输入子资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_postgre_cluster_provider_success() {
        Assert.assertTrue(db2SlaValidatorProvider.applicable(ResourceSubTypeEnum.DB2.getType()));
        Assert.assertFalse(db2SlaValidatorProvider.applicable(ResourceSubTypeEnum.MYSQL.getType()));
    }


    /**
     * 用例名称：db2表空间资源修改sla
     * 前置条件：无
     * 检查点：不报错
     */
    @Test
    public void execute_validate_success() {
        SlaBase slaBase = new SlaBase();
        db2SlaValidatorProvider.validateSLA(slaBase);
        Assert.assertTrue(true);
    }
}