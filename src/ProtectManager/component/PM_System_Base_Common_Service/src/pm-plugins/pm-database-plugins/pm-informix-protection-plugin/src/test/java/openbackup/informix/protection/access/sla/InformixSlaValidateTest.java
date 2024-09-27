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
package openbackup.informix.protection.access.sla;

import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.PolicyLimitConfig;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.informix.protection.access.sla.InformixSlaValidate;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

import java.util.Arrays;
import java.util.List;
import java.util.stream.Collectors;

/**
 * sla应用校验测试类
 *
 * @author zWX951267
 * @version [DataBackup 1.5.0]
 * @since 2023-05-17
 */
public class InformixSlaValidateTest {
    private final InformixSlaValidate informixSlaValidate = new InformixSlaValidate();

    /**
     * 用例场景：dameng环境检查类过滤
     * 前置条件：无
     * 检查点：过滤成功
     */
    @Test
    public void applicable_success() {
        Assert.assertTrue(informixSlaValidate.applicable(ResourceSubTypeEnum.INFORMIX_SERVICE.getType()));
        Assert.assertFalse(informixSlaValidate.applicable(ResourceSubTypeEnum.MYSQL_DATABASE.getType()));
    }

    /**
     * 用例场景：informix获取sla校验器配置
     * 前置条件：无
     * 检查点：获取到sla的自定义配置
     */
    @Test
    public void get_config_success() {
        SlaValidateConfig slaValidateConfig = informixSlaValidate.getConfig();
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
}