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
package openbackup.system.base.service;

import openbackup.system.base.bean.ClusterInternalNetPlane;
import openbackup.system.base.bean.DeviceNetworkInfo;
import openbackup.system.base.bean.NetWorkConfigInfo;
import openbackup.system.base.bean.NetWorkLogicIp;
import openbackup.system.base.bean.NetWorkRouteInfo;
import openbackup.system.base.bean.NetworkBaseInfo;
import openbackup.system.base.bean.NetworkConnectRequest;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.infrastructure.model.beans.NodePodInfo;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 初始化网络通用的服务
 *
 */
public interface NetworkService {
    /**
     * 获取网络信息
     *
     * @param networkName 网络名字
     * @return 网络信息
     */
    List<NetWorkConfigInfo> getNetWork(String networkName);

    /**
     * 获取内部通信ip
     *
     * @param communicateNetWork 网络名字
     * @return 内部通信ip
     */
    Optional<ClusterInternalNetPlane> getCommunicateNetWork(String communicateNetWork);


    /**
     * 获取网络信息
     *
     * @param communicateNetWork 网络名字
     * @return 网络信息
     */
    Optional<String> getCommunicateNetWorkIp(String communicateNetWork);

    /**
     * 获取系统网络配置
     *
     * @return 系统网络配置
     */
    DeviceNetworkInfo getDeviceNetworkInfo();

    /**
     * 获取初始化配置的网络平面ip集合
     *
     * @param netPlane 初始化配置的网络平面信息
     * @return 初始化配置的网络平面ip集合
     */
    List<String> getNetPlaneIp(List<NetWorkConfigInfo> netPlane);

    /**
     * 获取初始化配置的网络平面的ip和mask集合
     *
     * @param netPlane 初始化配置的网络平面信息
     * @return 初始化配置的网络平面的ip和mask集合
     */
    List<NetWorkLogicIp> getNetPlaneIpList(List<NetWorkConfigInfo> netPlane);

    /**
     * 获取ip及对应的初始化配置的网络平面的路由的map
     *
     * @param netPlane 初始化配置的网络平面信息
     * @return 初始化配置的网络平面中ip及对应的路由的map
     */
    Map<String, List<NetWorkRouteInfo>> getNetPlaneIpRouteList(List<NetWorkConfigInfo> netPlane);

    /**
     * 是否从版本1.6之前升级
     *
     * @return true:是 false:否
     */
    boolean isUpdateFromBeforeVersionSix();

    /**
     * 数据库是否存在初始化配置信息
     *
     * @return true:是 false:否
     */
    boolean isDbExistInitConfigInfo();

    /**
     * 检查到底座的连通性：如果isUseVrf为true，则使用ip vrf exec vrf-srv curl -kv
     * https://2.2.2.2:9527 --interface 1.1.1.1
     * --connect-timeout 1检查到指定控制器ip的连通性，否则使用curl -kv https://2.2.2.2:9527
     * --connect-timeout 1检查到当前控制器
     * 的连通性
     *
     * @param request 连通性请求
     * @return 是否连通
     */
    boolean isNetworkConnectivity(NetworkConnectRequest request);


    /**
     * 检查两个ip是否是同网段
     *
     * @param source 源ip
     * @param target 目标ip
     * @return 是否是同网段
     */
    boolean isSameSubNetwork(NetworkBaseInfo source, NetworkBaseInfo target);

    /**
     * 是否能创建绑定端口
     *
     * @param ethPortIdList 太网口端口的id
     */
    void checkBondPortCanCreate(List<String> ethPortIdList);

    /**
     * 以太网端口是否在自定义漂移组中
     *
     * @param ethPortId 以太网端口id
     * @return 自定义漂移组名称
     */
    String getFailOverGroupOfEthPort(String ethPortId);

    /**
     * 查询network-conf里的网络信息
     *
     * @return 网络信息
     */
    Map<String, JSONArray> queryNetworkInfo();

    /**
     * 查询节点网络信息
     *
     * @return 节点网络信息
     */
    Map<String, NodePodInfo> queryNodeNetInfo();

    /**
     * 检查两个ipv4地址是否为同网段
     *
     * @param ipAddress1 ipv4地址1
     * @param mask1 ipv4格式掩码1
     * @param ipAddress2 ipv4地址2
     * @param mask2 掩码2
     * @return 是否为同网段
     */
    boolean isIpv4SameNetworkSegment(String ipAddress1, String mask1, String ipAddress2, String mask2);

    /**
     * 检查两个ipv6地址是否为同网段
     *
     * @param ipAddress1 ipv6地址1
     * @param mask1 ipv6格式掩码1
     * @param ipAddress2 ipv6地址2
     * @param mask2 ipv6掩码2
     * @return 是否为同网段
     */
    boolean isIpv6SameNetworkSegment(String ipAddress1, String mask1, String ipAddress2, String mask2);

    /**
     * 检查当前节点是否有管理IP
     * 如果遇到网络配置信息或者当前节点名获取错误的情况 直接返回false
     *
     * @param isSync 是否需要同步
     * @return 是否有管理ip
     */
    boolean isNodeHaveManageIp(boolean isSync);

    /**
     * 检查当前节点是否在高优先级队列
     *
     * @return 是否在高优先级队列(转发时优先尝试)
     */
    boolean isNodeInHighPriority();

    /**
     * 检查当前请求是否是转发请求
     * 如果遇到网络配置信息或者当前节点名获取错误的情况 直接返回false
     *
     * @return 是否是转发的请求(检查请求头)
     */
    boolean isForwardRequest();

    /**
     * 是否需要立刻抛出异常转发给其他节点
     * 如果当前节点第一次被请求 且不是高优先级节点和管理ip节点时 则迅速转发给高优先级节点以节省时间
     *
     * @return 是否需要立刻转发
     */
    boolean shouldFastForward();
}
