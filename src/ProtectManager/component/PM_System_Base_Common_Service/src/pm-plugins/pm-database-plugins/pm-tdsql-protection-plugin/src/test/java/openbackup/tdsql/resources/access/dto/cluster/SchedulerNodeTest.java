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
package openbackup.tdsql.resources.access.dto.cluster;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;

import org.junit.Assert;
import org.junit.Test;

/**
 * 功能描述
 *
 */
public class SchedulerNodeTest {
    /**
     * 用例场景：测试SchedulerNode类
     * 前置条件：equals()和hashCode()方法正确
     * 检查点：校验通过
     */
    @Test
    public void test_scheduler_node() {
        EqualsVerifier.simple().forClass(SchedulerNode.class).verify();
        EqualsVerifier.simple().forClass(SchedulerNode.class).usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
