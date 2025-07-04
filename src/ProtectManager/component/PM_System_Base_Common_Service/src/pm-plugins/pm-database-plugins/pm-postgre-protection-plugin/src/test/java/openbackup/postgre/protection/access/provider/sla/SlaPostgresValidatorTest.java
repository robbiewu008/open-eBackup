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
package openbackup.postgre.protection.access.provider.sla;

import static org.assertj.core.api.Assertions.assertThat;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.postgre.protection.access.provider.sla.SlaPostgresValidator;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;
import org.springframework.test.context.junit4.SpringRunner;

/**
 * {@link SlaPostgresValidator} 测试类
 *
 */
@RunWith(SpringRunner.class)
@SpringBootTest(classes = {SlaPostgresValidator.class})
public class SlaPostgresValidatorTest {
    @Autowired
    private SlaPostgresValidator slaPostgresValidator;

    /**
     * 用例名称：获取provider成功<br/>
     * 前置条件：无<br/>
     * check点：能够获取到Postgres sla配置<br/>
     */
    @Test
    public void should_return_one_full_and_one_log_when_create_postgres_sla() throws Exception {
        SlaValidateConfig config = slaPostgresValidator.getConfig();
        assertThat(config).isNotNull();
        assertThat(config.getSpecificationConfig().getPoliciesConfig()).hasSize(4);
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
                PolicyAction.LOG, SlaConstants.LOG_BACKUP_POLICY_COUNT_DEFAULT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(2))
            .usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(
                PolicyAction.ARCHIVING, SlaConstants.ARCHIVE_POLICY_COUNT_LIMIT));
        assertThat(config.getSpecificationConfig().getPoliciesConfig().get(3))
            .usingRecursiveComparison()
            .isEqualTo(PolicyLimitConfig.of(
                PolicyAction.REPLICATION, SlaConstants.REPLICATION_POLICY_COUNT_LIMIT));
        assertThat(config.getBackupConfig().shouldCheckBackupPoliciesExtParamsIdentical());
    }
}