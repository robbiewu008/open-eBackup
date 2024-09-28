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
package openbackup.system.base.sdk.cluster.model.ha;

import lombok.Data;

import javax.validation.constraints.NotNull;
import javax.validation.constraints.Pattern;

/**
 * HA后置任务请求体
 *
 */
@Data
public class PostHaRequest {
    /**
     * 操作类型，add、modify、remove
     */
    @NotNull(message = "The type cannot be null. ")
    @Pattern(regexp = "add|modify|remove")
    private String type;

    /**
     * 任务结果，success：成功，fail：失败
     */
    @NotNull(message = "The result cannot be null. ")
    @Pattern(regexp = "success|fail")
    private String result;

    /**
     * 节点角色，PRIMARY：主节点，STANDBY：从节点
     */
    @NotNull(message = "The role cannot be null. ")
    @Pattern(regexp = "primary|standby")
    private String role;
}
