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
package openbackup.data.access.client.sdk.api.framework.agent.dto.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 修改agent应用类型结果
 *
 */
@Getter
@Setter
public class AgentUpdatePluginTypeResult {
    /**
     * -1:未返回值；0：失败；1：成功；2：中间状态；8：异常状态；9：初始状态
     */
    private int modifyStatus = -1;

    /**
     * 错误label
     */
    private String error;

    /**
     * 日志级别
     */
    private String level;

    /**
     * 发生时间
     */
    private Long startTime;
}
