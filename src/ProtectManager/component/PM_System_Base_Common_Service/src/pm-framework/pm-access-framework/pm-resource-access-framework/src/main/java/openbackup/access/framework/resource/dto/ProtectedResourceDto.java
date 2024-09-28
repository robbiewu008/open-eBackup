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
package openbackup.access.framework.resource.dto;

import lombok.Data;

import java.util.Map;

/**
 * 受保护资源DTO对象
 *
 */
@Data
public class ProtectedResourceDto {
    /**
     * 资源UUID
     */
    private String uuid;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型（主类）
     */
    private String type;

    /**
     * 资源子类
     */
    private String subType;

    /**
     * 资源路径
     */
    private String path;

    /**
     * 创建时间
     */
    private String createdTime;

    /**
     * 父资源名称
     */
    private String parentName;

    /**
     * 父资源uuid
     */
    private String parentUuid;

    /**
     * 受保护环境uuid
     */
    private String rootUuid;

    /**
     * 受保护状态
     */
    private Integer protectionStatus;

    /**
     * 资源的来源
     */
    private String sourceType;

    /**
     * 资源所属的用户
     */
    private String userId;

    /**
     * 资源授权的用户名称
     */
    private String authorizedUser;

    /**
     * 资源的扩展属性
     */
    private Map<String, String> extendInfo;
}
