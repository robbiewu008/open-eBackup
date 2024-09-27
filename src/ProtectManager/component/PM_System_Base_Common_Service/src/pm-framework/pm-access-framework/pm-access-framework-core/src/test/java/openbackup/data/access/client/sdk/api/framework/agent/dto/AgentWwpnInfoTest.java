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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import nl.jqno.equalsverifier.EqualsVerifier;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentWwpnInfo;

import org.junit.Assert;
import org.junit.Test;

/**
 * AgentWwpnInfo测试类
 *
 * @author l30023229
 * @since 2023-02-21
 */
public class AgentWwpnInfoTest {
    /**
     * 测试对象的属性，方法（hash, equals）
     */
    @Test
    public void test() {
        EqualsVerifier.simple().forClass(AgentWwpnInfo.class).withIgnoredFields("errorCode","errorMessage").verify();
        EqualsVerifier.simple().forClass(AgentWwpnInfo.class).withIgnoredFields("errorCode","errorMessage").usingGetClass().verify();
        Assert.assertTrue(true);
    }
}
