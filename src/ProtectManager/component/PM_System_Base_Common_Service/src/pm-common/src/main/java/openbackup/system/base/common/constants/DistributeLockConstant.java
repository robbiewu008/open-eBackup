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

import java.util.concurrent.TimeUnit;

/**
 * 功能描述
 *
 */
public interface DistributeLockConstant {
    /**
     * 启动时同步最新密钥文件的锁
     */
    String SECRET_SYNC_LOCK = "/kms/lock/SECRET_SYNC_LOCK";

    /**
     * 更新密钥文件的锁
     */
    String SECRET_UPDATE_LOCK = "/kms/lock/SECRET_UPDATE_LOCK";

    /**
     * 更新密钥生命周期的锁
     */
    String KEY_LIFETIME_UPDATE_LOCK = "/kms/lock/KEY_LIFETIME_UPDATE_LOCK";

    /**
     * 更新device secret的锁的前缀
     */
    String UPDATE_DEVICE_SECRET_LOCK_PREFIX = "/secret/devicesecret/update_lock_";

    /**
     * 获取更新device secret的锁的等待时间
     */
    long UPDATE_DEVICE_SECRET_LOCK_WAIT_TIME = 1L;

    /**
     * 获取更新device secret的锁的等待时间单位
     */
    TimeUnit UPDATE_DEVICE_SECRET_LOCK_WAIT_TIME_UNIT = TimeUnit.MINUTES;
}
