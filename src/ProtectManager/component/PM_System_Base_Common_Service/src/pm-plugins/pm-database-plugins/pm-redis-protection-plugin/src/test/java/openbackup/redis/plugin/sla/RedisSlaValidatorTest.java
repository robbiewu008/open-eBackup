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
package openbackup.redis.plugin.sla;

import com.huawei.oceanprotect.sla.sdk.validator.SlaValidateConfig;

import openbackup.redis.plugin.sla.RedisSlaValidator;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * SlaValidator测试
 *
 */
@RunWith(PowerMockRunner.class)
public class RedisSlaValidatorTest {
    private RedisSlaValidator redisSlaValidator = new RedisSlaValidator();

    /**
     * 用例场景：获取Sla校验配置
     * 前置条件：服务正常、集群节点正常
     * 检查点: 配置数量为3
     */
    @Test
    public void should_return_correct_config_when_specify_nas_config() {
        SlaValidateConfig config = redisSlaValidator.getConfig();
        Assert.assertEquals(3, config.getSpecificationConfig().getPoliciesConfig().size());
    }

    /**
     * 用例场景：sla校验，类型匹配“redis”
     * 前置条件：服务正常、集群节点正常
     * 检查点: 返回True
     */
    @Test
    public void test_match_redis_sla_success() {
        Assert.assertTrue(redisSlaValidator.applicable("Redis"));
    }

    /**
     * 用例场景：sla校验，类型匹配“Mysql”
     * 前置条件：服务正常、集群节点正常
     * 检查点: 返回False
     */
    @Test
    public void test_match_redis_sla_failed() {
        Assert.assertFalse(redisSlaValidator.applicable("Mysql"));
    }
}