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
package openbackup.db2.protection.access.enums;

import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;

import org.junit.Assert;
import org.junit.Test;

/**
 * @author lWX776769
 * @version [DataBackup 1.3.0]
 * @since 2023-02-02
 */
public class Db2ClusterTypeEnumTest {
    /**
     * 用例场景：db2集群枚举值未定义
     * 前置条件：枚举值未定义
     * 检查点: 抛出异常
     */
    @Test
    public void should_throw_IllegalArgumentException_if_enum_is_not_defined_when_get_by_type() {
        IllegalArgumentException illegalArgumentException = Assert.assertThrows(IllegalArgumentException.class,
            () -> Db2ClusterTypeEnum.getByType("test"));
        Assert.assertEquals(null, illegalArgumentException.getMessage());
    }
}