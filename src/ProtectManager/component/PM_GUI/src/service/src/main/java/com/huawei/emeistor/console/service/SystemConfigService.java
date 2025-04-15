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
package com.huawei.emeistor.console.service;

import java.util.Map;

/**
 * 系统配置service
 *
 */
public interface SystemConfigService {
    /**
     * 保存或更新配置信息，该方法整体有事务
     *
     * @param configMap 配置信息
     */
    void saveOrUpdateConfig(Map<String, String> configMap);

    /**
     * 查询配置信息
     *
     * @param key key
     * @param isNeedDecrypt 是否需要解密
     * @return value
     */
    String queryConfig(String key, boolean isNeedDecrypt);
}
