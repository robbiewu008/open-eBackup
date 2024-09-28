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
package openbackup.system.base.sdk.restore.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 内部接口调用恢复接口参数
 *
 */
@Data
public class RestoreRequestInternal {
    /**
     * user_id 用户ID
     */
    @JsonProperty("user_id")
    private String userId;

    /**
     * restore_req_string 前端下发的恢复任务参数字符串
     */
    @JsonProperty("restore_req_string")
    private String requestReq;
}
