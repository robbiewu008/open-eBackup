/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitResource;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import openbackup.system.base.common.utils.network.AddressUtil;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.Pattern;

/**
 * IP资源的相关信息
 *
 * @author l00347293
 * @since 2020-12-08
 */
@Data
@JsonIgnoreProperties(ignoreUnknown = true)
@JsonInclude(JsonInclude.Include.NON_NULL)
public class Ipv6Resource {
    /**
     * 起始IP
     */
    @Pattern(regexp = AddressUtil.IPV6REG, message = "please input correct ipv6 startIp address")
    private String startIp;

    /**
     * 结束IP
     */
    @Pattern(regexp = AddressUtil.IPV6REG, message = "please input correct ipv6 endIp address")
    private String endIp;

    /**
     * ipv6前缀
     */
    @Length(min = 1, max = 3, message = "The length of description is 1 ~ 3")
    private String prefix;

    /**
     * 网关
     */
    @Pattern(regexp = AddressUtil.IPV6REG, message = "please input correct ipv6 gateway")
    private String gateway;

    /**
     * 比较Ipv6资源内容是否完全一致
     *
     * @param res 比较资源
     * @return 是否内容相等
     */
    public boolean compare(Ipv6Resource res) {
        if (res == null) {
            return false;
        }
        if (this == res) {
            return true;
        }
        if ((!endIp.equals(res.getEndIp())) || (!startIp.equals(res.getStartIp()))
            || (!prefix.equals(res.getPrefix()))) {
            return false;
        }
        return true;
    }

    /**
     * 获取构造新的ipv6Resource
     *
     * @param ipv6Segment ipv6Segment
     * @return 新的ipv6Resource
     */
    public static Ipv6Resource castFromExpansionIpSegment(ExpansionIpSegment ipv6Segment) {
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp(ipv6Segment.getStartIp());
        ipv6Resource.setEndIp(ipv6Segment.getEndIp());
        return ipv6Resource;
    }

    /**
     * 获取构造新的ipv6Resource
     *
     * @param prefix 掩码
     * @param ipv6cfg ipv6cfg
     * @return 新的ipv6Resource
     */
    public static Ipv6Resource castFromIpv6Resource(String prefix, Ipv6Resource ipv6cfg) {
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp(ipv6cfg.getStartIp());
        ipv6Resource.setEndIp(ipv6cfg.getEndIp());
        ipv6Resource.setPrefix(prefix);
        return ipv6Resource;
    }

    /**
     * 获取构造新的InitResource
     *
     * @return 新的initResource
     */
    public InitResource castFromInitResource() {
        return new InitResource(startIp, endIp, prefix);
    }
}