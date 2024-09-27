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
package openbackup.data.access.framework.copy.index.dto;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

/**
 * 删除副本索引的响应
 *
 * @author g30003063
 * @since 2021-12-16
 */
@Getter
@Setter
public class CopyIndexDeleteResponseDto {
    @JsonProperty("request_id")
    private String requestId;

    @JsonProperty("user_id")
    private String userId;

    @JsonProperty("resource_id")
    private String resourceId;

    @JsonProperty("copy_id")
    private String copyId;

    @JsonProperty("status")
    private String status;

    @JsonProperty("error_code")
    private String errorCode;

    @JsonProperty("error_desc")
    private String errorDesc;
}
