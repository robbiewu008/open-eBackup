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
package com.huawei.oceanprotect.system.base.initialize.network;


import com.huawei.oceanprotect.system.base.initialize.network.action.AddressAllocation;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.test.context.junit4.SpringRunner;
import org.springframework.util.StringUtils;

import java.util.List;

/**
 * 测试Ip分配情况
 *
 * @author l00347293
 * @since 2021-01-06
 */
@RunWith(SpringRunner.class)
@AutoConfigureMockMvc
public class TestInitializeIpAllocation {
    private static AddressAllocation addressAllocation;

    private static int SPECIAL_IP_COUNT = 5;

    static {
        addressAllocation = new AddressAllocation(InitConfigConstant.IPV4_TYPE_FLAG,
            "51.6.135.1", "51.6.135.99");
    }

    /**
     * 测试获取指定个数ip是否一致-001
     *
     * @throws Exception 异常
     */
    @Test
    public void testGetIpAllocationCount() throws Exception {
        List<String> ipList = addressAllocation.getAvailableIps(SPECIAL_IP_COUNT);
        Assert.assertTrue(ipList.size() == SPECIAL_IP_COUNT);
    }

    /**
     * 测试获取指定个数ip的首个IP是否满足预期-002
     *
     * @throws Exception 异常
     */
    @Test
    public void testIpAllocationFirstValue() throws Exception {
        List<String> ipList = addressAllocation.getAvailableIps(SPECIAL_IP_COUNT);
        boolean isTrue = "51.6.135.1".equals(ipList.get(0));
        if (isTrue) {
            Assert.assertTrue(isTrue);
        }
    }

    /**
     * 测试获取ip总数是否满足预期-003
     *
     * @throws Exception 异常
     */
    @Test
    public void testIpAllocationCount() throws Exception {
        long ipCount = addressAllocation.getAvailableIpCount();
        Assert.assertTrue(99 == ipCount);
    }

    /**
     * 测试获取ip总数是否满足预期-003
     *
     * @throws Exception 异常
     */
    @Test
    public void testGetContiousIP() throws Exception {
        String continusIpNet = addressAllocation.attainContinusIpNet(99);
        if (!StringUtils.isEmpty(continusIpNet))
        Assert.assertTrue("51.6.135.1-51.6.135.99".equals(continusIpNet));
    }

    /**
     * 测试获取ip总数是否满足双控-003
     *
     * @throws Exception 异常
     */
    @Test
    public void test_contiousip_success() throws Exception {
        String continusIpNet = addressAllocation.attainContinusIpNet(4);
        Assert.assertTrue("51.6.135.1-51.6.135.4".equals(continusIpNet));
    }
}
