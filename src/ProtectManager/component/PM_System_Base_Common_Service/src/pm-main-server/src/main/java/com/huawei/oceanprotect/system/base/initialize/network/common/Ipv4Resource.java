/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.common;

import com.huawei.oceanprotect.system.base.initialize.network.beans.InitResource;

import com.fasterxml.jackson.annotation.JsonIgnoreProperties;
import com.fasterxml.jackson.annotation.JsonInclude;

import lombok.Data;
import openbackup.system.base.common.validator.constants.RegexpConstants;

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
public class Ipv4Resource {
    /**
     * 起始IP
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct ip startIp address")
    private String startIp;

    /**
     * 结束IP
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct ip endIp address")
    private String endIp;

    /**
     * IPv4掩码
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct mask address")
    private String mask;

    /**
     * 网关
     */
    @Pattern(regexp = RegexpConstants.IPADDRESS_V4, message = "please input correct gateway address")
    private String gateway;

    /**
     * 比较Ipv4资源内容是否完全一致
     *
     * @param res 比较资源
     * @return 是否内容相等
     */
    public boolean compare(Ipv4Resource res) {
        if (res == null) {
            return false;
        }
        if (this == res) {
            return true;
        }
        if ((!endIp.equals(res.getEndIp())) || (!startIp.equals(res.getStartIp())) || (!mask.equals(res.getMask()))) {
            return false;
        }
        return true;
    }

    /**
     * 获取构造新的ipv4Resource
     *
     * @param ipv4Segment ipv4Segment
     * @return 新的ipv4Resource
     */
    public static Ipv4Resource castFromExpansionIpSegment(ExpansionIpSegment ipv4Segment) {
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp(ipv4Segment.getStartIp());
        ipv4Resource.setEndIp(ipv4Segment.getEndIp());
        return ipv4Resource;
    }

    /**
     * 获取构造新的ipv4Resource
     *
     * @param mask 子网掩码
     * @param ipv4cfg ipv4cfg
     * @return 新的ipv4Resource
     */
    public static Ipv4Resource castFromIpv4Resource(String mask, Ipv4Resource ipv4cfg) {
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp(ipv4cfg.getStartIp());
        ipv4Resource.setEndIp(ipv4cfg.getEndIp());
        ipv4Resource.setMask(mask);
        return ipv4Resource;
    }

    /**
     * 获取构造新的InitResource
     *
     * @return 新的initResource
     */
    public InitResource castFromInitResource() {
        return new InitResource(startIp, endIp, mask);
    }
}