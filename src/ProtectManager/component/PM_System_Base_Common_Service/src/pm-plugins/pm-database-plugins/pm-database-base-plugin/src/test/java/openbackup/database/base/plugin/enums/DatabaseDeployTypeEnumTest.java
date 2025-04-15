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
package openbackup.database.base.plugin.enums;

import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * DatabaseDeployTypeEnumTest
 *
 */
public class DatabaseDeployTypeEnumTest {
    /**
     * 测试场景：DatabaseDeployTypeEnum枚举类验证
     * 前置条件: 无
     * 检查点：正常获取枚举值
     */
    @Test
    public void should_supply_agent_success() {
        Assert.assertEquals(DatabaseDeployTypeEnum.getLabel("1"), "database_single_deploy_type_label");
        Assert.assertEquals(DatabaseDeployTypeEnum.getLabel("2"), "database_aa_deploy_type_label");
        Assert.assertEquals(DatabaseDeployTypeEnum.getLabel("3"), "database_ap_deploy_type_label");
        Assert.assertEquals(DatabaseDeployTypeEnum.getLabel("4"), "database_sharding_deploy_type_label");
        Assert.assertEquals(DatabaseDeployTypeEnum.getLabel("5"), "");
    }
}