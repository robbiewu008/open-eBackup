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
package openbackup.system.base.util;

import com.google.common.collect.ImmutableMap;

import io.jsonwebtoken.lang.Strings;
import lombok.extern.slf4j.Slf4j;
import openbackup.system.base.common.utils.ExceptionUtil;

import org.apache.commons.io.FileUtils;

import java.io.File;
import java.io.IOException;
import java.util.Map;

/**
 * ConfigMapUtil工具类实现
 *
 */
@Slf4j
public class ConfigMapUtil {
    private static final Map<String, String> CONFIG_MAP = ImmutableMap.of("multicluster-conf",
        "/opt/multicluster_conf");

    /**
     * 获取configMap的值
     *
     * @param configMapKey configMapKey
     * @param key key值
     * @return value
     */
    public static String getValueInConfigMap(String configMapKey, String key) {
        String path = CONFIG_MAP.getOrDefault(configMapKey, Strings.EMPTY);
        String fullPath = path + "/" + key;
        File file = new File(fullPath);
        if (!file.exists()) {
            log.warn("Get value in config map file does not exits, fullPath: {}.", fullPath);
            return Strings.EMPTY;
        }
        try {
            return FileUtils.readFileToString(file);
        } catch (IOException e) {
            log.error("Get file failed", ExceptionUtil.getErrorMessage(e));
            return Strings.EMPTY;
        }
    }
}
