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

import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountMigrateRequest;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.util.Applicable;

import java.util.List;
import java.util.Optional;

/**
 * Live Mount Provider
 *
 * @author h30003246
 * @since 2021-01-05
 */
public interface LiveMountFlowService extends Applicable<String> {
    /**
     * migrate live mount pre-check
     *
     * @param liveMountEntity live mount entity
     * @param liveMountMigrateRequest migrate request
     * @return strict mode
     */
    boolean migrateLiveMountPreCheck(LiveMountEntity liveMountEntity, LiveMountMigrateRequest liveMountMigrateRequest);

    /**
     * migrate live mount
     *
     * @param param migrate params
     */
    void migrateLiveMount(JSONObject param);

    /**
     * get target resource uuid list
     *
     * @param targetResourceUuidList target resource uuid list
     * @param liveMountTargetLocation live mount target location
     * @param resourceEntity resource
     * @return filter resource uuid list
     */
    List<String> filterTargetResourceUuidList(List<String> targetResourceUuidList,
        LiveMountTargetLocation liveMountTargetLocation, ResourceEntity resourceEntity);

    /**
     * get target object name
     *
     * @param liveMountObject live mount object
     * @param resourceName resource name
     * @return target object name
     */
    Optional<String> getTargetObjectName(LiveMountObject liveMountObject, String resourceName);

    /**
     * get target environments
     *
     * @param liveMountEntities live mount entities
     * @param copy copy
     * @return environments
     */
    List<Environment> getEnvironments(List<LiveMountEntity> liveMountEntities, Copy copy);

    /**
     * refresh target resource
     *
     * @param jobId job id
     * @param liveMount live mount entity
     * @param count count
     * @param hasClearProtection has clear protection
     */
    void checkRefreshTargetResource(String jobId, LiveMountEntity liveMount, int count, boolean hasClearProtection);

    /**
     * get clone copy feature by resource sub type
     *
     * @param feature feature
     * @return cal feature
     */
    int getCloneCopyFeatureByResourceSubType(int feature);

    /**
     * get job target location
     *
     * @param liveMountEntity  livemount entity
     * @return target location
     */
    String getJobTargetLocation(LiveMountEntity liveMountEntity);
}
