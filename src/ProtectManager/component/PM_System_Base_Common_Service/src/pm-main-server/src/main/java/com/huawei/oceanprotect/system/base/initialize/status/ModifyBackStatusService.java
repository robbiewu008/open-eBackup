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
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;

import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.redisson.api.RMap;
import org.redisson.api.RedissonClient;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.util.StringUtils;

import java.util.ArrayList;
import java.util.List;

/**
 * 修改状态服务
 *
 * @since 2021-03-18
 */
@Slf4j
@Service
public class ModifyBackStatusService {
    @Autowired
    private InitNetworkConfigMapper modifyBackNetworkConfigMapper;

    @Autowired
    private RedissonClient redissonClient;

    /**
     * 获取初始化状态
     *
     * @return 初始化状态
     */
    @ExterAttack
    public ConfigStatus getModifyConfigStatus() {
        ConfigStatus status = new ConfigStatus();
        getModifyStatusCode(status);
        getModifyProgressCode(status);
        getModifyProgressDesc(status);
        getModifyProgressRate(status);
        getInitParams(status);
        return status;
    }

    /**
     * 设置初始化进度编码
     *
     * @param code 进度编码
     */
    public void setModifyProgressCode(String code) {
        modifyBackNetworkConfigMapper.updateInitConfig(
            new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_CODE, code));
    }

    /**
     * 设置初始化进度描述
     *
     * @param desc 进度描述
     */
    public void setModifyProgressDesc(String desc) {
        modifyBackNetworkConfigMapper.updateInitConfig(
            new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_DESC, desc));
    }

    /**
     * 设置初始化错误码参数详情
     *
     * @param params 参数列表
     */
    public void setInitProgressParams(List<String> params) {
        modifyBackNetworkConfigMapper.updateInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_ERROR_CODE_PARAM, params.toString()));
    }

    /**
     * 设置初始化进度比率
     *
     * @param rate 进度比率
     */
    public void setModifyProgressRate(int rate) {
        modifyBackNetworkConfigMapper.updateInitConfig(
            new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_RATE, String.valueOf(rate)));
    }

    /**
     * 清理
     */
    public void clrModifyConfigStatus() {
        modifyBackNetworkConfigMapper.deleteInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_CODE);
        modifyBackNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_CODE, ""));

        modifyBackNetworkConfigMapper.deleteInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_DESC);
        modifyBackNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_DESC, ""));

        modifyBackNetworkConfigMapper.deleteInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_RATE);
        modifyBackNetworkConfigMapper.insertInitConfig(new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_RATE,
            String.valueOf(InitConfigConstant.PROGRESS_RATE_00)));
        modifyBackNetworkConfigMapper.deleteInitConfig(InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS);
        modifyBackNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS,
                String.valueOf(InitConfigConstant.ERROR_CODE_RUNNING)));

        modifyBackNetworkConfigMapper.deleteInitConfig(InitConfigConstant.INIT_ERROR_CODE_PARAM);
        modifyBackNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.INIT_ERROR_CODE_PARAM, ""));
    }

    private void getModifyStatusCode(ConfigStatus status) {
        List<InitConfigInfo> values = modifyBackNetworkConfigMapper.queryInitConfig(
            InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS);
        if (Collections.isEmpty(values)) {
            status.setStatus(Constants.ERROR_CODE_OK);
            return;
        }
        int statusCode = Integer.parseInt(values.get(0).getInitValue());
        RMap<String, String> statusMap = redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG);
        String modifyStatus = statusMap.get(InitConfigConstant.MODIFY_BACKUP_STAUS_FLAG);
        log.info("backup modify status: {}", modifyStatus);
        if (statusCode == InitConfigConstant.ERROR_CODE_RUNNING && StringUtils.isEmpty(modifyStatus)) {
            status.setStatus(InitConfigConstant.ERROR_CODE_YES);
            return;
        }
        status.setStatus(statusCode);
    }

    /**
     * 查询修改的当前状态信息
     *
     * @return 当前状态
     */
    @ExterAttack
    public ConfigStatus queryModifyingStatus() {
        ConfigStatus status = new ConfigStatus();
        status.setStatus(Constants.ERROR_CODE_OK);
        List<InitConfigInfo> values = modifyBackNetworkConfigMapper.queryInitConfig(
            InitConfigConstant.MODIFY_BACK_PROGRESS_STATUS);
        if (Collections.isEmpty(values)) {
            return status;
        }
        int statusCode = Integer.parseInt(values.get(0).getInitValue());
        RMap<String, String> statusMap = redissonClient.getMap(InitConfigConstant.INIT_RUNNING_FLAG);
        String modifyStatus = statusMap.get(InitConfigConstant.MODIFY_BACKUP_STAUS_FLAG);
        log.info("current backup modify status: {}", modifyStatus);
        if ((!StringUtils.isEmpty(modifyStatus)) && statusCode == InitConfigConstant.ERROR_CODE_RUNNING) {
            status.setStatus(InitConfigConstant.ERROR_CODE_FAILED);
            status.setCode(values.get(0).getInitValue());
        }
        return status;
    }

    private void getModifyProgressCode(ConfigStatus status) {
        List<InitConfigInfo> values = modifyBackNetworkConfigMapper.queryInitConfig(
            InitConfigConstant.MODIFY_BACK_PROGRESS_CODE);
        if (Collections.isEmpty(values)) {
            status.setCode("0");
        } else {
            status.setCode(values.get(0).getInitValue());
        }
    }

    private void getModifyProgressDesc(ConfigStatus status) {
        List<InitConfigInfo> values = modifyBackNetworkConfigMapper.queryInitConfig(
            InitConfigConstant.MODIFY_BACK_PROGRESS_DESC);
        if (Collections.isEmpty(values)) {
            status.setDesc("");
        } else {
            status.setDesc(values.get(0).getInitValue());
        }
    }

    private void getModifyProgressRate(ConfigStatus status) {
        List<InitConfigInfo> values = modifyBackNetworkConfigMapper.queryInitConfig(
            InitConfigConstant.MODIFY_BACK_PROGRESS_RATE);
        if (Collections.isEmpty(values)) {
            status.setRate(0);
        } else {
            status.setRate(Integer.parseInt(values.get(0).getInitValue()));
        }
    }

    private void getInitParams(ConfigStatus status) {
        List<InitConfigInfo> values = modifyBackNetworkConfigMapper.queryInitConfig(
            InitConfigConstant.INIT_ERROR_CODE_PARAM);
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
