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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.util.BeanTools;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

/**
 * App Resource
 *
 */
@Setter
@Getter
public class AppResource {
    /**
     * 资源ID
     */
    private String uuid;

    /**
     * 资源名称
     */
    private String name;

    /**
     * 资源类型
     */
    private String type;

    /**
     * 资源子类型
     */
    private String subType;

    /**
     * 父资源的uuid。如备份数据库，这个是数据库实例的uuid
     */
    private String parentUuid;

    /**
     * 父资源的名称
     */
    private String parentName;

    /**
     * 资源的扩展信息
     */
    private Map<String, String> extendInfo;

    /**
     * AppResource 转换为 ProtectedResource
     *
     * @return ProtectedResource
     */
    public ProtectedResource castToProtectedResource() {
        return BeanTools.copy(this, ProtectedResource::new);
    }
}
