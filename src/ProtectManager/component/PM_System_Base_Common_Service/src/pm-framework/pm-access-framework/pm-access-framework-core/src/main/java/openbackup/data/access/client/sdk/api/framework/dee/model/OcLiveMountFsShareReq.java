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

import java.util.List;

/**
 * 一体机共享路径恢复创建共享请求
 *
 * @author w00574036
 * @since 2024-04-19
 * @version [OceanCyber 300 1.2.0]
 */
@Data
public class OcLiveMountFsShareReq {
    /**
     * 请求id
     */
    private String requestId;

    /**
     * 任务id
     */
    private String taskId;

    /**
     * nfs及cifs共享信息
     */
    private List<OcLiveMountFsShareInfo> liveMountFsShareInfos;
}
