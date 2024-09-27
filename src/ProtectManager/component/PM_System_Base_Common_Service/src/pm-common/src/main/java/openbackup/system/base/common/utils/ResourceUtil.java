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

import openbackup.system.base.security.exterattack.ExterAttack;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.io.InputStream;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Properties;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 服务器端国际化工具类
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-11
 */
public class ResourceUtil {
    /**
     * 国际化信息
     */
    private static final Map<String, Map<String, String>> resMaps = new ConcurrentHashMap<>();

    private static final Logger LOG = LoggerFactory.getLogger(ResourceUtil.class);

    private static final ResourceUtil RESOURCE_UTIL = new ResourceUtil();

    /**
     * 实体单例
     *
     * @return ResourceUtil
     */
    public static ResourceUtil getInstance() {
        return RESOURCE_UTIL;
    }

    /**
     * 国际化服务器端
     *
     * @param key    key
     * @param locale language
     * @return String value
     */
    public String getText(String key, Locale locale) {
        if (VerifyUtil.isEmpty(key) || VerifyUtil.isEmpty(locale)) {
            LOG.error("internal error, the locale or key is null.");
            return null;
        }
        // 某些资源类型用空格隔开，properties文件用下划线替代空格
        String newKey = key.replace(' ', '_');
        internalFindString(locale);
        if (VerifyUtil.isEmpty(resMaps) || resMaps.get(locale.toString()) == null) {
            return newKey;
        }
        Map<String, String> textMap = resMaps.get(locale.toString());
        if (VerifyUtil.isEmpty(textMap.get(newKey))) {
            return newKey;
        }

        return textMap.get(newKey);
    }

    @ExterAttack
    private synchronized void internalFindString(Locale locale) {
        if (resMaps.get(locale.toString()) != null) {
            return;
        }
        resMaps.put(locale.toString(), new HashMap<>());
        InputStream inputStream = null;
        try {
            String language = "en".equalsIgnoreCase(locale.toString()) ? "en" : "zh";
            inputStream = ResourceUtil.class.getClassLoader().getResourceAsStream("i18n/" + language + ".properties");
            if (inputStream == null) {
                return;
            }
            Properties properties = new Properties();
            properties.load(inputStream);
            Map<String, String> valueMap = resMaps.get(locale.toString());
            for (Entry<Object, Object> resource : properties.entrySet()) {
                Object key = resource.getKey();
                Object value = resource.getValue();
                valueMap.put(key != null ? key.toString() : null, value != null ? value.toString() : null);
            }
        } catch (IOException e) {
            LOG.error("IOException read i18n file error.");
        } finally {
            CommonUtil.close(inputStream);
        }
    }
}
