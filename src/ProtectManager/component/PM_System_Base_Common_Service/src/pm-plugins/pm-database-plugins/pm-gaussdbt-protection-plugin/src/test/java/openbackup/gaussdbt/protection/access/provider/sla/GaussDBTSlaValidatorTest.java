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
package openbackup.gaussdbt.protection.access.provider.sla;

import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.gaussdbt.protection.access.provider.sla.GaussDBTSlaValidator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * GaussDBTSlaValidatorTest
 *
 */
public class GaussDBTSlaValidatorTest {
    private SlaValidService slaValidService = PowerMockito.mock(SlaValidService.class);

    private final GaussDBTSlaValidator gaussDBTSlaValidator = new GaussDBTSlaValidator(slaValidService);

    /**
     * 用例场景：策略模式策略识别-GaussDBT
     * 前置条件：类型参数为GaussDBT
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(gaussDBTSlaValidator.applicable(ResourceSubTypeEnum.GAUSSDBT.getType()));
        Assert.assertFalse(gaussDBTSlaValidator.applicable(ResourceSubTypeEnum.DAMENG.getType()));
    }

    /**
     * 用例场景：GaussDBT类型sla创建配置备份类型
     * 前置条件：GaussDBT类型识别成功，服务正常
     * 检查点: 添加全量备份、差异增量备份、累积增量备份、日志备份、复制、归档成功
     */
    @Test
    public void add_sla_config_limit_success() {
        SlaValidateConfig slaValidateConfig = gaussDBTSlaValidator.getConfig();
        SlaValidateConfig.SpecificationConfig config = slaValidateConfig.getSpecificationConfig();
        List<PolicyLimitConfig> policiesConfig = config.getPoliciesConfig();
        Assert.assertEquals(6, policiesConfig.size());
        List<PolicyAction> actions = policiesConfig.stream()
            .map(PolicyLimitConfig::getAction)
            .collect(Collectors.toList());
        Assert.assertTrue(actions.containsAll(
            Arrays.asList(PolicyAction.FULL, PolicyAction.DIFFERENCE_INCREMENT, PolicyAction.CUMULATIVE_INCREMENT,
                PolicyAction.LOG, PolicyAction.ARCHIVING, PolicyAction.REPLICATION)));
    }

    /**
     * 用例场景：GaussDBT单机sla校验
     * 前置条件：GaussDBT类型识别成功，服务正常
     * 检查点: 检查sla策略类型
     */
    @Test
    public void execute_gaussdbt_single_sla_validate_success() {
        SlaBase slaBase = new SlaBase();
        gaussDBTSlaValidator.validateSLA(slaBase);
    }
}
