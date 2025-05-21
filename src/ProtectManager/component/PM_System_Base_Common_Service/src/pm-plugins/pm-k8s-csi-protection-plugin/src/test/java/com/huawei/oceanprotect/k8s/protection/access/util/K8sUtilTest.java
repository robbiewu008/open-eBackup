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
package com.huawei.oceanprotect.k8s.protection.access.util;

import com.huawei.oceanprotect.k8s.protection.access.constant.K8sExtendInfoKey;

import org.junit.Assert;
import org.junit.Test;

/**
 * K8sUtil测试类
 *
 */
public class K8sUtilTest {
    /**
     * 用例场景：根据agent id获取extend info key
     * 前置条件：无
     * 检查点: 获取成功，无异常
     */
    @Test
    public void test_get_internal_agent_connection_key() {
        String agentId = "1";
        String internalAgentConnectionKey = K8sUtil.getInternalAgentConnectionKey(agentId);
        Assert.assertEquals(K8sExtendInfoKey.INTERNAL_AGENT_CONNECTION_PREFIX + agentId, internalAgentConnectionKey);
    }

    /**
     * 用例场景：根据xtend info key获取agent id
     * 前置条件：无
     * 检查点: 获取成功，无异常
     */
    @Test
    public void test_get_agent_id_from_key() {
        String key1 = K8sExtendInfoKey.INTERNAL_AGENT_CONNECTION_PREFIX + "22";
        Assert.assertEquals("22", K8sUtil.getAgentIdFromExtendInfoKey(key1).get());

        String key2 = K8sExtendInfoKey.INTERNAL_AGENT_CONNECTION_PREFIX;
        Assert.assertEquals("", K8sUtil.getAgentIdFromExtendInfoKey(key2).get());

        String key3 = "33";
        Assert.assertFalse(K8sUtil.getAgentIdFromExtendInfoKey(key3).isPresent());
    }
}
