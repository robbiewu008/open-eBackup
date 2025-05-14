/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.network.util;

import com.huawei.oceanprotect.system.base.initialize.network.action.AddressAllocation;
import com.huawei.oceanprotect.system.base.initialize.network.beans.InitResource;
import com.huawei.oceanprotect.system.base.initialize.network.common.ExpansionIpSegment;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv4Resource;
import com.huawei.oceanprotect.system.base.initialize.network.common.Ipv6Resource;

import openbackup.system.base.common.validator.constants.RegexpConstants;

import org.springframework.util.ObjectUtils;
import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.function.Function;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * IP地址处理公共类
 *
 * @author swx1010572
 * @since 2021-09-24
 */
public class HandleIpUtils {
    private static final int OLD_NET_PLANE_COUNT = 3;

    private static final int OLD_LOGIC_PORT_COUNT = 4;

    /**
     * Ipv4或者Ipv6分配类实体化
     *
     * @param ipType 传入ip类型
     * @param startIp 起始ip
     * @param endIp 终止ip
     * @return 实体类可分配
     */
    public static AddressAllocation attainAddressInfo(String ipType, String startIp, String endIp) {
        if (InitConfigConstant.IPV6_TYPE_FLAG.equals(ipType)) {
            // IPV6的地址段
            return new AddressAllocation(InitConfigConstant.IPV6_TYPE_FLAG, startIp, endIp);
        }

        // IPV4的地址段
        return new AddressAllocation(InitConfigConstant.IPV4_TYPE_FLAG, startIp, endIp);
    }

    /**
     * 删除老版本中IP段属于逻辑端口IP段,生成新的平面网络使用Ip段
     *
     * @param ipv4cfg 老版本的IP段
     * @param controllerSize 当前控制器个数
     * @return Ipv4cfg
     */
    public static Ipv4Resource deleteIpv4LogicPortIp(Ipv4Resource ipv4cfg, final int controllerSize) {
        AddressAllocation addressAllocation = attainAddressInfo(InitConfigConstant.IPV4_TYPE_FLAG, ipv4cfg.getStartIp(),
            ipv4cfg.getEndIp());
        addressAllocation.getAvailableIps(4);
        String ipNet = addressAllocation.attainContinusIpNet(OLD_NET_PLANE_COUNT * controllerSize);
        String[] ipv4List = ipNet.split("-");
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        ipv4Resource.setStartIp(ipv4List[0]);
        ipv4Resource.setEndIp(ipv4List[1]);
        ipv4Resource.setMask(ipv4cfg.getMask());
        return ipv4Resource;
    }

    /**
     * 删除老版本中IP段属于逻辑端口IP段,生成新的平面网络使用Ip段
     *
     * @param ipv6cfg 老版本的IP段
     * @param controllerSize 当前控制器个数
     * @return Ipv6cfg
     */
    public static Ipv6Resource deleteIpv6LogicPortIp(Ipv6Resource ipv6cfg, final int controllerSize) {
        AddressAllocation addressAllocation = attainAddressInfo(InitConfigConstant.IPV6_TYPE_FLAG, ipv6cfg.getStartIp(),
            ipv6cfg.getEndIp());
        addressAllocation.getAvailableIps(OLD_LOGIC_PORT_COUNT);
        String ipNet = addressAllocation.attainContinusIpNet(OLD_NET_PLANE_COUNT * controllerSize);
        String[] ipv4List = ipNet.split("-");
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        ipv6Resource.setStartIp(ipv4List[0]);
        ipv6Resource.setEndIp(ipv4List[1]);
        ipv6Resource.setPrefix(ipv6cfg.getPrefix());
        return ipv6Resource;
    }

    /**
     * ipv4更新去除逻辑端口IP段
     *
     * @param ipv4cfg 老版本的IP段
     * @param ipv4cfgList 新IP段集合
     * @param controllerSize 当前控制器个数
     * @return Ipv4cfgList
     */
    public static List<Ipv4Resource> getIpv4NetPlaneList(List<Ipv4Resource> ipv4cfgList, Ipv4Resource ipv4cfg,
        final int controllerSize) {
        if (ipv4cfgList.size() != 0) {
            return ipv4cfgList;
        }
        Ipv4Resource ipv4Resource = new Ipv4Resource();
        if (!StringUtils.isEmpty(ipv4cfg.getStartIp())) {
            ipv4Resource = deleteIpv4LogicPortIp(ipv4cfg, controllerSize);
        }
        List<Ipv4Resource> newIpv4cfgList = new ArrayList<>();
        newIpv4cfgList.add(ipv4Resource);
        return newIpv4cfgList;
    }

