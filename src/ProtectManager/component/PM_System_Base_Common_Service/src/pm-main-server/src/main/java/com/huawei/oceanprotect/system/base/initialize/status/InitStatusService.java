/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.initialize.status;

import com.huawei.oceanprotect.base.cluster.sdk.service.ClusterBasicService;
import com.huawei.oceanprotect.system.base.constant.InitNetworkConfigConstants;
import com.huawei.oceanprotect.system.base.initialize.network.common.ConfigStatus;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import feign.codec.DecodeException;
import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 初始化状态服务
 *
 * @author w00493811
 * @since 2021-03-18
 */
@Component
@Slf4j
@Service
public class InitStatusService {
    @Autowired
    private InitNetworkConfigMapper initNetworkConfigMapper;

    @Autowired
    private RedissonClient redissonClient;

    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private ClusterBasicService clusterBasicService;

    /**
     * 获取初始化状态
     *
     * @return 初始化状态
     */
    public ConfigStatus getInitConfigStatus() {
        ConfigStatus status = new ConfigStatus();
        String esn = clusterBasicService.getCurrentClusterEsn();
        fillInitStatusCode(status, esn);
        fillInitProgressCode(status, esn);
        fillInitProgressDesc(status, esn);
        fillInitProgressRate(status, esn);
        fillInitParams(status, esn);
        return status;
    }

