/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize;

/**
 * 初始化bean对象类
 *
 * @author swx1010572
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-29
 */
public class MockNetworkBean {
    public static String backNetWork = "{\"ipType\":\"IPV4\",\"ipv4AnybackupVip\":\"129.115.132.10\","
        + "\"ipv4cfgList\":[{\"startIp\":\"129.115.132.15\",\"endIp\":\"129.115.132.150\","
        + "\"mask\":\"255.255.0.0\"}],\"ipv4VlanId\":\"1\",\"ipv4RouteCfg\":[],\"ipv6cfg\":{},"
        + "\"ipv6RouteCfg\":[],\"existIpv4\":true,\"existIpv6\":false,\"mtu\":\"1500\"}";

    public static String backNetWorkFromCopy = "{\"ipType\":\"IPV4\",\"ipv4AnybackupVip\":\"129.115.132.10\","
        + "\"ipv4cfgList\":[{\"startIp\":\"129.115.132.15\",\"endIp\":\"129.115.132.150\","
        + "\"mask\":\"255.255.0.0\"}],\"ipv4VlanId\":\"1\",\"ipv4RouteCfg\":[],\"ipv6cfg\":{},"
        + "\"ipv6RouteCfg\":[],\"existIpv4\":true,\"existIpv6\":false,\"mtu\":\"1500\"}";

    public static String backNetWorkDiffMask = "{\"ipType\":\"IPV4\",\"ipv4AnybackupVip\":\"129.115.132.10\","
        + "\"ipv4cfgList\":[{\"startIp\":\"129.115.132.15\",\"endIp\":\"129.115.132.150\","
        + "\"mask\":\"255.255.255.0\"}],\"ipv4VlanId\":\"1\",\"ipv4RouteCfg\":[],\"ipv6cfg\":{},"
        + "\"ipv6RouteCfg\":[],\"existIpv4\":true,\"existIpv6\":false,\"mtu\":\"1500\"}";

    public static String archiveNetWork = "{ \"existIpv4Dns\": true, \"existIpv4Vlan\": false, \"existIpv6Dns\": true, \"existIpv6Vlan\": true, \"ipType\": \"IPV6\", \"ipv4DnsServers\": [ \"51.6.135.226\",\"51.6.135.227\" ],"
        + "\"ipv4RouteCfg\": [ { \"gateway\": \"192.168.20.1\", \"subNetMask\": \"255.255.255.0\", \"targetAddress\": \"51.6.135.0\" } ], \"ipv4VlanId\": \"\", "
        + "\"ipv4cfgList\": [{ \"endIp\": \"192.168.20.49\", \"gateway\": \"192.168.20.1\", \"mask\": \"255.255.255.0\", \"startIp\": \"192.168.20.12\" }], "
        + "\"ipv6DnsServers\": [ \"2017:8::40\", \"2017:8::48\" ], \"ipv6RouteCfg\": [ { \"gateway\": \"2017:8::27\", \"prefixLength\": \"64\", \"targetAddress\": \"3000::0\" } ], "
        + "\"ipv6VlanId\": \"string\", \"ipv6cfgList\": [{ \"endIp\": \"2017:8::100\", \"prefix\": \"64\", \"startIp\": \"2017:8::50\" }],\"mtu\":\"1500\"";
}
