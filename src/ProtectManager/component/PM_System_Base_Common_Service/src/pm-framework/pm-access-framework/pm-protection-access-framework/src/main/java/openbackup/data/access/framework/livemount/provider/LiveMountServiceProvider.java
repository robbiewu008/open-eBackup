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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.client.sdk.api.framework.dme.DmeBackupClone;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

/**
 * 功能描述: 即时挂载流程扩展接口
 *
 */
public interface LiveMountServiceProvider extends DataProtectionProvider<String> {
    /**
     * 构造统一备份框架访问DME克隆副本接口的请求参数
     *
     * @param cloneCopyParam 副本克隆信息
     * @return DME副本克隆请求参数
     */
    DmeBackupClone buildDmeCloneCopyRequest(CloneCopyParam cloneCopyParam);

    /**
     * 校验源副本是否能被挂载
     * 如果副本无法被挂载，且当前流程是手动触发的，需要抛出异常，而不是返回false
     *
     * @param copy 源副本
     * @param isManual 是否手动触发的挂载
     * @return true: 可以挂载(流程继续); false: 不能挂载(流程正常终止，不会报错)
     */
    boolean isSourceCopyCanBeMounted(Copy copy, boolean isManual);

    /**
     * 构造LiveMount实体类
     *
     * @param liveMountObject liveMount对象
     * @param sourceResourceEntity 源资源实体类
     * @param targetResourceEntity 目标资源实体类
     * @return LiveMount实体类
     */
    LiveMountEntity buildLiveMountEntity(LiveMountObject liveMountObject, ResourceEntity sourceResourceEntity,
        ResourceEntity targetResourceEntity);

    /**
     * 即时挂载完结处理
     *
     * @param liveMountEntity LiveMountEntity
     */
    void processLiveMountTerminate(LiveMountEntity liveMountEntity);

    /**
     * 即时挂载是否支持日志副本
     *
     * @return 是否支持
     */
    default boolean isSupportLogCopy() {
        return true;
    }
}