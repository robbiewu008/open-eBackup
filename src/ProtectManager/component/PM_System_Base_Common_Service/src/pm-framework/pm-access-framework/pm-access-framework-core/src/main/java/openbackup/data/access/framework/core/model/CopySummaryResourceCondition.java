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
package openbackup.data.access.framework.core.model;

import com.fasterxml.jackson.annotation.JsonIgnore;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 副本资源查询条件
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-03-02
 */
@Getter
@Setter
public class CopySummaryResourceCondition {
    /**
     * 资源名称，模糊匹配
     */
    private String resourceName;

    /**
     * 资源位置，模糊匹配
     */
    private String resourceLocation;

    /**
     * 复制副本SLA名称，模糊匹配
     */
    private String protectedSlaName;

    /**
     * 资源子类型
     */
    private List<String> resourceSubType;

    /**
     * 资源状态
     */
    private List<String> resourceStatus;

    /**
     * 保护状态
     */
    private List<Boolean> protectedStatus;

    /**
     * 当前登录用户
     */
    @JsonIgnore
    private String userId;
}
