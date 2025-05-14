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
package com.huawei.oceanprotect.system.base.initialize.status;

import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatusKey;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.enums.ServiceType;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;

/**
 * 初始化状态服务
 *
 * @author w00493811
 * @since 2021-03-18
 */
@Component
@Slf4j
@Service
public class InitStandardBackupService {
    @Autowired
    private InitNetworkConfigMapper initStandardBackupConfigMapper;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * 获取初始化状态
     *
     * @param progressCode 当前参数具体值;
     * @return 初始化状态
     */
    @ExterAttack
    public ConfigStatus getInitConfigStatus(ServiceType progressCode) {
        ConfigStatus configStatus = new ConfigStatus(InitConfigConstant.ERROR_CODE_YES, "0", "", 0,
            new ArrayList<>());
        ConfigStatusKey statusKey = InitConfigConstant.SERVICE_PROGRESS_KEY.get(progressCode);

        // 获取status
        Optional.ofNullable(initStandardBackupConfigMapper.queryInitConfig(statusKey.getStatusKey()))
            .ifPresent(list -> {
                if (Collections.isEmpty(list) || (StringUtils.isEmpty(
                    redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG).get(statusKey.getInitRedisFlagKey()))
                    && (Integer.parseInt(list.get(0).getInitValue()) == InitConfigConstant.ERROR_CODE_RUNNING
                    || Integer.parseInt(list.get(0).getInitValue())
                        == InitConfigConstant.ERROR_CODE_STANDARD_ROLLBACKING))) {
                    // 1.当无进度状态 2.运行状态检查的redis值不存在并且进度状态为运行中时（线程异常退出） 返回可重入状态
                    configStatus.setStatus(InitConfigConstant.ERROR_CODE_YES);
                    return;
                }
                configStatus.setStatus(Integer.parseInt(list.get(0).getInitValue()));
            });

        // 获取code
        Optional.ofNullable(initStandardBackupConfigMapper.queryInitConfig(statusKey.getCodeKey()))
            .ifPresent(list -> {
                if (!Collections.isEmpty(list)) {
                    configStatus.setCode(list.get(0).getInitValue());
                }
            });

        // 获取description
        Optional.ofNullable(initStandardBackupConfigMapper.queryInitConfig(statusKey.getDescKey()))
            .ifPresent(list -> {
                if (!Collections.isEmpty(list)) {
                    configStatus.setDesc(list.get(0).getInitValue());
                }
            });

