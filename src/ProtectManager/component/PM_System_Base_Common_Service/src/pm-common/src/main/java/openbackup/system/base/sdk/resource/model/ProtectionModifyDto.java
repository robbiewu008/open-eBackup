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
package openbackup.system.base.sdk.resource.model;

import com.fasterxml.jackson.databind.PropertyNamingStrategies;
import com.fasterxml.jackson.databind.annotation.JsonNaming;

import lombok.Getter;
import lombok.Setter;

import java.util.List;
import java.util.Map;

/**
 * 修改保护请求体
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-15
 */
@Getter
@Setter
@JsonNaming(PropertyNamingStrategies.SnakeCaseStrategy.class)
public class ProtectionModifyDto {
    /**
     * SLA id
     */
    private String slaId;

    /**
     * 资源id
     */
    private String resourceId;

    /**
     * 是否为资源组
     */
    private Boolean isResourceGroup;

    /**
     * 是否为资源组子资源
     */
    private Boolean isGroupSubResource;

    /**
     * 资源组id
     */
    private String resourceGroupId;

    /**
     * 要删除的扩展属性 key
     */
    private List<String> deleteKeys;

    /**
     * 扩展属性
     */
    private Map<String, Object> extParameters;
}
