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
package openbackup.sqlserver.protection.sla;

import openbackup.sqlserver.protection.sla.SqlServerSlaValidatorProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * SQL Server的SLA应用校验
 *
 */
public class SqlServerSlaValidatorProviderTest {
    private SqlServerSlaValidatorProvider sqlServerSlaValidatorProvider = new SqlServerSlaValidatorProvider();

    /**
     * 用例场景：查找SLA Validator
     * 前置条件：分别输入正确和错误的子应用类型类型
     * 检 查 点：分别得到成功和失败的查找结果
     */
    @Test
    public void applicable_sqlserver_sla_validator_success() {
        Assert.assertTrue(sqlServerSlaValidatorProvider.applicable(ResourceSubTypeEnum.SQL_SERVER.getType()));
        Assert.assertFalse(sqlServerSlaValidatorProvider.applicable(ResourceSubTypeEnum.U_BACKUP_AGENT.getType()));
    }

    /**
     * 用例场景：查找SQL Server SLA配置限制
     * 前置条件：无
     * 检 查 点：成功获得需要的类型
     */
    @Test
    public void sqlserver_sla_validator_get_config_success() {
        Assert.assertEquals(5,
            sqlServerSlaValidatorProvider.getConfig().getSpecificationConfig().getPoliciesConfig().size());
    }
}
