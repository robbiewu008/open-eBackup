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
package openbackup.system.base.sdk.agent.model;

import lombok.Data;

import java.util.List;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-08-31
 */
@Data
public class AgentUpdateResultResponse {
    /**
     * -1:未返回值；0：失败；1：成功；2：中间状态；8：异常状态；9：初始状态
     */
    private int upgradeStatus = -1;

    /**
     * 任务日志级别
     */
    private String level;

    /**
     * 任务日志详情
     */
    private String logDetail;

    /**
     * 任务日志详情参数
     */
    private List<String> logDetailParam;

    /**
     * 任务信息
     */
    private String logInfo;

    /**
     * 任务信息参数
     */
    private List<String> logInfoParam;

    /**
     * 发生时间
     */
    private Long startTime;

    /**
     * 当前运行的agent版本
     */
    private String version;
}
