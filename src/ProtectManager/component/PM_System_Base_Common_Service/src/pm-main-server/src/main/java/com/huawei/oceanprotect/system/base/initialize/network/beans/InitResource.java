/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.beans;

import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4Resource;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv6Resource;

import lombok.Getter;
import lombok.Setter;

/**
 * IP资源的相关信息
 *
 * @author swx1010572
 * @version: [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-29
 */
@Getter
@Setter
public class InitResource {
    /**
     * 起始IP
     */
    private String startIp;

    /**
     * 结束IP
     */
    private String endIp;

    /**
     * 子网掩码
     */
    private String subNetMask;

    /**
     * 网关
     */
    private String gateway;

    public InitResource(String startIp, String endIp, String subNetMask) {
        this.startIp = startIp;
        this.endIp = endIp;
        this.subNetMask = subNetMask;
    }

    /**
     * 获取构造新的Ipv6Resource
     *
     * @return 新的Ipv6Resource
     */
    public Ipv6Resource castFromIpv6Resource() {
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp(startIp);
        ipv6Resource.setEndIp(endIp);
        ipv6Resource.setPrefix(subNetMask);
        return ipv6Resource;
    }


    /**
     * 获取构造新的Ipv4Resource
     *
     * @return 新的iIpv4Resource
     */
    public Ipv4Resource castFromIpv4Resource() {
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp(startIp);
        ipv4Resource.setEndIp(endIp);
        ipv4Resource.setMask(subNetMask);
        return ipv4Resource;
    }
}