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
package openbackup.system.base.common.constants;

/**
 * 资源相关错误码
 *
 */
public final class ResourceErrorCodeConstant {
    /**
     * Failed to get virtual machine disk information.
     */
    public static final long VIRTUAL_MACHINE_DISK_INFO_IS_EMPTY = 1677931410L;

    /**
     * Fail to protect hyper-v resource, because not to support shared disk.
     */
    public static final long NOT_SUPPORT_SHARED_DISK = 1677932051L;

    /**
     * Fail to protect hyper-v resource, because not to support physical hard disk.
     */
    public static final long NOT_SUPPORT_PHYSICAL_HARD_DISK = 1677932058L;

    /**
     * Fail to protect hyper-v resource, because not to support vhdx set disk.
     */
    public static final long NOT_SUPPORT_VHD_SET_DISK = 1677932052L;

    private ResourceErrorCodeConstant() {
    }
}
