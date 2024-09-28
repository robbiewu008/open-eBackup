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
package openbackup.data.protection.access.provider.sdk.backup.v2;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.enums.AgentMountTypeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * 备份拦截器，通过该拦截器可以针对不同的资源在下发备份命名给数据保护引擎时
 * <li>对备份参数进行修改或扩展</li>
 * <li>备份拦截器和资源的子类型关联</li>
 *
 */
public interface BackupInterceptorProvider extends DataProtectionProvider<String> {
    /**
     * 应用备份拦截器，对备份请求进行拦截，对备份参数信息修改或扩展
     *
     * @param backupTask 备份任务参数对象{@link BackupTask}
     * @return 返回备份任务
     */
    BackupTask initialize(BackupTask backupTask);

    /**
     * 备份任务后置流程(可选)
     *
     * @param postBackupTask 后置备份任务
     */
    default void finalize(PostBackupTask postBackupTask) {
    }

    /**
     * 是否支持数据备份与日志备份并行备份
     *
     * @param resource 资源
     * @return 是否支持并行
     */
    default boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return false;
    }

    /**
     * 转换备份类型
     *
     * @param backupType 备份类型
     * @param resource 资源
     * @return 转换后的备份类型
     */
    default BackupTypeConstants transferBackupType(BackupTypeConstants backupType, ProtectedResource resource) {
        return backupType;
    }

    /**
     * 获取备份任务需要锁定的资源
     * <p>
     * 框架会锁定副本，对副本加读锁 </br>
     * 框架会锁定资源（日志为@log），对资源加写锁 </br>
     * 其余需要锁定的资源需要插件自行上报 </br>
     * </p>
     *
     * @param resource basic resource，由{@link ResourceService#getBasicResourceById(String)}查询出来
     * @return 需要锁定的资源列表
     */
    default List<LockResourceBo> getLockResources(ProtectedResource resource) {
        return Collections.emptyList();
    }

    /**
     * 备份所使用的挂载类型
     *
     * @param backupTask 任务对象
     * @return agent挂载类型
     */
    default Optional<AgentMountTypeEnum> getMountType(BackupTask backupTask) {
        return Optional.empty();
    }
}
