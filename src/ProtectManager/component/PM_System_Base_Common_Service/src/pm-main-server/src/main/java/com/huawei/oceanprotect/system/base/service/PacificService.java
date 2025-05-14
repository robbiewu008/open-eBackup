/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service;

import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;

/**
 * pacific服务接口
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-09
 */
public interface PacificService {
    /**
     * 获取业务网络配置信息NetworkInfo
     *
     * @param deviceId 设备id
     * @param username username
     * @return 业务网络配置信息NetworkInfo
     */
    NetworkInfoDto getNetworkInfo(String deviceId, String username);

    /**
     * 获取业务网络配置信息NetworkInfo
     *
     * @param deviceId 设备id
     * @param username username
     * @param manageIp 管理ip
     * @return 业务网络配置信息NetworkInfo
     */
    NetworkInfoDto getNetworkInfo(String deviceId, String username, String manageIp);
}
