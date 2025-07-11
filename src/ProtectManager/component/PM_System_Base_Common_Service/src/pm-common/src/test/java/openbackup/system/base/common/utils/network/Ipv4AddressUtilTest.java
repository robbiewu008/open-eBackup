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
package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.utils.network.Ipv4AddressUtil;

import org.junit.Assert;
import org.junit.Test;

public class Ipv4AddressUtilTest {
    @Test
    public void testConvertIpv4() {
        Assert.assertEquals(Ipv4AddressUtil.ipv4ToLong(""), 0);
        Assert.assertEquals(Ipv4AddressUtil.ipv4ToLong("1.1.1.1"), 16843009L);
        Assert.assertFalse(Ipv4AddressUtil.isValidIPv4("1.1.1"));
        Assert.assertFalse(Ipv4AddressUtil.isValidIPv4("127.0.0.1"));
        Assert.assertFalse(Ipv4AddressUtil.isValidIPv4(""));
        Assert.assertTrue(Ipv4AddressUtil.isValidIPv4("1.1.1.1"));
    }

    @Test
    public void should_return_true_if_ip_is_valid_when_isValidIpv4All() {
        String ip = "192.168.0.1";
        boolean result = Ipv4AddressUtil.isValidIpv4All(ip);
        Assert.assertTrue(result);
    }

    @Test
    public void should_return_false_if_ip_is_invalid_when_isValidIpv4All() {
        String firstIp = "0.0.0.0";
        boolean firstResult = Ipv4AddressUtil.isValidIpv4All(firstIp);
        Assert.assertFalse(firstResult);

        String secondIp = "127.0.0.1";
        boolean secondResult = Ipv4AddressUtil.isValidIpv4All(secondIp);
        Assert.assertFalse(secondResult);

        String thirdIp = "224.0.0.0";
        boolean thirdResult = Ipv4AddressUtil.isValidIpv4All(thirdIp);
        Assert.assertFalse(thirdResult);

        String fourthIp = "1";
        boolean fourthResult = Ipv4AddressUtil.isValidIpv4All(fourthIp);
        Assert.assertFalse(fourthResult);
    }

    @Test
    public void testGetNetMaskPrefixLen() {
        Assert.assertEquals(Ipv4AddressUtil.calNetMaskPrefixLen("255.255.0.0"), 16);
        Assert.assertEquals(Ipv4AddressUtil.calNetMaskPrefixLen("255.255.64.0"), 16);
    }
}
