/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.controller.log.context;

import com.huawei.oceanprotect.system.base.initialize.network.common.InitNetworkBody;

import com.alibaba.fastjson.JSON;

import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import org.springframework.stereotype.Service;

import java.util.Objects;

/**
 * InitLogService
 *
 * @author l30057246
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-05-30
 */
@Slf4j
@Service
@CalleeMethods(name = "initLogParamsService")
public class InitLogParamsService {
    /**
     * 获取pacific备份网络配置的json 字符串
     *
     * @param request request
     * @return 备份网络配置的json 字符串
     */
    @CalleeMethod
    public String getBackupNetworkInfoFromRequest(InitNetworkBody request) {
        if (Objects.isNull(request.getBackupNetworkConfig())
            || Objects.isNull(request.getBackupNetworkConfig().getPacificInitNetWorkInfoList())) {
            return "--";
        }
        return JSON.toJSONString(request.getBackupNetworkConfig().getPacificInitNetWorkInfoList());
    }

    /**
     * 获取pacific归档网络配置的json 字符串
     *
     * @param request request
     * @return 归档网络配置的json 字符串
     */
    @CalleeMethod
    public String getArchiveNetworkInfoFromRequest(InitNetworkBody request) {
        if (Objects.isNull(request.getArchiveNetworkConfig())
            || Objects.isNull(request.getArchiveNetworkConfig().getPacificInitNetWorkInfoList())) {
            return "--";
        }
        return JSON.toJSONString(request.getArchiveNetworkConfig().getPacificInitNetWorkInfoList());
    }

    /**
     * 获取pacific复制网络配置的json 字符串
     *
     * @param request request
     * @return 复制网络配置的json 字符串
     */
    @CalleeMethod
    public String getCopyNetworkInfoFromRequest(InitNetworkBody request) {
        if (Objects.isNull(request.getCopyNetworkConfig())
            || Objects.isNull(request.getCopyNetworkConfig().getPacificInitNetWorkInfoList())) {
            return "--";
        }
        return JSON.toJSONString(request.getCopyNetworkConfig().getPacificInitNetWorkInfoList());
    }

    /**
     * 获取LogicPort备份网络配置的json 字符串
     *
     * @param request request
     * @return 备份网络配置的json 字符串
     */
    @CalleeMethod
    public String getBackupNetworkName(InitNetworkBody request) {
        if (Objects.isNull(request.getBackupNetworkConfig())
            || Objects.isNull(request.getBackupNetworkConfig().getLogicPorts())) {
            return "--";
        }
        return JSON.toJSONString(request.getBackupNetworkConfig().getLogicPorts());
    }

    /**
     * 获取LogicPort复制网络配置的json 字符串
     *
     * @param request request
     * @return 复制网络配置的json 字符串
     */
    @CalleeMethod
    public String getCopyNetworkName(InitNetworkBody request) {
        if (Objects.isNull(request.getCopyNetworkConfig())
            || Objects.isNull(request.getCopyNetworkConfig().getLogicPorts())) {
            return "--";
        }
        return JSON.toJSONString(request.getCopyNetworkConfig().getLogicPorts());
    }

    /**
     * 获取LogicPort归档网络配置的json 字符串
     *
     * @param request request
     * @return 归档网络配置的json 字符串
     */
    @CalleeMethod
    public String getArchiveNetworkName(InitNetworkBody request) {
        if (Objects.isNull(request.getArchiveNetworkConfig())
            || Objects.isNull(request.getArchiveNetworkConfig().getLogicPorts())) {
            return "--";
        }
        return JSON.toJSONString(request.getArchiveNetworkConfig().getLogicPorts());
    }

    /**
     * 获取name 字符串
     *
     * @param request request
     * @return name的json 字符串
     */
    @CalleeMethod
    public String getStorageAuthName(InitNetworkBody request) {
        if (Objects.isNull(request.getStorageAuth())) {
            return "--";
        }
        return request.getStorageAuth().getUsername();
    }
}