    /**
     * ipv4更新去除逻辑端口IP段
     *
     * @param ipv6cfg 老版本的IP段
     * @param ipv6cfgList 新IP段集合
     * @param controllerSize 当前控制器个数
     * @return Ipv6cfgList
     */
    public static List<Ipv6Resource> getIpv6NetPlaneList(List<Ipv6Resource> ipv6cfgList, Ipv6Resource ipv6cfg,
        final int controllerSize) {
        if (ipv6cfgList.size() != 0) {
            return ipv6cfgList;
        }
        Ipv6Resource ipv6Resource = new Ipv6Resource();
        if (!StringUtils.isEmpty(ipv6cfg.getStartIp())) {
            ipv6Resource = deleteIpv6LogicPortIp(ipv6cfg, controllerSize);
        }
        List<Ipv6Resource> newIpv6cfgList = new ArrayList<>();
        newIpv6cfgList.add(ipv6Resource);
        return newIpv6cfgList;
    }

    /**
     * 归档网络  ipv4更新IP段
     *
     * @param ipv4cfg 老版本的IP段
     * @param ipv4cfgList 新IP段集合
     * @return Ipv4cfgList
     */
    public static List<Ipv4Resource> getArchiveIpv4NetPlaneList(List<Ipv4Resource> ipv4cfgList, Ipv4Resource ipv4cfg) {
        if (ipv4cfgList.size() != 0) {
            return ipv4cfgList;
        }
        List<Ipv4Resource> newIpv4cfgList = new ArrayList<>();
        newIpv4cfgList.add(ipv4cfg);
        return newIpv4cfgList;
    }

    /**
     * 归档网络  ipv6更新IP段
     *
     * @param ipv6cfg 老版本的IP段
     * @param ipv6cfgList 新IP段集合
     * @return Ipv6cfgList
     */
    public static List<Ipv6Resource> getArchiveIpv6NetPlaneList(List<Ipv6Resource> ipv6cfgList, Ipv6Resource ipv6cfg) {
        if (ipv6cfgList.size() != 0) {
            return ipv6cfgList;
        }
        List<Ipv6Resource> newIpv6cfgList = new ArrayList<>();
        newIpv6cfgList.add(ipv6cfg);
        return newIpv6cfgList;
    }

    /**
     * IPV6 平面网络段
     *
     * @param ipv6cfg 老版本的IP段
     * @return IPV6 段
     */
    public static String getIpv6netPlaneRange(InitResource ipv6cfg) {
        AddressAllocation addressAllocation = attainAddressInfo(InitConfigConstant.IPV6_TYPE_FLAG, ipv6cfg.getStartIp(),
            ipv6cfg.getEndIp());
        return addressAllocation.attainContinusIpNet();
    }

    /**
     * IPV4 平面网络段
     *
     * @param ipv4cfg 老版本的IP段
     * @return IPV4 段
     */
    public static String getIpv4netPlaneRange(InitResource ipv4cfg) {
        AddressAllocation addressAllocation = attainAddressInfo(InitConfigConstant.IPV4_TYPE_FLAG, ipv4cfg.getStartIp(),
            ipv4cfg.getEndIp());
        return addressAllocation.attainContinusIpNet();
    }

    /**
     * 生成新的ipv6地址段
     *
     * @param ipv6cfgList 可能存在 IP地址段
     * @param ipv6cfg 可能存在 IP地址段
     * @param controllerSize 当前控制器个数
     * @param isNeedLogicIp 判断是否是需要截取逻辑端口IP
     * @return 原始的IP地址段
     */
    public static List<ExpansionIpSegment> getIpv6ExpansionIpSegmentList(List<Ipv6Resource> ipv6cfgList,
        Ipv6Resource ipv6cfg, final int controllerSize, boolean isNeedLogicIp) {
        List<Ipv6Resource> ipv6NetPlaneList = new ArrayList<>();
        if (isNeedLogicIp) {
            ipv6NetPlaneList = getIpv6NetPlaneList(ipv6cfgList, ipv6cfg, controllerSize);
        } else {
            ipv6NetPlaneList = getArchiveIpv6NetPlaneList(ipv6cfgList, ipv6cfg);
        }
        if (StringUtils.isEmpty(ipv6NetPlaneList.get(0).getStartIp())) {
            return new ArrayList<>();
        }
        return ipv6NetPlaneList.stream()
            .map(ipv6Resource ->
                new ExpansionIpSegment(ipv6Resource.getStartIp(), ipv6Resource.getPrefix(), ipv6Resource.getEndIp()))
            .collect(Collectors.toList());
    }

