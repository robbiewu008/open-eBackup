/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.utils.network;

import openbackup.system.base.common.utils.network.Ipv6AddressUtil;

import org.junit.Assert;
import org.junit.Test;

import java.math.BigInteger;

/**
 * Ipv6AddressUtil Test
 */
public class Ipv6AddressUtilTest {
    @Test
    public void should_return_correct_biginteger_if_input_valid_ipv6_when_ipv6ToBigInteger() {
        // 1. IPv6地址范围
        String minimumIpv6 = "0000:0000:0000:0000:0000:0000:0000:0000";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(minimumIpv6), BigInteger.ZERO);
        String maximumIpv6 = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(maximumIpv6),
                new BigInteger("340282366920938463463374607431768211455"));
        // 2. 省略前导0格式
        String ipv6SecondFirst = "1050:0000:0000:0000:0005:0600:300c:326b";
        String ipv6SecondSecond = "1050:0:0:0:5:600:300c:326b";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6SecondFirst),
                new BigInteger("21683031681241440176744766643582546539"));
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6SecondSecond),
                new BigInteger("21683031681241440176744766643582546539"));
        // 3. 双冒号格式
        String ipv6ThirdFirst = "ff06:0:0:0:0:0:0:c3";
        String ipv6ThirdSecond = "ff06::c3";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6ThirdFirst),
                new BigInteger("338984292706304756556241983349463187651"));
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6ThirdSecond),
                new BigInteger("338984292706304756556241983349463187651"));
        // 4. IPv4映射的IPv6地址
        String ipv6FourthFirst = "0:0:0:0:0:ffff:192.1.56.10";
        String ipv6FourthSecond = "::ffff:192.1.56.10";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6FourthFirst),
                new BigInteger("281473903048714"));
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6FourthSecond),
                new BigInteger("281473903048714"));
        // 5. 通用示例
        String ipv6FifthFirst = "::";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6FifthFirst), BigInteger.ZERO);
        String ipv6FifthSecond = "1::";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6FifthSecond),
                new BigInteger("5192296858534827628530496329220096"));
        String ipv6FifthThird = "::1";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6FifthThird), BigInteger.ONE);
        String ipv6FifthFourth = "FEDC:BA98:7654:3210:FEDC:BA98:7654:3210";
        Assert.assertEquals(Ipv6AddressUtil.ipv6ToBigInteger(ipv6FifthFourth),
                new BigInteger("338770000845734292534325025077361652240"));
    }
}
