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

import openbackup.data.access.framework.livemount.common.model.LiveMountCloneRequest;
import openbackup.data.access.framework.livemount.common.model.LiveMountCreateCheckParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountExecuteParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountMigrateParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountRefreshParam;
import openbackup.data.access.framework.livemount.common.model.LiveMountUnmountParam;
import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.copy.CopyInfoBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;

import java.util.List;

/**
 * Live Mount Provider
 *
 * @author l00272247
 * @since 2020-09-17
 */
public interface LiveMountProvider extends DataProtectionProvider<String> {
    /**
     * create live mount pre-check
     *
     * @param liveMountCreateCheckParam live mount create check param
     */
    void createLiveMountPreCheck(LiveMountCreateCheckParam liveMountCreateCheckParam);

    /**
     * execute live mount
     *
     * @param liveMountExecuteParam live mount execute param
     */
    void executeLiveMount(LiveMountExecuteParam liveMountExecuteParam);

    /**
     * unmount live mount
     *
     * @param unmountParam unmount param
     */
    void unmountLiveMount(LiveMountUnmountParam unmountParam);

    /**
     * clone copy
     *
     * @param request request
     * @return clone copy info
     */
    CopyInfoBo cloneCopy(LiveMountCloneRequest request);

    /**
     * update performance setting
     *
     * @param liveMountEntity live mount entity
     */
    void updatePerformanceSetting(LiveMountEntity liveMountEntity);

    /**
     * refresh target resource
     *
     * @param liveMountRefreshParam live mount refresh param
     * @return resource uuid
     */
    List<String> refreshTargetResource(LiveMountRefreshParam liveMountRefreshParam);

    /**
     * migrate live mount
     *
     * @param migrateParam live mount migrate param
     */
    void migrateLiveMount(LiveMountMigrateParam migrateParam);

    /**
     * 添加live mount文件系统名称用于更新
     *
     * @param liveMountEntity liveMountEntity
     * @return LiveMountEntity
     */
    default LiveMountEntity addLiveMountFileSystemName(LiveMountEntity liveMountEntity) {
        return liveMountEntity;
    }
}