    /**
     * 生成新的ipv4地址段
     *
     * @param ipv4cfgList 可能存在 IP地址段
     * @param ipv4cfg 可能存在 IP地址段
     * @param controllerSize 当前控制器个数
     * @param isNeedLogicIp 判断是否是需要截取逻辑端口IP
     * @return 原始的IP地址段
     */
    public static List<ExpansionIpSegment> getIpv4ExpansionIpSegmentList(List<Ipv4Resource> ipv4cfgList,
        Ipv4Resource ipv4cfg, final int controllerSize, boolean isNeedLogicIp) {
        List<Ipv4Resource> ipv4NetPlaneList = new ArrayList<>();
        if (isNeedLogicIp) {
            ipv4NetPlaneList = getIpv4NetPlaneList(ipv4cfgList, ipv4cfg, controllerSize);
        } else {
            ipv4NetPlaneList = getArchiveIpv4NetPlaneList(ipv4cfgList, ipv4cfg);
        }
        if (StringUtils.isEmpty(ipv4NetPlaneList.get(0).getStartIp())) {
            return new ArrayList<>();
        }
        return ipv4NetPlaneList.stream()
            .map(ipv4Resource ->
                new ExpansionIpSegment(ipv4Resource.getStartIp(), ipv4Resource.getMask(), ipv4Resource.getEndIp()))
            .collect(Collectors.toList());
    }

    /**
     * 判断Ipv6地址段对象是否存在
     *
     * @param ipv6cfgList 可能存在 IP地址段
     * @param ipv6cfg 可能存在 IP地址段
     * @return 原始的IP地址段
     */
    public static boolean isNetPlaneIpv6Exist(List<Ipv6Resource> ipv6cfgList, Ipv6Resource ipv6cfg) {
        if (ipv6cfgList.size() == 0 && StringUtils.isEmpty(ipv6cfg.getEndIp())) {
            return Boolean.TRUE;
        }
        return Boolean.FALSE;
    }

    /**
     * 生成新的ipv4地址段
     *
     * @param ipv4cfgList 可能存在 IP地址段
     * @param ipv4cfg 可能存在 IP地址段
     * @return 原始的IP地址段
     */
    public static boolean isNetPlaneIpv4Exist(List<Ipv4Resource> ipv4cfgList, Ipv4Resource ipv4cfg) {
        if (ipv4cfgList.size() == 0 && StringUtils.isEmpty(ipv4cfg.getEndIp())) {
            return Boolean.TRUE;
        }
        return Boolean.FALSE;
    }

    /**
     * 判断 新的Ipv4 与 老的Ipv4 是否完成一致 是否一致
     *
     * @param newIpv4cfgList 新的 IP地址段
     * @param oldIpv4cfgList 老的 IP地址段
     * @return 原始的IP地址段
     */
    public static boolean compareIpv4(List<Ipv4Resource> newIpv4cfgList, List<Ipv4Resource> oldIpv4cfgList) {
        if (newIpv4cfgList.size() != oldIpv4cfgList.size()) {
            return false;
        }

        Map<String, Ipv4Resource> collect = newIpv4cfgList.stream()
            .collect(Collectors.toMap(Ipv4Resource::getStartIp, Function.identity()));
        return oldIpv4cfgList.stream()
            .allMatch(oldIpv4Resource -> oldIpv4Resource.compare(collect.get(oldIpv4Resource.getStartIp())));
    }

    /**
     * 判断 新的Ipv6 与 老的Ipv6 是否完成一致 是否一致
     *
     * @param newIpv6cfgList 新的 IP地址段
     * @param oldIpv6cfgList 老的 IP地址段
     * @return 原始的IP地址段
     */
    public static boolean compareIpv6(List<Ipv6Resource> newIpv6cfgList, List<Ipv6Resource> oldIpv6cfgList) {
        if (newIpv6cfgList.size() != oldIpv6cfgList.size()) {
            return false;
        }

        Map<String, Ipv6Resource> collect = newIpv6cfgList.stream()
            .collect(Collectors.toMap(Ipv6Resource::getStartIp, Function.identity()));
        return oldIpv6cfgList.stream()
            .allMatch(oldIpv6Resource -> oldIpv6Resource.compare(collect.get(oldIpv6Resource.getStartIp())));
    }