    /**
     * 设置初始化进度编码
     *
     * @param code 进度编码
     * @param deviceId 设备编号
     */
    public void setInitProgressCode(String code, String deviceId) {
        initNetworkConfigMapper.updateInitConfigByEsnAndType(
            new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_CODE, code, deviceId));
    }

    /**
     * 设置初始化进度描述
     *
     * @param desc 进度描述
     * @param deviceId 设备编号
     */
    public void setInitProgressDesc(String desc, String deviceId) {
        initNetworkConfigMapper.updateInitConfigByEsnAndType(
            new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_DESC, desc, deviceId));
    }

    /**
     * 设置初始化错误码参数详情
     *
     * @param params 参数列表
     * @param deviceId 设备编号
     *
     */
    public void setInitProgressParams(List<String> params, String deviceId) {
        initNetworkConfigMapper.updateInitConfigByEsnAndType(
            new InitConfigInfo(InitConfigConstant.INIT_ERROR_CODE_PARAM, params.toString(), deviceId));
    }

    /**
     * 设置初始化进度比率
     *
     * @param rate 进度比率
     */
    public void setInitProgressRate(int rate) {
        initNetworkConfigMapper.updateInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_RATE, String.valueOf(rate)));
    }

    /**
     * 设置初始化进度比率
     *
     * @param rate 进度比率
     * @param deviceId 设备编号
     */
    public void setInitProgressRate(int rate, String deviceId) {
        initNetworkConfigMapper.updateInitConfigByEsnAndType(
            new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_RATE, String.valueOf(rate), deviceId));
    }

    /**
     * 清理
     *
     * @param deviceId 设备编号
     */
    public void clrInitConfigStatus(String deviceId) {
        initNetworkConfigMapper.deleteInitConfigByEsnAndType(InitConfigConstant.INIT_PROGRESS_CODE, deviceId);
        initNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_CODE, "", deviceId));

        initNetworkConfigMapper.deleteInitConfigByEsnAndType(InitConfigConstant.INIT_PROGRESS_DESC, deviceId);
        initNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_DESC, "", deviceId));

        initNetworkConfigMapper.deleteInitConfigByEsnAndType(InitConfigConstant.INIT_PROGRESS_RATE, deviceId);
        initNetworkConfigMapper.insertInitConfig(new InitConfigInfo(InitConfigConstant.INIT_PROGRESS_RATE,
            String.valueOf(InitConfigConstant.PROGRESS_RATE_00), deviceId));

        initNetworkConfigMapper.deleteInitConfigByEsnAndType(InitConfigConstant.INIT_ERROR_CODE_PARAM, deviceId);
        initNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_ERROR_CODE_PARAM, "", deviceId));

        initNetworkConfigMapper.deleteInitConfigByEsnAndType(Constants.BACKUP_NETWORK_FLAG, deviceId);
        initNetworkConfigMapper.deleteInitConfigByEsnAndType(Constants.ARCHIVE_NETWORK_FLAG, deviceId);
        initNetworkConfigMapper.deleteInitConfigByEsnAndType(Constants.REPLICATION_NETWORK_FLAG, deviceId);
    }

    /**
     * 更新初始化状态为OK
     */
    public void updateInitStatus() {
        initNetworkConfigMapper.deleteInitConfig(Constants.INIT_ERROR_FLAG);
        initNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(Constants.INIT_ERROR_FLAG, String.valueOf(Constants.ERROR_CODE_OK)));
    }

    /**
     * 查询初始化的当前状态信息
     *
     * @return 当前状态
     */
    public ConfigStatus queryInitStatus() {
        ConfigStatus status = new ConfigStatus();
        List<InitConfigInfo> statusCode = initNetworkConfigMapper.queryInitConfig(Constants.INIT_ERROR_FLAG);
        if (Collections.isEmpty(statusCode)) {
            status.setStatus(InitConfigConstant.ERROR_CODE_YES);
            return status;
        }
        int initValue = Integer.valueOf(statusCode.get(0).getInitValue());
        if (initValue == InitConfigConstant.ERROR_CODE_NO || initValue == Constants.ERROR_CODE_OK) {
            status.setStatus(Constants.ERROR_CODE_OK);
            return status;
        }
        RMap<String, String> redisMap = redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG);
        String initStatus = redisMap.get(InitConfigConstant.INIT_STAUS_FLAG);
        log.info("system initialize status: {}", initStatus);
        if (initValue == InitConfigConstant.ERROR_CODE_RUNNING
            && (!StringUtils.isEmpty(initStatus))) {
            log.info("current process is initializing, do nothing");
            status.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);
            return status;
        }
        status.setStatus(InitConfigConstant.ERROR_CODE_YES);
        initNetworkConfigMapper.deleteInitConfig(Constants.INIT_ERROR_FLAG);
        return status;
    }

    private void fillInitStatusCode(ConfigStatus status, String esn) {
        List<InitConfigInfo> modifyStatus = initNetworkConfigMapper.queryInitConfigByEsnAndType(
            Constants.MODIFY_STATUS_FLAG, esn);
        if (VerifyUtil.isEmpty(modifyStatus)) {
            fillInitStatus(status, esn);
        } else {
            fillModifyStatus(status, modifyStatus.get(0));
        }
    }

    private void fillInitStatus(ConfigStatus status, String esn) {
        List<InitConfigInfo> values = initNetworkConfigMapper.queryInitConfigByEsnAndType(Constants.INIT_ERROR_FLAG,
            esn);
        if (Collections.isEmpty(values)) {
            status.setStatus(InitConfigConstant.ERROR_CODE_RUNNING);
        } else {
            status.setStatus(Integer.parseInt(values.get(0).getInitValue()));
        }
    }

    private boolean isModifyOverTime(InitConfigInfo modifyStatus) {
        if (isModifyRunning(modifyStatus)) {
            long lastInitTime = modifyStatus.getCreateTime();
            return System.currentTimeMillis() - lastInitTime > InitNetworkConfigConstants.MODIFY_OVER_TIME;
        }
        return false;
    }
    private void fillModifyStatus(ConfigStatus status, InitConfigInfo modifyStatus) {
        if (isModifyOverTime(modifyStatus)) {
            status.setStatus(InitConfigConstant.MODIFY_NETWORK_ERROR);
        } else {
            status.setStatus(Integer.parseInt(modifyStatus.getInitValue()));
        }
    }

    private boolean isModifyRunning(InitConfigInfo modifyStatus) {
        return String.valueOf(InitConfigConstant.MODIFY_NETWORK_RUNNING).equals(modifyStatus.getInitValue());
    }

    private void fillInitProgressCode(ConfigStatus status, String esn) {
        List<InitConfigInfo> values = initNetworkConfigMapper.queryInitConfigByEsnAndType(
            InitConfigConstant.INIT_PROGRESS_CODE, esn);
        if (Collections.isEmpty(values)) {
            status.setCode("0");
        } else {
            status.setCode(values.get(0).getInitValue());
        }
    }

    private void fillInitProgressDesc(ConfigStatus status, String esn) {
        List<InitConfigInfo> values = initNetworkConfigMapper.queryInitConfigByEsnAndType(
            InitConfigConstant.INIT_PROGRESS_DESC, esn);
        if (Collections.isEmpty(values)) {
            status.setDesc("");
        } else {
            status.setDesc(values.get(0).getInitValue());
        }
    }

    private void fillInitProgressRate(ConfigStatus status, String esn) {
        List<InitConfigInfo> values = initNetworkConfigMapper.queryInitConfigByEsnAndType(
            InitConfigConstant.INIT_PROGRESS_RATE, esn);
        if (Collections.isEmpty(values)) {
            status.setRate(0);
        } else {
            status.setRate(Integer.parseInt(values.get(0).getInitValue()));
        }
    }

    private void fillInitParams(ConfigStatus status, String esn) {
        List<InitConfigInfo> values = initNetworkConfigMapper.queryInitConfigByEsnAndType(
            InitConfigConstant.INIT_ERROR_CODE_PARAM, esn);
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

    /**
     * 从network-conf配置文件中获取backup_net_plane来判断初始化是否完成
     *
     * @return 是否初始化完成
     */
    public boolean isInitFinish() {
        try {
            infrastructureRestApi.getCommonConfValue(Constants.NAME_SPACE,
                Constants.CONFIG_MAP, Constants.BACKUP_NET_PLANE);
        } catch (DecodeException | LegoUncheckedException e) {
            log.info("Init is not finish.", ExceptionUtil.getErrorMessage(e));
            return false;
        }
        return true;
    }
}
