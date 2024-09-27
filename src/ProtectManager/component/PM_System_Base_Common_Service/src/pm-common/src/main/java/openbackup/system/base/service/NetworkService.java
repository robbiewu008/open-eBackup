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
import openbackup.system.base.bean.NetworkBaseInfo;
import openbackup.system.base.bean.NetworkConnectRequest;
import openbackup.system.base.common.utils.JSONObject;

import java.util.List;
import java.util.Optional;

/**
 * 初始化网络通用的服务
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024/2/28
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
     * 获取网络信息
     *
     * @param networkName 网络名字
     * @return 网络信息的json对象
     */
    JSONObject getNetWorkOfJson(String networkName);


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
}
