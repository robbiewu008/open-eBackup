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

import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * 枚举测试类
 *
 */
public class EnumTest {
    /**
     * 测试场景：DeployTypeEnum枚举类验证
     * 前置条件: 无
     * 检查点：正常获取枚举值
     */
    @Test
    public void get_deploy_type_val() {
        Assert.assertEquals(DatabaseDeployTypeEnum.SINGLE.getType(), DatabaseDeployTypeEnum.valueOf("SINGLE").getType());
    }

    /**
     * 测试场景：LockedValueEnum枚举类验证
     * 前置条件: 无
     * 检查点：正常获取枚举值
     */
    @Test
    public void get_locked_val() {
        Assert.assertEquals(LockedValueEnum.OPTIONAL.getLocked(), LockedValueEnum.valueOf("OPTIONAL").getLocked());
    }

    /**
     * 测试场景：NodeType枚举类验证
     * 前置条件: 无
     * 检查点：正常获取枚举值
     */
    @Test
    public void get_node_type_val() {
        Assert.assertEquals(NodeType.MASTER.getNodeType(), NodeType.valueOf("MASTER").getNodeType());
    }

    /**
     * 测试场景：SpeedStatisticsEnum枚举类验证
     * 前置条件: 无
     * 检查点：正常获取枚举值
     */
    @Test
    public void get_speed_statistics_val() {
        Assert.assertEquals(SpeedStatisticsEnum.UBC.getType(), SpeedStatisticsEnum.valueOf("UBC").getType());
    }
}
