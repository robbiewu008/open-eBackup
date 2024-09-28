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
package openbackup.system.base.config;

import openbackup.system.base.common.utils.AppUtil;
import openbackup.system.base.common.utils.KMS4Token;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;

/**
 * 系统配置工具类，负责系统配置的加载和初始化
 *
 */
public class SystemConfig {
    private static final Logger LOGGER = LoggerFactory.getLogger(SystemConfig.class);

    private static final String CONF_FILE_NAME = "dpa.properties";

    private static final String KEY_STORE_FILE = "/opt/OceanProtect/protectmanager/cert/pm.store.p12";

    private static final String CONF_DIR_NAME = "conf";

    private static final String DPA_SERVER_PORT = "dpa.server.port";

    private static final String KEYSTORE_PASS_FILE = "ssl.keystore.password_file";

    private static final String KEY_ALIAS = "ssl.key.alias";

    private static final String PRIVATE_KEY_PASS_FILE = "ssl.privatekey.password_file";

    private static final int DEFAULT_PORT = 10443;

    private static final SystemConfig INSTANCE = new SystemConfig();

    private Properties conf;

    private SystemConfig() {
        try {
            init();
        } catch (IOException e) {
            throw AppUtil.fatal(e, 1, "Init dpa config file failed.", null);
        }
    }

    @ExterAttack
    private void init() throws IOException {
        File workDir = new File("");
        LOGGER.info(workDir.getCanonicalPath());
        File configFile = searchConfigFile(new File(workDir.getCanonicalPath() + File.separator + "conf"));
        if (configFile == null) {
            configFile =
                    searchConfigFile(
                            new File(
                                    workDir.getCanonicalPath()
                                            + File.separator
                                            + "BOOT-INF"
                                            + File.separator
                                            + "classes"
                                            + File.separator
                                            + "conf"));
            if (configFile == null) {
                // 非base的其他java服务也要用这个配置文件，依赖证书初始化时拷贝一份到共享nas
                configFile = searchConfigFile(new File(KEY_STORE_FILE));
                if (configFile == null) {
                    LOGGER.error("dpa config file is not exists.");
                    System.exit(0);
                }
            }
        }
        conf = new Properties();
        if (LOGGER.isInfoEnabled()) {
            LOGGER.info("dpa file path: " + configFile.getPath());
        }
        try (FileInputStream fileInputStream = new FileInputStream(configFile)) {
            conf.load(fileInputStream);
        } catch (IOException e) {
            throw e;
        }
    }

    private File searchConfigFile(File curDir) throws IOException {
        if (LOGGER.isInfoEnabled()) {
            LOGGER.info("searchConfigFile:" + curDir.getName());
        }
        if (curDir == null || !curDir.exists()) {
            return null;
        }

        // 搜索当前目录的子目录
        File[] files = curDir.listFiles();
        if (files != null && files.length > 0) {
            for (File file : files) {
                File configFile = getFile(file);
                if (configFile != null) {
                    return configFile;
                }
            }
        }

        // 搜索不到往上一级目录搜索
        return searchConfigFile(curDir.getParentFile());
    }

    private File getFile(File file) throws IOException {
        File configFile = null;
        if (LOGGER.isInfoEnabled()) {
            LOGGER.info("searchConfigFile:" + file.getName());
        }
        if (file.isDirectory() && CONF_DIR_NAME.equals(file.getName())) {
            configFile = new File(file.getCanonicalPath() + File.separator + CONF_FILE_NAME);
            if (configFile.exists()) {
                return configFile;
            }
        }
        return configFile;
    }

    public static SystemConfig getInstance() {
        return INSTANCE;
    }

    /**
     * 根据key获取对应的配置
     *
     * @param key 配置对应key
     * @return 返回可以对应的配置
     */
    public String getConfig(String key) {
        return conf.getProperty(key);
    }

    /**
     * 获取DPA Management Server的端口，默认为10443
     *
     * @return DPA Management Server的端口
     */
    public int getServerPort() {
        String strPort = conf.getProperty(DPA_SERVER_PORT, "" + DEFAULT_PORT);
        return Integer.valueOf(strPort);
    }

    public String getKeystoreFile() {
        return KEY_STORE_FILE;
    }

    /**
     * 从配置文件中获取证书库的密码
     *
     * @return 证书库的密码
     */
    public String getKeystorePass() {
        String filePath = conf.getProperty(KEYSTORE_PASS_FILE);
        return getPassword(filePath);
    }

    private String getPassword(String filePath) {
        File file = new File(filePath);
        if (!file.exists()) {
            return "";
        }
        char[] result = new char[(int) file.length()];
        try (FileReader fileReader = new FileReader(file)) {
            fileReader.read(result);
        } catch (IOException e) {
            throw AppUtil.fatal(e, 1, "Read keystore password file failed.", null);
        }
        String password = KMS4Token.getInstance().decryptText(String.valueOf(result));
        if (VerifyUtil.isEmpty(password)) {
            LOGGER.error("KMS decrpt password failed, return null.");
        }
        return password;
    }

    /**
     * 从配置文件中获取秘钥的别名
     *
     * @return 秘钥的别名
     */
    public String getKeyAlias() {
        return conf.getProperty(KEY_ALIAS);
    }

    /**
     * 从配置文件中获取私钥
     *
     * @return 私钥
     */
    public String getKeyPass() {
        String filePath = conf.getProperty(PRIVATE_KEY_PASS_FILE);
        return getPassword(filePath);
    }
}
