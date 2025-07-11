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
package openbackup.dameng.protection.access.sla;

import openbackup.database.base.plugin.service.impl.SlaValidService;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;
import org.powermock.api.mockito.PowerMockito;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * sla应用校验测试类
 *
 */
public class DamengSlaValidateTest {
    private final SlaValidService slaValidService = PowerMockito.mock(SlaValidService.class);

    private final DamengSlaValidate damengSlaValidate = new DamengSlaValidate(slaValidService);

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(damengSlaValidate.applicable(ResourceSubTypeEnum.DAMENG.getType()));
        Assert.assertFalse(damengSlaValidate.applicable(ResourceSubTypeEnum.MYSQL_DATABASE.getType()));
    }

    /**
     * 用例场景：dameng获取sla校验器配置
     * 前置条件：无
     * 检查点：获取到sla的自定义配置
     */
    @Test
    public void get_config_success() {
        SlaValidateConfig slaValidateConfig = damengSlaValidate.getConfig();
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
     * 用例名称：dameng修改sla
     * 前置条件：无
     * 检查点：不报错
     */
    @Test
    public void validate_success() {
        SlaValidService slaValidService = PowerMockito.mock(SlaValidService.class);
        SlaBase slaBase = new SlaBase();
        damengSlaValidate.validateSLA(slaBase);
        Assert.assertTrue(true);
    }
}
