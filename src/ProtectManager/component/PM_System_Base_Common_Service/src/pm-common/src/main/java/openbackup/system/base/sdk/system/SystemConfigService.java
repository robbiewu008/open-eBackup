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
package openbackup.system.base.sdk.system;

import java.util.List;
import java.util.Map;

/**
 * SystemConfigService
 *
 */
public interface SystemConfigService {
    /**
     * 定时更新feign超时时间
     */
    void updateFeignConfig();

    /**
     * Gets value from config for specified key
     *
     * @param key key
     * @return value
     */
    String getConfigValue(String key);

    /**
     * add key to config
     *
     * @param key key
     * @param value value
     */
    void addConfig(String key, String value);

    /**
     * update value of specified key to config
     *
     * @param key key
     * @param value value
     */
    void updateConfig(String key, String value);

    /**
     * 插入或更新
     *
     * @param key key
     * @param value value
     */
    void upsertConfig(String key, String value);

    /**
     * 一次性获取多个config value
     *
     * @param keys keys
     * @return configmap
     */
    Map<String, String> getConfigValues(List<String> keys);

    /**
     * 获取boolean类型值，获取不到时默认返回false
     *
     * @param key 键
     * @return boolean类型值
     */
    boolean getBooleanConfigValue(String key);

    /**
     * 根据模糊的key模糊搜索对应的config对应的UUID进行删除
     *
     * @param fuzzKey key的模糊匹配
     * @param neededKeyList 需要保留的key值（非必填）
     */
    void removeConfigByFuzzKey(String fuzzKey, List<String> neededKeyList);

    /**
     * 根据模糊的key查找除所有对应的config KEY VALUE
     *
     * @param fuzzKey key的模糊匹配
     * @return configmap
     */
    Map<String, String> getConfigByFuzzKey(String fuzzKey);
}
