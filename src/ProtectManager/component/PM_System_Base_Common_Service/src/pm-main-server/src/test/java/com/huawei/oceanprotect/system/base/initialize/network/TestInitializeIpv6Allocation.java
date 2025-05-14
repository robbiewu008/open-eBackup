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
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.springframework.boot.test.autoconfigure.web.servlet.AutoConfigureMockMvc;
import org.springframework.test.context.junit4.SpringRunner;

import java.util.List;

/**
 * 测试Ipv6分配情况
 *
 * @author l00347293
 * @since 2021-04-12
 */
@RunWith(SpringRunner.class)
@AutoConfigureMockMvc
public class TestInitializeIpv6Allocation {
    private static AddressAllocation addressAllocation;

    private static int SPECIAL_IP_COUNT = 5;

    private static int LOGIC_IP_COUNT = 4;

    private static AddressAllocation specialAddressAllocation;

    @Before
    public void init() {
        addressAllocation = new AddressAllocation(InitConfigConstant.IPV6_TYPE_FLAG,
            "2017:8:42:96:c11::196", "2017:8:42:96:c11::1A0");

        specialAddressAllocation = new AddressAllocation(InitConfigConstant.IPV6_TYPE_FLAG,
            "2017::101", "2017::106");
    }

    /**
     * 测试获取指定个数ip是否一致-001
     *
     * @throws Exception 异常
     */
    @Test
    public void testGetIpAllocationCount() throws Exception {
        List<String> ipList = addressAllocation.getAvailableIps(SPECIAL_IP_COUNT);
        System.out.println("ZZZZZZZ:" + ipList.toString());
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
        Assert.assertTrue("2017:8:42:96:c11::196".equals(ipList.get(0)));
    }

    /**
     * 测试获取ip总数是否满足预期-003
     *
     * @throws Exception 异常
     */
    @Test
    public void testIpAllocationCount() throws Exception {
        long ipCount = addressAllocation.getAvailableIpCount();
        Assert.assertTrue(11 == ipCount);
    }

    /**
     * 测试获取ip总数是否满足预期-003
     *
     * @throws Exception 异常
     */
    @Test
    public void testGetContiousIP() throws Exception {
        String continusIpNet = addressAllocation.attainContinusIpNet(10);
         Assert.assertTrue("2017:8:42:96:c11::196-2017:8:42:96:c11::19f".equals(continusIpNet));
    }

    /**
     * 测试获取ip连续是否满足预期-004
     *
     * @throws Exception 异常
     */
    @Test
    public void testGetContiousIpv6() throws Exception {
        String continusIpNet = specialAddressAllocation.attainContinusIpNet(SPECIAL_IP_COUNT);
        Assert.assertTrue("2017::101-2017::105".equals(continusIpNet));
    }

    /**
     * 测试获取ip连续是否满足预期-004
     *
     * @throws Exception 异常
     */
    @Test
    public void test_expansion_ipv6_success() throws Exception {
        AddressAllocation addressAllocations = new AddressAllocation(InitConfigConstant.IPV6_TYPE_FLAG,
            "2017:8:42:96:c11::1001", "2017:8:42:96:c11::1014");
        List<String> availableIps = addressAllocations.getAvailableIps(LOGIC_IP_COUNT);
        Assert.assertTrue(availableIps.size() == LOGIC_IP_COUNT);
        String modifyIpNetFromIpRange = addressAllocations.getModifyIpNetFromIpRange(
            "2017:8:42:96:c11::1005-2017:8:42:96:c11::100a", 12);
        List<String> availableIps1 = addressAllocations.getAvailableIps(LOGIC_IP_COUNT);
        Assert.assertTrue(availableIps1.size() == LOGIC_IP_COUNT);
    }
}
