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
import openbackup.system.base.query.PageQueryConfig;

import java.sql.Timestamp;

/**
 * ResourceGroup Po
 *
 */
@TableName("T_RESOURCE_GROUP")
@Getter
@Setter
@PageQueryConfig(
        conditions = {"path", "source_sub_type", "%name%", "%uuid%", "protection_status"},
        orders = {"created_time"})
public class ResourceGroupPo {
    /**
     * 资源组UUID
     */
    @TableId
    private String uuid;

    /**
     * 资源组名称
     */
    private String name;

    /**
     * 资源路径
     */
    private String path;

    /**
     * 资源类别
     */
    private String sourceType;

    /**
     * 资源子类
     */
    private String sourceSubType;

    /**
     * 创建时间
     */
    private Timestamp createdTime;

    /**
     * 扩展信息
     */
    private String extendStr;

    /**
     * 资源组所属的用户
     */
    private String userId;

    /**
     * 资源组保护状态
     */
    private Integer protectionStatus;

    /**
     * 资源组所属范围资源uuid
     */
    private String scopeResourceId;

    /**
     * 资源组类型：manual-手动选择，rule-按规则过滤
     */
    private String groupType;
}