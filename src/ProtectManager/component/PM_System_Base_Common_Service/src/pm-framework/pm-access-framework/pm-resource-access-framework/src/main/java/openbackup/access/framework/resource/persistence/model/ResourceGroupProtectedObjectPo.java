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
package openbackup.access.framework.resource.persistence.model;

import com.baomidou.mybatisplus.annotation.TableId;
import com.baomidou.mybatisplus.annotation.TableName;

import lombok.Getter;
import lombok.Setter;

/**
 * ResourceGroup Member Po
 *
 * @author c00631681
 * @since 2024-1-18
 */
@TableName("T_RESOURCE_GROUP_MEMBER")
@Getter
@Setter
public class ResourceGroupProtectedObjectPo {
    /**
     * UUID
     */
    @TableId
    private String uuid;

    /**
     * 资源ID
     */
    private String sourceId;

    /**
     * 资源子类
     */
    private String sourceSubType;

    /**
     * 资源组id
     */
    private String resourceGroupId;
}