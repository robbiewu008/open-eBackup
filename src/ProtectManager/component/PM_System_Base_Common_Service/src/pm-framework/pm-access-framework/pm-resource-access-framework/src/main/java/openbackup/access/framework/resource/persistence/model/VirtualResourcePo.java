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

import lombok.Data;
import lombok.extern.slf4j.Slf4j;

/**
 * VirtualResourcePo虚拟资源信息
 *
 */
@TableName("VIRTUAL_RESOURCE")
@Data
@Slf4j
public class VirtualResourcePo {
    /**
     * 虚拟机UUID
     */
    @TableId
    private String uuid;

    /**
     * 虚拟机IP字符串
     */
    private String vmIp;

    /**
     * 虚拟机环境IP
     */
    private String envIp;

    /**
     * 虚拟机连接状态
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
     * 虚拟机uncommitted
     */
    private String uncommitted;

    /**
     * 虚拟机mo_id
     */
    private String moId;

    /**
     * 子资源
     */
    private String children;

    /**
     * 虚拟机是否是模板
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
     * 虚拟机instance Id
     */
    private String instanceId;

    /**
     * 虚拟机设置中引导选项中的固件信息，如"bios"
     */
    private String firmware;
}
