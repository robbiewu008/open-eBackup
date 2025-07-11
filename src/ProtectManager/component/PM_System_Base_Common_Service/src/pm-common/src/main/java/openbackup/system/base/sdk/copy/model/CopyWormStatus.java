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
package openbackup.system.base.sdk.copy.model;

import openbackup.system.base.util.EnumUtil;

/**
 * 副本Worm状态
 *
 */
public enum CopyWormStatus {
    /**
     * 普通副本
     */
    UNSET(1),
    /**
     * 未设置WORM
     */
    SETTING(2),
    /**
     * WORM设置成功
     */
    SET_SUCCESS(3),
    /**
     * WORM设置失败
     */
    SET_FAILED(4),

    /**
     * WORM已过期
     */
    SET_EXPIRED(5);

    private final int status;

    CopyWormStatus(int status) {
        this.status = status;
    }

    /**
     * 获取status
     *
     * @return status
     */
    public int getStatus() {
        return status;
    }

    /**
     * 获取枚举类
     *
     * @param status status
     * @return CopyWormStatus
     */
    public static CopyWormStatus get(int status) {
        return EnumUtil.get(CopyWormStatus.class, CopyWormStatus::getStatus, status, false);
    }
}
