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
package com.huawei.oceanprotect.system.base.initialize.backstorage.enums;

import lombok.ToString;

/**
 * 初始化备份错误码
 *
 * @author w00493811
 * @since 2020-12-28
 */
@ToString
public enum InitBackActionResultCode {
    /**
     * 成功
     */
    SUCCESS(0),
    /**
     * 失败
     */
    FAILURE(1),
    /**
     * 设置存储池硬盘域参数SSD_DISK_NUM失败
     */
    ERROR_SET_DISK_POOL_SSD_DISK_NUM_FAILED(10000),
    /**
     * 设置存储池扩展信息失败
     */
    ERROR_SET_STORAGE_POOL_EXPAND_INFO_FAILED(10010),
    /**
     * 增加文件系统信息失败
     */
    ERROR_ADD_FILESYSTEMS_FAILED(40000),
    /**
     * 增加文件系统信息失败
     */
    ERROR_ADD_NFS_SHARE_FAILED(40010),
    /**
     * 增加文件系统信息失败
     */
    ERROR_ADD_NFS_CLIENT_FAILED(40020),
    /**
     * 初始化卷信息（爱数）失败
     */
    ERROR_INIT_VOLUMES_FAILED(40030),
    /**
     * 获取存储池信息失败
     */
    ERROR_GET_STORAGE_POOLS_FAILED(80000),
    /**
     * 获取存储池扩展信息失败
     */
    ERROR_GET_CONTROLLERS_FAILED(80010),
    /**
     * 获取文件系统信息失败
     */
    ERROR_GET_FILESYSTEMS_FAILED(80020),
    /**
     * 获取NFS客户端失败
     */
    ERROR_GET_NFS_CLIENTS_FAILED(80030),

    /**
     * 获取NET_PLANE_IPS失败
     */
    ERROR_GET_NET_PLANE_IPS_FAILED(80040);

    /**
     * 编码
     */
    private int code;

    /**
     * 带参数初始化函数
     *
     * @param theCode 编码
     */
    InitBackActionResultCode(int theCode) {
        code = theCode;
    }

    /**
     * 是否OK
     *
     * @return 是否OK
     */
    public boolean isOkay() {
        return this == SUCCESS;
    }
}