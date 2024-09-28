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
package openbackup.goldendb.protection.access.provider;

import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaBase;
import com.huawei.oceanprotect.sla.sdk.enums.PolicyAction;
import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.goldendb.protection.access.provider.GoldenDBSlaValidator;
import openbackup.system.base.common.exception.LegoCheckedException;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;

import java.util.Collections;
import java.util.List;

/**
 * 功能描述
 *
 */
public class GoldenDBSlaValidatorTest {
    private GoldenDBSlaValidator goldenDBTSlaValidator;

    @Before
    public void setUp() throws Exception {
        goldenDBTSlaValidator = new GoldenDBSlaValidator();
    }

    /**
     * 用例场景：获取校验config成功
     * 前置条件：无
     * 检查点: 未抛出异常
     */
    @Test
    public void test_getConfig_success() {
        SlaValidateConfig config = goldenDBTSlaValidator.getConfig();
        Assert.assertNotNull(config);
    }

    /**
     * 用例场景：创建sla检查成功
     * 前置条件：无
     * 检查点: 未抛出异常
     */
    @Test
    public void test_validate_success() {
        SlaBase slaBase = new SlaBase();
        PolicyDto policyDto = new PolicyDto();
        policyDto.setAction(PolicyAction.LOG);
        List<PolicyDto> policyDtos = Collections.singletonList(policyDto);
        slaBase.setPolicyList(policyDtos);
        goldenDBTSlaValidator.validateSLA(slaBase);
    }

    /**
     * 用例场景：创建sla有不支持的备份类型
     * 前置条件：无
     * 检查点: 抛出异常
     */
    @Test
    public void test_validate_error() {
        SlaBase slaBase = new SlaBase();
        PolicyDto policyDto = new PolicyDto();
        policyDto.setAction(PolicyAction.CUMULATIVE_INCREMENT);
        List<PolicyDto> policyDtos = Collections.singletonList(policyDto);
        slaBase.setPolicyList(policyDtos);
        Assert.assertThrows(LegoCheckedException.class, () -> goldenDBTSlaValidator.validateSLA(slaBase));
    }
}
