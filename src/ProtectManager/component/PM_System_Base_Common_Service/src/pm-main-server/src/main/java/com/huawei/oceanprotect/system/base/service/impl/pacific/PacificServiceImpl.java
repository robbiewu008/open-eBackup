/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl.pacific;

import com.huawei.oceanprotect.system.base.dto.pacific.NetworkInfoDto;
import com.huawei.oceanprotect.system.base.dto.pacific.NodeNetworkInfoDto;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.service.OpenStorageService;
import com.huawei.oceanprotect.system.base.service.PacificService;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.sdk.devicemanager.entity.NodeInfoDto;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * pacific服务接口实现类
 *
 * @author y30046482
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-01-09
 */
@Service
@Slf4j
public class PacificServiceImpl implements PacificService {
    @Autowired
    private OpenStorageService openStorageService;

    /**
     * 获取业务网络配置信息NetworkInfo
     *
     * @param deviceId 设备id
     * @param username username
     * @return 业务网络配置信息NetworkInfo
     */
    @Override
    public NetworkInfoDto getNetworkInfo(String deviceId, String username) {
        return getNetworkInfo(deviceId, username, null);
    }

    /**
     * 获取业务网络配置信息NetworkInfo
     *
     * @param deviceId 设备id
     * @param username username
     * @param manageIp 管理ip 支持模糊查询
     * @return 业务网络配置信息NetworkInfo
     */
    @Override
    public NetworkInfoDto getNetworkInfo(String deviceId, String username, String manageIp) {
        log.info("Get network info start, device id: {}.", deviceId);
        Map<String, NodeInfoDto> networkInfo = openStorageService.getNetworkInfo(deviceId, username);
        if (StringUtils.isEmpty(manageIp)) {
            List<NodeNetworkInfoDto> nodeNetworkInfoList = networkInfo.entrySet()
                .stream()
                .map(entry -> new NodeNetworkInfoDto(entry.getKey(), entry.getValue().getIpList()))
                .collect(Collectors.toList());
            return new NetworkInfoDto(nodeNetworkInfoList);
        }
        List<NodeNetworkInfoDto> collect = networkInfo.keySet().stream()
            .filter(ip -> ip.contains(manageIp))
            .map(ip -> {
                NodeInfoDto nodeInfoDto = networkInfo.get(ip);
                return new NodeNetworkInfoDto(nodeInfoDto.getPacificNodeInfoVo().getManageIp(),
                    nodeInfoDto.getIpList());
        }).collect(Collectors.toList());

        return new NetworkInfoDto(collect);
    }
}
