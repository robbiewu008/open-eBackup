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

import openbackup.system.base.common.utils.network.AddressUtil;

import org.junit.Assert;
import org.junit.Test;

/**
 * AddressUtil
 *
 * @author w00439064
 * @since 2021-04-05
 */
public class AddressUtilTest {
    /**
     * testCheckLocalIP
     */
    @Test
    public void testCheckLocalIP() {
        Assert.assertFalse(AddressUtil.checkLocalIP(null));
        Assert.assertFalse(AddressUtil.checkLocalIP(""));
        Assert.assertTrue(AddressUtil.checkLocalIP("127.0.0.1"));
        Assert.assertTrue(AddressUtil.checkLocalIP("localHost"));
        Assert.assertFalse(AddressUtil.checkLocalIP("1.1.1.1"));
        Assert.assertNotNull(AddressUtil.getLocalIP());
    }

    /**
     * testIpToInt
     */
    @Test
    public void testIpToInt() {
        Assert.assertNull(AddressUtil.ipToInt(null).orElse(null));
        Assert.assertEquals(AddressUtil.ipToInt("127.0.0.1").get().intValue(), 2130706433);
        Assert.assertEquals(AddressUtil.ipToInt("0:0:0:0:0:0:0:1").get().intValue(), 1);
        Assert.assertEquals(AddressUtil.ipToInt("0:0::0:0:0:0:1").get().intValue(), 1);
        Assert.assertEquals(AddressUtil.ipToInt("0:0:0:0:0:0::1").get().intValue(), 1);
    }

    /**
     * testLongToIPv4
     */
    @Test
    public void testLongToIPv4() {
        Assert.assertEquals(AddressUtil.longToIPv4(2130706433L), "127.0.0.1");
    }

    /**
     * testIsValidIpv4
     */
    @Test
    public void testIsValidIpv4() {
        Assert.assertFalse(AddressUtil.isValidIPv4(""));
        Assert.assertFalse(AddressUtil.isValidIPv4("323.0.0.34"));
        Assert.assertFalse(AddressUtil.isValidIPv4("127.0.0.1"));
        Assert.assertTrue(AddressUtil.isValidIPv4("123.1.0.1"));
    }

    /**
     * testIsValidIpv6
     */
    @Test
    public void testIsValidIpv6() {
        Assert.assertFalse(AddressUtil.isValidIPv6(""));
        Assert.assertFalse(AddressUtil.isValidIPv6("0:0:0:0:0:0:0:1"));
        Assert.assertFalse(AddressUtil.isValidIPv6("0:0:0:0:0:0:0:"));
        Assert.assertFalse(AddressUtil.isValidIPv6("0:0:0:0:0:0:0:"));
        Assert.assertTrue(AddressUtil.isValidIPv6("1:1:1:1:1:1:1:1"));
    }

    /**
     * testIsIpAddress
     */
    @Test
    public void testIsIpAddress() {
        Assert.assertTrue(AddressUtil.isIpAddress("1.1.1.1"));
    }

    /**
     * testFillIpv6
     */
    @Test
    public void testFillIpv6() {
        Assert.assertEquals(AddressUtil.fillIpv6("1:1:1:1:1:1:1:1", "-"),
            "0001-0001-0001-0001-0001-0001-0001-0001");
        Assert.assertEquals(AddressUtil.fillIpv6(":1:1:1:1:1:1:", "-"),
                "0000-0001-0001-0001-0001-0001-0001-0000");
        Assert.assertEquals(AddressUtil.fillIpv6("::1:1:1:1::", "-"),
                "0000-0000-0001-0001-0001-0001-0000-0000");
        Assert.assertEquals(AddressUtil.fillIpv6("::11:221:1231:1::", "-"),
                "0000-0000-0011-0221-1231-0001-0000-0000");
        Assert.assertEquals(AddressUtil.fillIpv6("", "-"),
                "0000");
    }

    /**
     * testIpType
     */
    @Test
    public void testIpType() {
        Assert.assertEquals(AddressUtil.ipType(""), "");
        Assert.assertEquals(AddressUtil.ipType("127.0.0.1"), "IPV4");
        Assert.assertEquals(AddressUtil.ipType("1:1:1:1:1:1:1:1"), "IPV6");
    }

    /**
     * testConvertIpv6
     */
    @Test
    public void testConvertIpv6() {
        Assert.assertEquals(AddressUtil.convertIpv6("1:1:1:1:1:1:1:1"), "[1:1:1:1:1:1:1:1]");
        Assert.assertEquals(AddressUtil.convertIpv6("[1:1:1:1:1:1:1:1]"), "[1:1:1:1:1:1:1:1]");
    }

    /**
     * testConvertIpv6
     */
    @Test
    public void testFormatIPv6() {
        Assert.assertEquals(AddressUtil.formatIPv6("1:1:1:1:1:1:1:1"), "[1:1:1:1:1:1:1:1]");
    }

    /**
     * testGetStrWithoutPercent
     */
    @Test
    public void testGetStrWithoutPercent() {
        Assert.assertEquals(AddressUtil.getStrWithoutPrecent("127.0.0.1%1234"), "127.0.0.1");
        Assert.assertEquals(AddressUtil.getStrWithoutPrecent("127.0.0.1"), "127.0.0.1");
    }

    /**
     * testGetStrWithoutPercent
     */
    @Test
    public void testIsValidIP() {
        Assert.assertFalse(AddressUtil.isValidIP("127.0.0.1"));
    }

    /**
     * testMaskToIp
     */
    @Test
    public void testMaskToIp() {
        Assert.assertEquals(AddressUtil.maskToIp(17), "255.255.128.0");
    }

    /**
     * testIsSameVersion
     */
    @Test
    public void testIsSameVersion() {
        Assert.assertTrue(AddressUtil.isSameVersion("192.168.1.1", "127.0.0.1"));
        Assert.assertFalse(AddressUtil.isSameVersion("192.168.1.1", "0:0:0:0:0:0:0:1"));
        Assert.assertFalse(AddressUtil.isSameVersion("test1", "test2"));
        Assert.assertTrue(AddressUtil.isSameVersion("0:0:0:0:0:0:0:1", "0:0:0:0:0:0:0:1"));
    }

    /**
     * TestIsLoopbackAddress
     */
    @Test
    public void TestIsLoopbackAddress() {
        Assert.assertTrue(AddressUtil.isLoopbackAddress("127.0.0.1"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("127.255.255.254"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("localhost"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("::1"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("::ffff:7f00:1"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("::ffff:127.0.0.1"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("127.1"));
        Assert.assertTrue(AddressUtil.isLoopbackAddress("127.0.1"));
        Assert.assertFalse(AddressUtil.isLoopbackAddress("8.40.143.102"));
        Assert.assertFalse(AddressUtil.isLoopbackAddress("192.168.143.102"));
        Assert.assertFalse(AddressUtil.isLoopbackAddress("ff06:0:0:0:0:0:0:c3"));
    }
}
