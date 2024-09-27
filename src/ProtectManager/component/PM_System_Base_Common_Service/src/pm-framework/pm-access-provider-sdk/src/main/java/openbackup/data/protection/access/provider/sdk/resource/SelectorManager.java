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
package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;

import java.util.List;
import java.util.Map;

/**
 * 选择器管理器
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-29
 */
public interface SelectorManager {
    /**
     * 根据资源信息选择agent
     *
     * @param resource 资源
     * @param jobType 任务类型
     * @param parameters 扩展参数
     * @return 当前任务所需的agent
     */
    List<Endpoint> selectAgentByResource(ProtectedResource resource, String jobType, Map<String, String> parameters);
}
