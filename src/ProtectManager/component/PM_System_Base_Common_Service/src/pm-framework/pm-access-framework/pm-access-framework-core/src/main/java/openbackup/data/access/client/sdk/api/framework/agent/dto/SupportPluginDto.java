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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

import java.util.List;

/**
 * Agent返回的插件数据模型，包括插件名字、插件版本以及插件支持的应用
 *
 */
@Data
public class SupportPluginDto {
    /**
     * 插件名称
     */
    private String pluginName;

    /**
     * 插件版本号
     */
    private String pluginVersion;

    /**
     * 支持应用的版本信息
     */
    private List<SupportApplicationDto> supportApplications;
}
