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
package openbackup.openstack.protection.access.dto;

import lombok.Getter;
import lombok.Setter;

/**
 *  磁盘信息
 *
 */
@Getter
@Setter
public class VolInfo {
    /**
     * 磁盘id
     */
    private String id;

    /**
     * 磁盘名称
     */
    private String name;

    /**
     * 盘类型：true-系统盘 false-数据盘
     */
    private String bootable;

    /**
     * 磁盘大小
     */
    private String size;

    /**
     * 是否共享盘
     */
    private String shareable;

    /**
     * 盘架构：arm/x86_64
     */
    private String architecture;

    /**
     * 是否为克隆卷
     */
    private String fullClone;

    /**
     * 卷类型
     */
    private String volumeType;

    /**
     * 挂载路径
     */
    private String device;
}
