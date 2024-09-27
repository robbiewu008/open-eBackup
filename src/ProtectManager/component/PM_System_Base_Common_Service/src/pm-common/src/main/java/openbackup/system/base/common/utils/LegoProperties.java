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
package openbackup.system.base.common.utils;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.util.Locale;
import java.util.Properties;

/**
 * 用于系统配置参数读取
 * 默认读取SDK模块类路径的default.properties
 * 运行时，如果conf/lego.properties文件存在，则加载conf/lego.properties文件定义的属性
 * 两个文件属性重合时，以conf/lego.properties为准
 *
 * @author s90004407
 * @version [Lego V100R002C10, 2010-7-8]
 * @since 2019-11-01
 */
public class LegoProperties {
    /**
     * 转储文件
     */
    private static final String TRASFER_URL = "/app/alarm";

    /* * 系统默认参数配置文件 */
    private static final String DEFAULT_PROPERTIES = "default.properties";

    /* * 属性保存 */
    private static final Properties PROPS = new Properties();

    /* * 日志记录 */

    private static final Logger LOG = LoggerFactory.getLogger(LegoProperties.class);

    static {
        /*
         * 读取默认配置
         */
        InputStream is = null;
        try {
            ClassLoader clazzLoader = LegoProperties.class.getClassLoader();
            if (clazzLoader != null) {
                is = clazzLoader.getResourceAsStream(DEFAULT_PROPERTIES);
            }
            if (is != null) {
                PROPS.load(is);
            }
        } catch (IOException e) {
            LOG.error("read default.properties fail.", ExceptionUtil.getErrorMessage(e));
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    LOG.error("Close the input stream about:{}", DEFAULT_PROPERTIES, ExceptionUtil.getErrorMessage(e));
                }
            }
        }

        /*
         * 把配置信息写入系统变量
         */
        // 直接调用System.setProperties方法，会覆盖System原有配置，因此需要加上原有properties再set
        Properties sysProps = System.getProperties();
        sysProps.putAll(PROPS);
        System.setProperties(sysProps);
    }

    private LegoProperties() {}

    /**
     * 通过properties文件的key获取value值
     *
     * @param key          参数key
     * @param defaultValue 默认返回值，如果value为空，返回defaultValue
     * @return String [返回类型说明]
     */
    public static String getProperty(String key, String defaultValue) {
        String value = PROPS.getProperty(key);

        if (value == null || value.length() <= 0) {
            value = defaultValue;
        }

        return value;
    }

    /**
     * 通过properties文件的key获取value值
     *
     * @param key 参数key
     * @return int [返回类型说明]
     */
    public static int getInt(String key) {
        String value = PROPS.getProperty(key);

        if (value == null || value.length() <= 0) {
            return 0;
        }

        return NumberUtil.convertToInteger(value);
    }

    /**
     * 获取IC是否安装在与iemp集成的环境中
     *
     * @return boolean
     */
    public static boolean isInstalledIntegrated() {
        String isInIntegrated = getProperty("is.integrated.in.iemp", "false");
        return Boolean.valueOf(isInIntegrated);
    }

    /**
     * 转储文件路径配置
     *
     * @return String
     */
    public static String getStorageFilePath() {
        // 默认路径
        String defaultPath = TRASFER_URL + File.separator;
        String value = "";
        String osName = System.getProperty("os.name");
        if (osName == null) {
            return defaultPath;
        }
        String os = osName.toLowerCase(Locale.getDefault());
        if (os.startsWith("win")) {
            value = PROPS.getProperty("storage.file.windows.path");
            if (value == null || value.length() <= 0) {
                return defaultPath;
            }
            String replace = "\\" + File.separator;
            value = value.replaceAll("/", replace);
        } else if (os.startsWith("linux")) {
            value = PROPS.getProperty("storage.file.linux.path");
            if (value == null || value.length() <= 0) {
                return defaultPath;
            }
        } else {
            LOG.info("os type error");
        }

        // 统一在后缀加上文件分隔符
        if (!value.endsWith(File.separator)) {
            value = value + File.separator;
        }

        // 如果配置的目录不存在则返回默认路径
        File storageFilePath = new File(value);
        if (!storageFilePath.exists()) {
            return defaultPath;
        }
        return value;
    }
}
