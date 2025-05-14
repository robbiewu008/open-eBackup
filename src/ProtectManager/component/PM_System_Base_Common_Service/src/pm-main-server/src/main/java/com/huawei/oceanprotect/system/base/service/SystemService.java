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
package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.DependentInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.LldInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.ManualInitNetworkBody;
import com.huawei.oceanprotect.system.base.initialize.network.common.PacificInitNetworkBody;
import com.huawei.oceanprotect.system.base.vo.DeviceInfo;

import openbackup.system.base.bean.DeviceUser;

/**
 * 系统服务接口
 *
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-04
 */
public interface SystemService {
    /**
     * 获取pacific 多节点下的业务网络配置信息
     *
     * @param manageIp 节点的管理ip
     * @return 业务网络配置信息
     */
    NetworkInfoDto getNetworkInfo(String manageIp);

    /**
     * 获取pacific 指定节点下的业务网络配置信息
     *
     * @param manageIp 节点的管理ip
     * @param ifaceName 端口
     * @param ipAddress ip地址和掩码
     * @return 业务网络配置信息
     */
    NodeNetworkInfoDto getNodeNetworkInfo(String manageIp, String ifaceName, String ipAddress);

    /**
     * 配置存储认证，成功会将存储认证信息写入secret，并返回设备id
     *
     * @param deviceUser 认证信息
     * @return deviceId
     */
    String configStorageAuth(DeviceUser deviceUser);

    /**
     * 更新指定设备和用户的secret
     *
     * @param deviceUser deviceUser
     */
    void updateServiceUserDeviceSecret(DeviceUser deviceUser);

    /**
     * 获取设备信息:esn和用户名
     *
     * @return 获取设备信息:esn和用户名
     */
    DeviceInfo getDeviceInfo();

    /**
     * 系统是否已经初始化
     *
     * @return true:已经初始化 false:未初始化
     */
    boolean isInitialized();

    /**
     * 手动初始化
     *
     * @param manualInitNetworkBody 初始化参数
     * @return 初始化状态
     */
    ConfigStatus createManualInitConfig(ManualInitNetworkBody manualInitNetworkBody);

    /**
     * lld初始化
     *
     * @param lldInitNetworkBody 初始化参数
     * @return 初始化状态
     */
    ConfigStatus createLldInitConfig(LldInitNetworkBody lldInitNetworkBody);

    /**
     * 分布式一体机初始化
     *
     * @param pacificInitNetworkBody 初始化参数
     * @return 初始化状态
     */
    ConfigStatus createPacificInitConfig(PacificInitNetworkBody pacificInitNetworkBody);

    /**
     * 软硬解耦初始化
     *
     * @param dependentInitNetworkBody 初始化参数
     * @return 初始化状态
     */
    ConfigStatus createDependentInitConfig(DependentInitNetworkBody dependentInitNetworkBody);
}
