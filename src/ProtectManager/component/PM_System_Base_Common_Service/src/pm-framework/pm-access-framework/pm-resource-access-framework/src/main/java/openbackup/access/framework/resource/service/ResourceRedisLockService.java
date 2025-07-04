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
package openbackup.access.framework.resource.service;

import openbackup.system.base.sdk.resource.model.ResourceLockEntity;

import java.util.List;

/**
 * 资源Redis冗余锁
 *
 */
public interface ResourceRedisLockService {
    /**
     * 获取redis锁
     *
     * @param lockId lockId
     * @param resources resources
     * @param expireTime 锁过期时间
     * @return 是否获取到锁
     */
    boolean acquireLock(String lockId, List<ResourceLockEntity> resources, long expireTime);

    /**
     * 获取redis锁。默认超时时间3s
     *
     * @param lockId lockId
     * @param resources resources
     * @return 是否获取到锁
     */
    boolean acquireLock(String lockId, List<ResourceLockEntity> resources);

    /**
     * 解除redis锁。
     *
     * @param lockId lockId
     * @return 是否解锁成功
     */
    boolean unlock(String lockId);

    /**
     * 获取redis锁。并在多集群场景下同步给其他节点
     *
     * @param resources resources
     * @param lockId lockId
     * @return 是否获取到锁
     */
    boolean lockAndMultiClusterSync(String lockId, List<ResourceLockEntity> resources);

    /**
     * 解除redis锁。并在多集群场景下同步给其他节点
     *
     * @param lockId lockId
     * @return 是否解锁成功
     */
    boolean unlockAndMultiClusterSync(String lockId);
}
