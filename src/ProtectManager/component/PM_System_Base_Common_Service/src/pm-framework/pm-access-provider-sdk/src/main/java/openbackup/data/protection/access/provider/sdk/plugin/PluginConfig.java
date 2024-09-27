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

import com.fasterxml.jackson.databind.JsonNode;

import lombok.Data;

import java.util.Map;

/**
 * 插件配置类
 *
 * @since 2022-05-20
 */
@Data
public class PluginConfig {
    private String type;

    private String subType;

    /**
     * 配置Map，通用结构
     * key：顶层key
     * value: 对应值
     */
    private Map<String, JsonNode> configMap;
}
