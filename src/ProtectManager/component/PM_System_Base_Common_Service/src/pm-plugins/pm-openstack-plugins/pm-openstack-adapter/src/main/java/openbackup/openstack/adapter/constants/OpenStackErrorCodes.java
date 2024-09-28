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
package openbackup.openstack.adapter.constants;

/**
 * OpenStack北向接口错误码
 *
 */
public final class OpenStackErrorCodes {
    /**
     * 创建备份配额小于已占用配额
     */
    public static final int INIT_QUOTA_LESS_THAN_USED = 403;

    /**
     * 查不到备份、恢复任务或副本信息
     */
    public static final int NOT_FOUND = 404;

    private OpenStackErrorCodes() {}
}
