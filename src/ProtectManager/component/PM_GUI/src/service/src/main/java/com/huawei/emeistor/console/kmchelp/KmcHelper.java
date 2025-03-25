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
package com.huawei.emeistor.console.kmchelp;

import com.huawei.emeistor.console.exception.LegoCheckedException;
import com.huawei.emeistor.console.kmchelp.kmc.KmcInstance;
import com.huawei.emeistor.console.util.ExceptionUtil;
import com.huawei.kmc.common.AppException;
import com.huawei.kmc.common.Constant;
import com.huawei.kmc.common.InitStage;
import com.huawei.kmc.crypt.CryptoAPI;

import lombok.extern.slf4j.Slf4j;

import java.io.File;
import java.io.IOException;
import java.nio.charset.StandardCharsets;

/**
 * <提供wcc加解密方法>
 * <功能详细描述>
 *
 * @see [相关类/方法]
 */
@Slf4j
public class KmcHelper {
    private static volatile KmcHelper instance = null;

    // 锁，保证同一时刻只有一个更新操作
    private static final Object UPDATING_LOCKER = new Object();

    /*
     * 私有构造方法
     */
    private KmcHelper() {
    }

    /**
     * 获取加密工具类实例
     *
     * @return 实例
     */
    public static KmcHelper getInstance() {
        if (instance == null) {
            synchronized (UPDATING_LOCKER) {
                if (instance == null) {
                    instance = new KmcHelper();
                    setInstanceWccConfig();
                }
            }
        }
        return instance;
    }

    private static void setInstanceWccConfig() {
        try {
            instance.setWccConfig();
        } catch (Exception e) {
            instance = null;
            throw new LegoCheckedException("setWccConfig exception, {}", ExceptionUtil.getErrorMessage(e));
        }
    }

    /*
     * 配置wcc环境变量
     */
    private void setWccConfig() throws IOException {
        String wccConfPath = getConfigPath();
        if (wccConfPath == null) {
            return;
        }
        try {
            KmcInstance.initComponent(wccConfPath, "", false, true);
        } catch (Exception e) {
            log.info("setWccConfig: {}:", ExceptionUtil.getErrorMessage(e));
        }
    }

    /*
     * 重置wcc环境变量
     */
    private void resetWccConfig() {
        try {
            KmcInstance.releaseComponent();
        } catch (Exception e) {
            log.error("reset wcc config exception,{}", ExceptionUtil.getErrorMessage(e));
        }
    }

    /**
     * 获取密钥配置路径
     *
     * @return 密钥配置路径
     * @throws IOException IOException
     */
    public static String getConfigPath() throws IOException {
        String currPath = (new File("")).getCanonicalPath();
        // 默认当前目录就是LegoRuntime
        return currPath.isEmpty()
            ? System.getProperty("beetle.application.home.path")
            : currPath + File.separator + "conf" + File.separator + "wcc";
    }

    /**
     * decrypt
     * <参数说明及返回类型说明>
     *
     * @param txt 需要解密的字符串
     * @return String 解密后字符串
     * @see [类、类#方法、类#成员]
     */
    public String decrypt(String txt) {
        if (txt == null || txt.isEmpty()) {
            return txt;
        }
        String decryptCtxt = null;
        try {
            decryptCtxt = new String(KmcInstance.getInstance().decrypt(txt.getBytes(StandardCharsets.UTF_8)),
                StandardCharsets.UTF_8);
        } catch (Exception e) {
            log.error("kmc result fail, {}", ExceptionUtil.getErrorMessage(e));
            // 重新加载证书,再重新解密
            initKmc();
            try {
                // 再次解密失败，则抛出解密失败
                decryptCtxt = new String(KmcInstance.getInstance().decrypt(txt.getBytes(StandardCharsets.UTF_8)),
                    StandardCharsets.UTF_8);
            } catch (AppException ex) {
                throw new LegoCheckedException("kmc result fail, {}", ExceptionUtil.getErrorMessage(e));
            }
        }
        return decryptCtxt;
    }
    private void initKmc() {
        try {
            // 如果已经初始化，且密钥文件存在，则先解除初始化，如果密钥文件不存在则不重新初始化
            boolean isInitDone = InitStage.INIT_KMC_DONE.getValue() == CryptoAPI.getInitStage();
            if (isInitDone) {
                KmcInstance.releaseComponent();
                log.info("reInit kmc component.");
            }
            KmcInstance.initComponent(KmcHelper.getConfigPath(), "", false, true);
            log.info("init kmc component complete.");
        } catch (Exception ex) {
            log.error("Init KMC Error:", ex);
            throw new LegoCheckedException("Init KMC Error:", ex);
        }
    }

    /**
     * encrypt
     * <参数说明及返回类型说明>
     *
     * @param txt 需要加密的字符串
     * @return String 加密后的结果
     * @see [类、类#方法、类#成员]
     */
    public String encrypt(String txt) {
        if (txt == null || txt.isEmpty()) {
            return txt;
        }

        String encryptCtxt = null;
        try {
            encryptCtxt = new String(KmcInstance.getInstance().encrypt(txt.getBytes(StandardCharsets.UTF_8)),
                StandardCharsets.UTF_8);
        } catch (Exception e) {
            log.error("kmc generate fail, {}", ExceptionUtil.getErrorMessage(e));
            // 重新加载证书,再重新解密
            initKmc();
            try {
                // 再次加密失败，则抛出解密失败
                encryptCtxt = new String(KmcInstance.getInstance().encrypt(txt.getBytes(StandardCharsets.UTF_8)),
                    StandardCharsets.UTF_8);
            } catch (AppException ex) {
                throw new LegoCheckedException("kmc generate fail, {}", ExceptionUtil.getErrorMessage(e));
            }
        }
        return encryptCtxt;
    }

    /**
     * 更新密钥文件
     *
     * @param lifeTimeDays 密钥文件有效期
     */
    public void checkAndUpdateMk(int lifeTimeDays) {
        try {
            KmcInstance.getInstance().checkAndUpdateMk(Constant.DEFAULT_DOMAIN, lifeTimeDays);
        } catch (Exception e) {
            log.error("check and update mk exception, {}", ExceptionUtil.getErrorMessage(e));
        }
    }
}
