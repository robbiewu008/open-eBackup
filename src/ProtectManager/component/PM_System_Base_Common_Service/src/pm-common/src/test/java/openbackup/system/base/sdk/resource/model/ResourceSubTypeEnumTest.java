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
package openbackup.system.base.sdk.resource.model;

import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.junit.Assert;
import org.junit.Ignore;
import org.junit.Test;

/**
 * ResourceSubTypeEnumTest
 *
 */
public class ResourceSubTypeEnumTest {
    private static final String VM_BACKUP_AGENT_SUB_TYPE = "VMBackupAgent";

    private static final String U_BACKUP_AGENT_SUB_TYPE = "UBackupAgent";

    private static final String S_BACKUP_AGENT_SUB_TYPE = "SBackupAgent";

    private static final int VM_BACKUP_AGENT_ORDER = 2;

    private static final int U_BACKUP_AGENT_ORDER = 4;

    private static final int S_BACKUP_AGENT_ORDER = 149;

    /**
     * 测试用例：测试枚举类的功能
     * 前置条件：无
     * CHECK点：功能是否正常调用
     */
    @Test
    public void testEnumGetType() {
        Assert.assertEquals(ResourceSubTypeEnum.VM_BACKUP_AGENT.getType(), VM_BACKUP_AGENT_SUB_TYPE);
        Assert.assertEquals(ResourceSubTypeEnum.U_BACKUP_AGENT.getType(), U_BACKUP_AGENT_SUB_TYPE);
        Assert.assertEquals(ResourceSubTypeEnum.S_BACKUP_AGENT.getType(), S_BACKUP_AGENT_SUB_TYPE);
    }

    /**
     * 测试用例：测试枚举类的get()
     * 前置条件：无
     * CHECK点：功能是否正常调用
     */
    @Test
    public void testGetResourceSubTypeEnum() {
        Assert.assertEquals(ResourceSubTypeEnum.VM_BACKUP_AGENT, ResourceSubTypeEnum.get(VM_BACKUP_AGENT_SUB_TYPE));
        Assert.assertEquals(ResourceSubTypeEnum.U_BACKUP_AGENT, ResourceSubTypeEnum.get(U_BACKUP_AGENT_SUB_TYPE));
        Assert.assertEquals(ResourceSubTypeEnum.S_BACKUP_AGENT, ResourceSubTypeEnum.get(S_BACKUP_AGENT_SUB_TYPE));
    }

    /**
     * 测试用例：测试枚举类的getOrderBySubTypeSilent()
     * 前置条件：无
     * CHECK点：功能是否正常调用
     */
    @Test
    @Ignore
    public void testGetOrderBySubTypeSilent() {
        Assert.assertEquals(new Integer(VM_BACKUP_AGENT_ORDER),
                ResourceSubTypeEnum.getOrderBySubTypeSilent(VM_BACKUP_AGENT_SUB_TYPE));
        Assert.assertEquals(new Integer(U_BACKUP_AGENT_ORDER),
                ResourceSubTypeEnum.getOrderBySubTypeSilent(U_BACKUP_AGENT_SUB_TYPE));
        Assert.assertEquals(new Integer(S_BACKUP_AGENT_ORDER),
                ResourceSubTypeEnum.getOrderBySubTypeSilent(S_BACKUP_AGENT_SUB_TYPE));
    }

    /**
     * 测试用例：测试枚举类的isCommonAgent()
     * 前置条件：无
     * CHECK点：功能是否正常调用
     */
    @Test
    public void testIsCommonAgent() {
        Assert.assertTrue(ResourceSubTypeEnum.isSelectAgent(VM_BACKUP_AGENT_SUB_TYPE));
        Assert.assertTrue(ResourceSubTypeEnum.isSelectAgent(U_BACKUP_AGENT_SUB_TYPE));
        Assert.assertTrue(ResourceSubTypeEnum.isSelectAgent(S_BACKUP_AGENT_SUB_TYPE));
    }
}
