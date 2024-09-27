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
package openbackup.gaussdb.protection.access.provider;

import static org.assertj.core.api.Assertions.assertThat;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import com.google.common.collect.Lists;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-14
 */
@RunWith(PowerMockRunner.class)
public class GaussDBAgentLinkStatusProviderTest {
    private GaussDBAgentLinkStatusProvider gaussDBAgentLinkStatusProvider;

    @Before
    public void init() {
        gaussDBAgentLinkStatusProvider = new GaussDBAgentLinkStatusProvider();
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void applicable_success() {
        ProtectedResource nodeResource1 = new ProtectedResource();
        nodeResource1.setSubType(ResourceSubTypeEnum.HCS_GAUSSDB_INSTANCE.getType());
        boolean applicable = gaussDBAgentLinkStatusProvider.applicable(nodeResource1);
        Assert.assertTrue(applicable);
    }

    /**
     * 用例场景：过滤
     * 前置条件：无
     * 检查点: 识别成功
     */
    @Test
    public void get_link_status_order_success() {
        List<LinkStatusEnum> list = gaussDBAgentLinkStatusProvider.getLinkStatusOrderList();
        List<LinkStatusEnum> expected = Lists.newArrayList(LinkStatusEnum.ONLINE, LinkStatusEnum.OFFLINE);
        assertThat(list).usingRecursiveComparison().isEqualTo(expected);
    }
}
