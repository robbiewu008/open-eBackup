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

import com.huawei.kmc.common.ILogger;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;

/**
 * 文 件 名: SysPropertiesUtils
 * 包 名: com.huawei.cbh.modules.console.util
 * 描 述: 加载配置文件参数
 *
 */
public class SysPropertiesUtils {
    /**
     * 默认配置参数
     */
    public static final String DEFAULT_FILE_NAME = "kmc.properties";

    private static final ILogger log = new CryptLogger();

    /**
     * 从kmc.properties文件中读取默认配置。
     *
     * @param path 当前路径
     * @return Properties
     */
    public static Properties getPropertiesFromFile(String path) {
        Properties tempProperties = new Properties();

        try (FileReader fileReader = new FileReader(path + File.separator + DEFAULT_FILE_NAME);
            BufferedReader bufferedReader = new BufferedReader(fileReader)
        ) {
            tempProperties.load(bufferedReader);
        } catch (FileNotFoundException e) {
            log.error("file not found exception");
        } catch (IOException e) {
            log.error("load default properties file exception");
        }

        return tempProperties;
    }
}