    /**
     * 判断ipv4是否存在相同IP段
     *
     * @param oldIpv4cfgList 数据库中IP段
     * @param newIpv4cfgList 传入的ip段
     * @return 判断结果
     */
    public static boolean isExistSameIpv4Segment(List<Ipv4Resource> oldIpv4cfgList, List<Ipv4Resource> newIpv4cfgList) {
        // 该StartIp为数据库中startIp不会相同
        Map<String, Ipv4Resource> collect = oldIpv4cfgList.stream()
            .collect(Collectors.toMap(Ipv4Resource::getStartIp, Function.identity()));
        boolean isExistSameIpSegment = false;
        for (Ipv4Resource ipSegment : newIpv4cfgList) {
            Ipv4Resource ipv4Resource = collect.get(ipSegment.getStartIp());
            if (ObjectUtils.isEmpty(ipv4Resource) & StringUtils.isEmpty(ipv4Resource.getEndIp())) {
                continue;
            }
            isExistSameIpSegment = ipv4Resource.getEndIp().equals(ipSegment.getEndIp());
            break;
        }
        return isExistSameIpSegment;
    }

    /**
     * 判断ipv6是否存在相同IP段
     *
     * @param oldIpv6cfgList 数据库中IP段
     * @param newIpv6cfgList 传入的ip段
     * @return 判断结果
     */
    public static boolean isExistSameIpv6Segment(List<Ipv6Resource> oldIpv6cfgList, List<Ipv6Resource> newIpv6cfgList) {
        // 该StartIp为数据库中startIp不会相同
        Map<String, Ipv6Resource> collect = oldIpv6cfgList.stream()
            .collect(Collectors.toMap(Ipv6Resource::getStartIp, Function.identity()));
        boolean isExistSameIpSegment = false;
        for (Ipv6Resource ipSegment : newIpv6cfgList) {
            Ipv6Resource ipv6Resource = collect.get(ipSegment.getStartIp());
            if (ObjectUtils.isEmpty(ipv6Resource) & StringUtils.isEmpty(ipv6Resource.getEndIp())) {
                continue;
            }
            isExistSameIpSegment = ipv6Resource.getEndIp().equals(ipSegment.getEndIp());
            break;
        }
        return isExistSameIpSegment;
    }

    /**
     * 获取全新的Ipv4列表
     *
     * @param mask 子网
     * @param netPlane 要添加的ipv4列表
     * @return Ipv4列表
     */
    public static List<Ipv4Resource> getExpansionIpv4List(String mask, List<ExpansionIpSegment> netPlane) {
        List<Ipv4Resource> newIpv4cfgList = new ArrayList<>();
        netPlane.stream().map(Ipv4Resource::castFromExpansionIpSegment).forEach(newIpv4cfgList::add);
        return getNewIpv4cfgList(mask, newIpv4cfgList);
    }

    /**
     * 获取全新的Ipv6列表
     *
     * @param prefix 掩码
     * @param netPlane 要添加的ipv6列表
     * @return Ipv6列表
     */
    public static List<Ipv6Resource> getExpansionIpv6List(String prefix, List<ExpansionIpSegment> netPlane) {
        List<Ipv6Resource> newIpv6cfgList = new ArrayList<>();
        netPlane.stream().map(Ipv6Resource::castFromExpansionIpSegment).forEach(newIpv6cfgList::add);
        return getNewIpv6cfgList(prefix, newIpv6cfgList);
    }

    /**
     * 获取全新的Ipv4列表
     *
     * @param network 子网掩码
     * @param ipv4cfgList 要添加的ipv4列表
     * @return Ipv4列表
     */
    public static List<Ipv4Resource> getNewIpv4cfgList(String network, List<Ipv4Resource> ipv4cfgList) {
        List<Ipv4Resource> newIpv4cfgList = new ArrayList<>();
        ipv4cfgList.stream()
            .map(ipv4cfg -> Ipv4Resource.castFromIpv4Resource(network, ipv4cfg))
            .forEach(newIpv4cfgList::add);
        return newIpv4cfgList;
    }

    /**
     * 获取全新的Ipv4列表
     *
     * @param network 子网掩码
     * @param ipv6cfgList 要添加的ipv4列表
     * @return Ipv6列表
     */
    public static List<Ipv6Resource> getNewIpv6cfgList(String network, List<Ipv6Resource> ipv6cfgList) {
        List<Ipv6Resource> newIpv6cfgList = new ArrayList<>();
        ipv6cfgList.stream()
            .map(ipv6cfg -> Ipv6Resource.castFromIpv6Resource(network, ipv6cfg))
            .forEach(newIpv6cfgList::add);
        return newIpv6cfgList;
    }

    /**
     * 判断IP是否是IPv4或者IPv6
     *
     * @param ip ip值
     * @return 返回判断
     */
    public static boolean isIpv4OrIpv6(String ip) {
        Pattern pattern = Pattern.compile(RegexpConstants.IP_V4V6_ADDRESS);
        Matcher matcher = pattern.matcher(ip);
        return matcher.matches();
    }
}
