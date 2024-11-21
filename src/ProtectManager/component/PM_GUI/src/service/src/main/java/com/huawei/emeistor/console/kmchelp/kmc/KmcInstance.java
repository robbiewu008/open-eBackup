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
package com.huawei.emeistor.console.kmchelp.kmc;

import com.huawei.emeistor.console.kmchelp.KmcHelper;
import com.huawei.kmc.common.AppException;
import com.huawei.kmc.common.ILogger;
import com.huawei.kmc.common.InitStage;
import com.huawei.kmc.common.KmcMkInfo;
import com.huawei.kmc.crypt.CryptoAPI;

import lombok.extern.slf4j.Slf4j;

import java.io.IOException;
import java.util.Locale;
import java.util.Objects;
import java.util.Optional;
import java.util.Properties;

/**
 * 文 件 名: KmcInstance
 * 包 名: com.huawei.kmctests.unitest
 * 描 述: 获取CryptoAPI单例实例, 本例是为了向前兼容
 * 建议产品直接通过 CryptoAPI.getInstance()获取实例
 *
 */
@Slf4j
public class KmcInstance {
    static final ILogger logger = new CryptLogger();

    private KmcInstance() {}

    /**
     * 外部获取实例
     * 为保持API demo例子的延续性，这里将调用CryptoAPI类的getInstance
     *
     * @return CryptoAPI 加密API实例对象
     */
    public static CryptoAPI getInstance() {
        return CryptoAPI.getInstance();
    }

    /**
     * 释放API实例
     *
     * @throws Exception e
     */
    public static synchronized void releaseComponent() throws Exception {
        getInstance().finalized();
    }

    /**
     * 初始化KMC组件
     *
     * @param path 初始化配置文件
     * @param backupPath 初始化备份配置文件
     * @param isOnce 是否一次性使用
     * @param isRunning 是否是长期运行
     * @throws IOException e
     * @throws AppException e
     */
    public static synchronized void initComponent(String path, String backupPath, boolean isOnce, boolean isRunning)
            throws IOException, AppException {
        initComponent(path, backupPath, isOnce, isRunning, null);
    }

    /**
     * 初始化KMC组件
     *
     * @param path 初始化配置文件
     * @param backupPath 初始化备份配置文件
     * @param isOnce 是否一次性使用
     * @param isRunning 是否是长期运行
     * @param configPath kmc配置文件路径
     * @throws IOException e
     * @throws AppException e
     */
    public static synchronized void initComponent(String path, String backupPath, boolean isOnce, boolean isRunning,
        String configPath) throws IOException, AppException {
        // 当前的用户是否是root,是的话调用shell 删除信号量
        // DPA要求所有调用加密都是用同一个用户
        if (InitStage.INIT_KMC_DONE.getValue() == CryptoAPI.getInitStage()) {
            return;
        }
        Properties kmcProps =
            SysPropertiesUtils.getPropertiesFromFile(Optional.ofNullable(configPath).orElse(KmcHelper.getConfigPath()));
        // 使找的路径不是当前执行路径，是绝对路径
        kmcProps.put(Constant.PRIMARY_KEY_STORE_FILE, path + "/" + kmcProps.get(Constant.PRIMARY_KEY_STORE_FILE));
        if (!Objects.equals(backupPath, "")) {
            kmcProps.put(Constant.STANDBY_KEY_STORE_FILE, backupPath);
        }

        CryptoAPI.setJniLogger(logger);
        CryptoAPI.setLogLevel(ILogger.LogLevel.DEBUG);
        // codeCheck修改 很low 但是有用
        String debugInfo = String.format(Locale.ROOT, "tempProperties-1: %d", kmcProps.size());
        logger.debug(debugInfo);
        getInstance().initialize(kmcProps);
        log.info("Start init KMC.");
        int count = getInstance().getMkCount();
        for (int i = 0; i < count; i++) {
            KmcMkInfo kmcMkInfo = getInstance().getMkInfo(i);
            logger.error("KMC Key: " + kmcMkInfo.getKeyId());
            logger.error("KMC Key: " + kmcMkInfo.getKeyType());
            logger.error("KMC Key: " + kmcMkInfo.getMkCreateTime());
        }
        log.info("init KMC finished.");
    }
}
