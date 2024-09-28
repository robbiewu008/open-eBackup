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
package openbackup.clickhouse.plugin.sla;

import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.clickhouse.plugin.sla.ClickHouseSlaValidator;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * ClickHouse SLA校验测试类
 *
 */
public class ClickHouseSlaValidatorTest{
    private final ClickHouseSlaValidator clickHouseSlaValidator = new ClickHouseSlaValidator();

    /**
     * 用例场景：策略模式策略识别-ClickHouse
     * 前置条件：类型参数为 ClickHouse
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(clickHouseSlaValidator.applicable(ResourceSubTypeEnum.CLICK_HOUSE.getType()));
        Assert.assertFalse(clickHouseSlaValidator.applicable(ResourceSubTypeEnum.REDIS.getType()));
    }

    /**
     * 用例场景：ClickHouse类型sla创建配置备份类型
     * 前置条件：ClickHouse类型识别成功，服务正常
     * 检查点: 添加全量备份、复制、归档成功
     */
    @Test
    public void add_sla_config_limit_success() {
        SlaValidateConfig slaValidateConfig = clickHouseSlaValidator.getConfig();
        SlaValidateConfig.SpecificationConfig config = slaValidateConfig.getSpecificationConfig();
        List<PolicyLimitConfig> policiesConfig = config.getPoliciesConfig();
        Assert.assertEquals(3, policiesConfig.size());
        List<PolicyAction> actions = policiesConfig.stream()
                .map(PolicyLimitConfig::getAction)
                .collect(Collectors.toList());
        Assert.assertTrue(actions.containsAll(Arrays.asList(PolicyAction.FULL, PolicyAction.ARCHIVING,
                PolicyAction.REPLICATION)));
    }
}
