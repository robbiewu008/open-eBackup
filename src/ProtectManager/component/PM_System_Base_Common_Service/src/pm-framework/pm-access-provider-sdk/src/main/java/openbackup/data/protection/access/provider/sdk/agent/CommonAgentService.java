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
package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;

import java.util.List;

/**
 * agent服务
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/9/15
 */
public interface CommonAgentService {
    /**
     * 填充下发agent通用信息
     *
     * @param endpoints agent信息
     */
    void supplyAgentCommonInfo(List<Endpoint> endpoints);

    /**
     * 默认的获取agent挂载类型方法
     *
     * @param jobId 任务ID
     * @return 挂载方法
     */
    AgentMountTypeEnum getJobAgentMountTypeByJob(String jobId);


    /**
     * 默认的获取agent挂载类型方法
     *
     * @param unitId 存储单元ID
     * @return 挂载方法
     */
    AgentMountTypeEnum getJobAgentMountTypeByUnitId(String unitId);
}
