/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.utils.network.IPv6Address;
import openbackup.system.base.common.utils.network.IPv6AddressRange;
import openbackup.system.base.common.utils.network.IPv6Network;
import openbackup.system.base.common.utils.network.IPv6NetworkMask;

import org.junit.Assert;
import org.junit.Test;

import java.math.BigInteger;

public class IPv6AddressRangeTest {
    @Test
    public void testIPv6Address() {
        IPv6Address first1 = new IPv6Address(1, 1);
        IPv6Address last1 = new IPv6Address(2, 2);
        IPv6AddressRange range1 = new IPv6AddressRange(first1, last1);
        Assert.assertNotNull(IPv6AddressRange.fromFirstAndLast(first1, last1));
        Assert.assertTrue(range1.contains(first1));
        IPv6Address first2 = new IPv6Address(3, 3);
        IPv6Address last2 = new IPv6Address(4, 4);
        IPv6Address address1 = new IPv6Address(3, 10);
        IPv6Address address2 = new IPv6Address(5, 10);
        IPv6AddressRange range2 = new IPv6AddressRange(first2, last2);
        Assert.assertFalse(range1.contains(range2));
        Assert.assertFalse(range1.overlaps(range2));
        Assert.assertEquals(range1.size(), new BigInteger("18446744073709551618"));
        Assert.assertNotNull(range1.toSubnets());
        Assert.assertEquals(range2.remove(first1).get(0), range2);
        Assert.assertNotNull(range2.remove(first2));
        Assert.assertNotNull(range2.remove(last2));
        Assert.assertNotNull(range2.remove(address1));
        Assert.assertNotNull(range2.extend(first1));
        Assert.assertNotNull(range2.extend(address1));
        Assert.assertNotNull(range2.extend(address2));
        IPv6NetworkMask mask1 = new IPv6NetworkMask(24);
        IPv6Network network1  =  IPv6Network.fromAddressAndMask(first2, mask1);
        Assert.assertNotNull(range2.remove(network1));
    }

    @Test(expected = IllegalArgumentException.class)
    public void testIPv6AddressRaiseException() {
        IPv6Address first = new IPv6Address(2, 2);
        IPv6Address last = new IPv6Address(1, 1);
        IPv6AddressRange range = new IPv6AddressRange(first, last);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testIPv6AddressRemoveWhenNullAddressRaiseException() {
        IPv6Address first = new IPv6Address(1, 1);
        IPv6Address last = new IPv6Address(2, 2);
        IPv6AddressRange range = new IPv6AddressRange(first, last);
        range.remove((IPv6Address)null);
    }

    @Test(expected = IllegalArgumentException.class)
    public void testIPv6AddressRemoveWhenNullNetworkRaiseException() {
        IPv6Address first = new IPv6Address(1, 1);
        IPv6Address last = new IPv6Address(2, 2);
        IPv6AddressRange range = new IPv6AddressRange(first, last);
        range.remove((IPv6Network)null);
    }
}
