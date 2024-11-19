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

import lombok.Data;
import lombok.extern.slf4j.Slf4j;

/**
 * VirtualResourceResponsePo
 *
 */
@Data
@Slf4j
public class VirtualResourceResponsePo {
    /**
     * 虚拟机UUID
     */
    private String uuid;

    /**
     * 虚拟机vm_ip
     */
    private String vmIp;

    /**
     * 虚拟机环境IP
     */
    private String envIp;

    /**
     * 连通性状态
     */
    private int linkStatus;

    /**
     * 虚拟机capacity
     */
    private int capacity;

    /**
     * 虚拟机freeSpace
     */
    private String freeSpace;

    /**
     * uncommitted信息
     */
    private String uncommitted;

    /**
     * 虚拟机mo_id
     */
    private String moId;

    /**
     * 子资源信息
     */
    private String children;

    /**
     * 是否是模板
     */
    private boolean isTemplate;

    /**
     * 虚拟机alias类型
     */
    private String aliasType;

    /**
     * 虚拟机alias值
     */
    private String aliasValue;

    /**
     * 虚拟机操作系统类型
     */
    private String osType;

    /**
     * 虚拟机标记信息
     */
    private String tags;

    /**
     * 虚拟机instanceId
     */
    private String instanceId;

    /**
     * 虚拟机名称
     */
    private String name;

    /**
     * 资源类型
     */
    private String type;

    /**
     * 用户ID
     */
    private String userId;

    /**
     * authorizedUser信息
     */
    private String authorizedUser;

    /**
     * 资源子类型
     */
    private String subType;

    /**
     * source类型
     */
    private String sourceType;

    /**
     * 虚拟机路径
     */
    private String path;

    /**
     * 父资源名称
     */
    private String parentName;

    /**
     * 父资源UUID
     */
    private String parentUuid;

    /**
     * 根节点UUID
     */
    private String rootUuid;

    /**
     * 子资源UUID
     */
    private String childrenUuids;

    /**
     * discriminator
     */
    private String discriminator;

    /**
     * 创建时间
     */
    private String createdTime;

    /**
     * 保护状态
     */
    private int protectionStatus;

    /**
     * 版本信息
     */
    private String version;

    /**
     * auth信息
     */
    private String auth;
}
