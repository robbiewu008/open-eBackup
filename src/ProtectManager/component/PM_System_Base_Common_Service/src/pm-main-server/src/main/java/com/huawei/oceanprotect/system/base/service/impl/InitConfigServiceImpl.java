/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package com.huawei.oceanprotect.system.base.service.impl;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.initialize.network.common.InitConfigConstant;
import com.huawei.oceanprotect.system.base.service.InitConfigService;

import openbackup.data.access.framework.core.dao.InitNetworkConfigMapper;
import openbackup.data.access.framework.core.dao.beans.InitConfigInfo;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.sdk.system.model.StorageAuth;

import org.apache.logging.log4j.util.Strings;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Optional;

/**
 * 获取配置信息serviceImpl类
 *
 * @author swx1010572
 * @version: [DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Service
public class InitConfigServiceImpl implements InitConfigService {
    private final InitNetworkConfigMapper initNetworkConfigMapper;

    private final EncryptorService encryptorService;

    public InitConfigServiceImpl(InitNetworkConfigMapper initNetworkConfigMapper, EncryptorService encryptorService) {
        this.initNetworkConfigMapper = initNetworkConfigMapper;
        this.encryptorService = encryptorService;
    }

    /**
     * 更新数据库的存储信息
     *
     * @param storageAuth 存储信息
     */
    @Override
    public void updateLocalStorageAuth(StorageAuth storageAuth) {
        try {
            storageAuth.setPassword(encryptorService.encrypt(storageAuth.getPassword()));
            initNetworkConfigMapper.deleteInitConfig("storageAuth");
            initNetworkConfigMapper.insertInitConfig(new InitConfigInfo("storageAuth",
                JSONObject.fromObject(storageAuth).toString()));
        } finally {
            Optional.ofNullable(storageAuth).ifPresent(a -> StringUtil.clean(a.getPassword()));
        }
    }

    /**
     * 获取数据库中存储信息
     *
     * @return 数据库中存储信息
     */
    @Override
    public StorageAuth getLocalStorageAuth() {
        List<InitConfigInfo> initConfigInfos = initNetworkConfigMapper.queryInitConfig("storageAuth");
        if (initConfigInfos == null || initConfigInfos.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }
        String initValue = initConfigInfos.get(0).getInitValue();
        StorageAuth storageAuth = JSONObject.toBean(initValue, StorageAuth.class);
        storageAuth.setPassword(encryptorService.decrypt(storageAuth.getPassword()));
        return storageAuth;
    }

    /**
     * 更新数据库的存储的deviceId
     *
     * @param deviceId 存储信息
     */
    @Override
    public void updateLocalStorageDeviceId(String deviceId) {
        initNetworkConfigMapper.deleteInitConfigByEsnAndType(InitConfigConstant.STORAGE_DEVICE_ID, deviceId);
        initNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo(InitConfigConstant.STORAGE_DEVICE_ID, deviceId));
    }

    /**
     * 获取数据库中存储的deviceId
     *
     * @return 数据库中存储的deviceId
     */
    @Override
    public String getLocalStorageDeviceId() {
        List<InitConfigInfo> initConfigInfos = initNetworkConfigMapper
            .queryInitConfig(InitConfigConstant.STORAGE_DEVICE_ID);
        if (initConfigInfos == null || initConfigInfos.size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED);
        }
        return initConfigInfos.get(0).getInitValue();
    }

    /**
     * 更新数据库的存储的deviceIp
     *
     * @param deviceIp 存储信息
     */
    @Override
    public void updateLocalStorageDeviceIp(String deviceIp) {
        initNetworkConfigMapper.deleteInitConfig("storageDeviceIp");
        initNetworkConfigMapper.insertInitConfig(
            new InitConfigInfo("storageDeviceIp", deviceIp));
    }

    /**
     * 获取数据库中存储的deviceIp
     *
     * @return 数据库中存储的deviceIp
     */
    @Override
    public String getLocalStorageDeviceIp() {
        List<InitConfigInfo> initConfigInfos = initNetworkConfigMapper.queryInitConfig("storageDeviceIp");
        if (initConfigInfos == null || initConfigInfos.size() == 0) {
            return Strings.EMPTY;
        }
        return initConfigInfos.get(0).getInitValue();
    }
}
