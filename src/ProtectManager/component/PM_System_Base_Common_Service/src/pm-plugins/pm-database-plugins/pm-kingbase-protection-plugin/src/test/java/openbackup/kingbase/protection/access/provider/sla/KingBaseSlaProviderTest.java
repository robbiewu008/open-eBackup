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
package openbackup.kingbase.protection.access.provider.sla;

import static org.assertj.core.api.Assertions.assertThat;

import com.huawei.oceanprotect.sla.sdk.constants.SlaConstants;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.kingbase.protection.access.provider.sla.KingBaseSlaProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * {@link KingBaseSlaProvider} 测试类
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-19
 */
public class KingBaseSlaProviderTest {
    private final KingBaseSlaProvider kingBaseSlaProvider = new KingBaseSlaProvider();

    /**
     * 用例名称：获取KingBase的sla配置信息
     * 前置条件：无
     * check点：能够获取到KingBase的sla配置
     */
    @Test
    public void should_return_one_full_and_one_log_when_create_postgres_sla() {
        SlaValidateConfig config = kingBaseSlaProvider.getConfig();
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

    /**
     * 用例场景：KingBase的sla适配器
     * 前置条件：输入子资源类型
     * 检查点：是否返回true
     */
    @Test
    public void applicable_postgre_cluster_provider_success() {
        Assert.assertTrue(kingBaseSlaProvider.applicable(ResourceSubTypeEnum.KING_BASE.getType()));
        Assert.assertFalse(kingBaseSlaProvider.applicable(ResourceSubTypeEnum.MYSQL.getType()));
    }
}