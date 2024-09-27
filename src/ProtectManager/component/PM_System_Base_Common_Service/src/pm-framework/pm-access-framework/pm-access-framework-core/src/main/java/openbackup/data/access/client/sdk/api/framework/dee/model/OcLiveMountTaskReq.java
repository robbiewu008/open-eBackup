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
package openbackup.data.access.client.sdk.api.framework.dee.model;

import lombok.Data;

/**
 * 共享路径恢复任务请求参数
 *
 * @author w00574036
 * @since 2024-04-19
 * @version [OceanCyber 300 1.2.0]
 */
@Data
public class OcLiveMountTaskReq {
    /**
     * 任务id
     */
    private String taskId;

    /**
     * 请求id
     */
    private String requestId;

    /**
     * 克隆共享保留时间/小时
     */
    private int fileSystemKeepTime;

    /**
     * 任务状态，success、failed
     */
    private String status;

    /**
     * 任务对应存储设备ID
     */
    private String deviceId;
}
