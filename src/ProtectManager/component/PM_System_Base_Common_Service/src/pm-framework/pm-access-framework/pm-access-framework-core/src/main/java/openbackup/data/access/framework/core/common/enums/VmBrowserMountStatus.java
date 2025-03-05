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
package openbackup.data.access.framework.core.common.enums;

import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 浏览挂载状态
 *
 */
public enum VmBrowserMountStatus {
    /**
     * 未挂载
     */
    UMOUNT("Umount"),

    /**
     * 挂载中
     */
    MOUNTING("Mounting"),

    /**
     * 已挂载
     */
    MOUNTED("Mounted"),

    /**
     * 挂载失败
     */
    MOUNT_FAIL("Mount_fail"),

    /**
     * 卸载中
     */
    MOUNT_DELETING("Mount_deleting"),

    /**
     * 卸载失败
     */
    MOUNT_DELETE_FAIL("Mount_delete_fail"),

    /**
     * 不支持挂载
     */
    UNSUPPORTED("Unsupported");

    private final String status;

    VmBrowserMountStatus(String status) {
        this.status = status;
    }

    /**
     * 获取副本挂载状态
     *
     * @return string
     */
    @JsonValue
    public String getBrowserMountStatus() {
        return status;
    }
}