        // 获取rate
        Optional.ofNullable(initStandardBackupConfigMapper.queryInitConfig(statusKey.getRateKey()))
            .ifPresent(list -> {
                if (!Collections.isEmpty(list)) {
                    configStatus.setRate(Integer.parseInt(list.get(0).getInitValue()));
                }
            });
        // 获取错误码参数列表
        getInitParams(configStatus, progressCode);
        return configStatus;
    }

    /**
     * 设置初始化进度编码
     *
     * @param code 进度编码
     * @param serviceType 服务类型
     */
    public void setInitProgressCode(String code, ServiceType serviceType) {
        String serviceProgressCode = InitConfigConstant.SERVICE_PROGRESS_KEY.get(serviceType).getCodeKey();
        initStandardBackupConfigMapper.updateInitConfig(
            new InitConfigInfo(serviceProgressCode, code));
    }

    /**
     * 设置初始化进度描述
     *
     * @param desc 进度描述
     */
    public void setInitProgressDesc(String desc) {
        initStandardBackupConfigMapper.updateInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_STANDARD_PROGRESS_DESC, desc));
    }

    /**
     * 设置初始化错误码参数详情
     *
     * @param params 参数列表
     * @param serviceType 服务类型
     */
    public void setInitProgressParams(List<String> params, ServiceType serviceType) {
        String serviceProgressParams = InitConfigConstant.SERVICE_PROGRESS_KEY.get(serviceType).getErrorCodeParamKey();
        initStandardBackupConfigMapper.updateInitConfig(
            new InitConfigInfo(serviceProgressParams, params.toString()));
    }

    /**
     * 设置服务进度状态
     *
     * @param status 进度状态
     * @param serviceType 服务类型
     */
    public void setInitProgressStatus(int status, ServiceType serviceType) {
        String serviceProgressStatus = InitConfigConstant.SERVICE_PROGRESS_KEY.get(serviceType).getStatusKey();
        initStandardBackupConfigMapper.updateInitConfig(new InitConfigInfo(serviceProgressStatus,
            String.valueOf(status)));
    }

    /**
     * 设置服务进度比率
     *
     * @param rate 进度比率
     * @param serviceType 服务类型
     */
    public void setInitProgressRate(int rate, ServiceType serviceType) {
        String serviceProgressRate = InitConfigConstant.SERVICE_PROGRESS_KEY.get(serviceType).getRateKey();
        initStandardBackupConfigMapper.updateInitConfig(new InitConfigInfo(serviceProgressRate,
            String.valueOf(rate)));
    }

    /**
     * 重置服务对应的进度状态
     *
     * @param serviceType 服务类型
     */
    public void resetServiceInitProgressConfig(ServiceType serviceType) {
        // 根据服务类型清理相关的服务进度状态
        ConfigStatusKey configStatusKey = InitConfigConstant.SERVICE_PROGRESS_KEY.get(serviceType);

        // 更新服务类型对应的进度为1：运行中
        initStandardBackupConfigMapper.deleteInitConfig(configStatusKey.getStatusKey());
        initStandardBackupConfigMapper.insertInitConfig(new InitConfigInfo(configStatusKey.getStatusKey(),
            String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING)));

        // 清理服务类型对应的进度错误码
        initStandardBackupConfigMapper.deleteInitConfig(configStatusKey.getCodeKey());
        initStandardBackupConfigMapper.insertInitConfig(new InitConfigInfo(configStatusKey.getCodeKey(), ""));

        // 清理服务类型对应的进度描述
        initStandardBackupConfigMapper.deleteInitConfig(configStatusKey.getDescKey());
        initStandardBackupConfigMapper.insertInitConfig(new InitConfigInfo(configStatusKey.getDescKey(), ""));

        // 清理服务类型对应的进度比率
        initStandardBackupConfigMapper.deleteInitConfig(configStatusKey.getRateKey());
        initStandardBackupConfigMapper.insertInitConfig(
            new InitConfigInfo(configStatusKey.getRateKey(), String.valueOf(InitConfigConstant.PROGRESS_RATE_05)));

        // 此处与上面不同，是清除初始化错误相关的参数列表
        initStandardBackupConfigMapper.deleteInitConfig(configStatusKey.getErrorCodeParamKey());
        initStandardBackupConfigMapper.insertInitConfig(
            new InitConfigInfo(configStatusKey.getErrorCodeParamKey(), ""));
    }

    /**
     * 查询服务类型的当前状态信息
     *
     * @param serviceType 服务类型
     * @return isServiceProgressRunning 当前服务线程是否运行中：任何一个服务status为运行中或Redis中的key为运行
     */
    public boolean isServiceProgressRunning(ServiceType serviceType) {
        ConfigStatusKey statusKey = InitConfigConstant.SERVICE_PROGRESS_KEY.get(serviceType);
        return isRedisFlagRunning(statusKey.getInitRedisFlagKey());
    }

    /**
     * 判断Redis中本线程Flag对应的值是否为运行中，1分钟过期
     *
     * @param initRedisFlagKey RedisRunningFlag对应的key
     * @return isRedisFlagRunning RedisRunningFlag中key对应值是否为Running
     */
    public boolean isRedisFlagRunning(String initRedisFlagKey) {
        RMap<String, String> statusMap = redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG);
        String redisStatusFlag = statusMap.get(initRedisFlagKey);
        log.info("current standard service progress status redis flag: {}", redisStatusFlag);
        return !StringUtils.isEmpty(redisStatusFlag);
    }

    /**
     * 判断标准备份服务进度状态是否为运行中，数据库中的值
     *
     * @param statusKey statusKey 对应的key
     * @return isServiceStatusRunning SFTP服务或标准备份服务开启进度状态是否为运行中
     */
    private boolean isServiceProgressStatusRunning(String statusKey) {
        List<InitConfigInfo> values = initStandardBackupConfigMapper.queryInitConfig(
            statusKey);
        // 两者服务状态都不存在时，则认为未运行
        if (Collections.isEmpty(values)) {
            return false;
        }

        // 某一个状态存在则判断状态是否为Running或者回滚中
        if (!Collections.isEmpty(values)) {
            return InitConfigConstant.RUNNING_STATUS.get(Integer.parseInt(values.get(0).getInitValue()));
        }
        return false;
    }

    private void getInitParams(ConfigStatus status, ServiceType progressCode) {
        ConfigStatusKey statusKey = InitConfigConstant.SERVICE_PROGRESS_KEY.get(progressCode);
        List<InitConfigInfo> values = initStandardBackupConfigMapper.queryInitConfig(statusKey.getErrorCodeParamKey());
        if (!Collections.isEmpty(values)) {
            String params = values.get(0).getInitValue();
            if (StringUtils.isEmpty(params)) {
                return;
            }
            String trimParams = params.substring(1, params.length() - 1); // 处理如[20,30]常规字符串两侧的中括号[]
            String[] paramSplit = trimParams.split("\\,");
            log.info("getInitParams: {}", paramSplit.toString());
            List<String> list = new ArrayList<>();
            for (String param : paramSplit) {
                list.add(param.trim());
            }

            status.setParams(list);
        }
    }
}
