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
package openbackup.data.protection.access.provider.sdk.plugin;

import java.util.List;
import java.util.Optional;

/**
 * 插件配置管理
 *
 * @since 2022-05-20
 */
public interface PluginConfigManager {
    /**
     * 初始化
     */
    void init();

    /**
     * 获取配置
     *
     * @return 配置
     */
    List<PluginConfig> getPluginConfigs();

    /**
     * 根据subtype获取配置
     *
     * @param subType subType
     * @return 配置
     */
    Optional<PluginConfig> getPluginConfig(String subType);
}
