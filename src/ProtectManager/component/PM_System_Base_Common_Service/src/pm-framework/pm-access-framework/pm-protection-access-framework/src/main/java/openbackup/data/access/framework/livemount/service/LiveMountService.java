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
package openbackup.data.access.framework.livemount.service;

import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.entity.CreateWindowsUserResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.entity.QueryWindowsUserResponse;
import com.huawei.oceanprotect.system.base.sdk.devicemanager.openstorage.entity.WindowsUserParam;

import openbackup.data.access.framework.livemount.common.model.LiveMountModel;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.common.model.UnmountExtendParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountMigrateRequest;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.resource.model.ResourceEntity;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * Live Mount Service
 *
 */
public interface LiveMountService {
    /**
     * LIVE_MOUNT_DEBUTS
     */
    String LIVE_MOUNT_DEBUTS = "live_mount.debuts";

    /**
     * create live mount
     *
     * @param liveMountObject live mount object
     * @param copy copy
     * @param policy policy
     * @return live mount uuid list and source copy info
     */
    Map.Entry<Copy, List<LiveMountEntity>> createLiveMounts(LiveMountObject liveMountObject, Copy copy,
        LiveMountPolicyEntity policy);

    /**
     * query resource by id
     *
     * @param resourceId resource id
     * @return resource
     */
    ResourceEntity queryResource(String resourceId);

    /**
     * query copy resource summary
     *
     * @param resourceId resource id
     * @return copy resource summary
     */
    CopyResourceSummary queryCopyResourceSummary(String resourceId);

    /**
     * query resources
     *
     * @param resourceIds resource ids
     * @param type type
     * @return resource entities
     */
    List<ResourceEntity> queryResources(List<String> resourceIds, String type);

    /**
     * get resource id of source copy
     *
     * @param liveMountEntity live mount entity
     * @return the resource id of source copy
     */
    String getSourceCopyResourceId(LiveMountEntity liveMountEntity);

    /**
     * get resource of source copy
     *
     * @param liveMountEntity live mount entity
     * @return resource of source copy
     */
    ResourceEntity getSourceCopyResource(LiveMountEntity liveMountEntity);

    /**
     * update live mount
     *
     * @param liveMountEntity live mount entity
     * @param resourceId resource id
     * @param policyId policy id
     */
    void updateLiveMountPolicy(LiveMountEntity liveMountEntity, String resourceId, String policyId);

    /**
     * update mounted copy info
     *
     * @param liveMountEntity live mount entity
     * @param mountedCopy mounted copy
     */
    void updateMountedCopyInfo(LiveMountEntity liveMountEntity, Copy mountedCopy);

    /**
     * delete live mount
     *
     * @param liveMountId live mount id
     */
    void deleteLiveMount(String liveMountId);

    /**
     * update live mount
     *
     * @param liveMountEntity live mount entity
     * @param policy policy
     * @param copyId copy id
     * @param isManualUpdate 是否是手动触发的更新
     */
    void updateLiveMount(LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy, String copyId,
        boolean isManualUpdate);

    /**
     * execute live mount on copy changed
     *
     * @param liveMountEntity live mount entity
     * @param policy policy
     * @param sourceCopy source copy
     * @param isStrict strict mode
     */
    void executeLiveMountOnCopyChanged(LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy, Copy sourceCopy,
        boolean isStrict);

    /**
     * clean mounted copy info
     *
     * @param liveMountEntity live mount entity
     */
    void cleanMountedCopyInfo(LiveMountEntity liveMountEntity);

    /**
     * execute live mount
     *
     * @param liveMountEntity live mount entity
     * @param policy policy
     * @param sourceCopy source copy
     * @param mountedCopy mounted copy
     * @param isDebuts debuts
     */
    void executeLiveMount(LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy, Copy sourceCopy,
        Copy mountedCopy, boolean isDebuts);

    /**
     * execute live mount
     *
     * @param liveMountEntity live mount entity
     * @param policy policy
     * @param sourceCopy source copy
     * @param mountedCopy mounted copy
     */
    default void executeLiveMount(LiveMountEntity liveMountEntity, LiveMountPolicyEntity policy, Copy sourceCopy,
        Copy mountedCopy) {
        executeLiveMount(liveMountEntity, policy, sourceCopy, mountedCopy, false);
    }

    /**
     * query valid copy
     *
     * @param sourceResourceId source resource id
     * @param copyId copy id
     * @return valid copy
     */
    Copy queryValidCopy(String sourceResourceId, String copyId);

    /**
     * query live mount entities
     *
     * @param page page
     * @param size size
     * @param conditions conditions
     * @param orders orders
     * @return live mount entity page
     */
    BasePage<LiveMountModel> queryLiveMountEntities(int page, int size, String conditions, List<String> orders);

    /**
     * query live mount entities by policy id
     *
     * @param policyId live mount policy id
     * @return policy entity
     */
    List<LiveMountEntity> queryLiveMountEntitiesByPolicyId(String policyId);

    /**
     * get live mount entity by id
     *
     * @param liveMountId live mount id
     * @return live mount entity
     */
    LiveMountEntity selectLiveMountEntityById(String liveMountId);

    /**
     * modify live mount
     *
     * @param liveMountId live mount id
     * @param liveMountParam live mount param
     */
    void modifyLiveMount(String liveMountId, LiveMountParam liveMountParam);

    /**
     * destroy live mount
     *
     * @param liveMountId live mount id
     * @param isReserveCopy reserve copy
     * @param isForceDelete force delete
     * @param extendParam extendParam
     */
    void unmountLiveMount(String liveMountId, boolean isReserveCopy, boolean isForceDelete,
        UnmountExtendParam extendParam);

    /**
     * initial Live Mount Schedule
     *
     * @param liveMountEntity live mount entity
     * @param policyId policy id
     * @return schedule id
     */
    Optional<String> initialLiveMountSchedule(LiveMountEntity liveMountEntity, String policyId);

    /**
     * initial Live Mount Schedule
     *
     * @param liveMountEntity live mount entity
     * @param policyId policy id
     */
    void initialAndUpdateLiveMountSchedule(LiveMountEntity liveMountEntity, String policyId);

    /**
     * update live mount status
     *
     * @param liveMountEntity live mount entity
     * @param status status
     */
    void updateLiveMountStatus(LiveMountEntity liveMountEntity, LiveMountStatus status);

    /**
     * update live mount schedule
     *
     * @param liveMountEntity live mount entity
     * @param scheduleId schedule id
     */
    void updateLiveMountSchedule(LiveMountEntity liveMountEntity, String scheduleId);

    /**
     * update live mount parameters
     *
     * @param liveMountEntity liveMountEntity
     * @param parameters parameters
     */
    void updateLiveMountParameters(LiveMountEntity liveMountEntity, String parameters);

    /**
     * update live mounted resource
     *
     * @param liveMountEntity live mount entity
     * @param mountedResource mounted resource
     */
    void updateLiveMountMountedResource(LiveMountEntity liveMountEntity, String mountedResource);

    /**
     * get target environments
     *
     * @param resourceId resource id
     * @return environment entity page
     */
    BasePage<Environment> queryTargetEnvironments(String resourceId);

    /**
     * activate the live mount
     *
     * @param liveMountId live mount id
     */
    void activateLiveMount(String liveMountId);

    /**
     * deactivate the live mount
     *
     * @param liveMountId live mount id
     */
    void deactivateLiveMount(String liveMountId);

    /**
     * if live mount enable status is not equal activated, raise error.
     *
     * @param enableStatus activated, disabled
     * @param isStrict true, false
     * @return enable_status
     */
    boolean checkHasActive(String enableStatus, boolean isStrict);

    /**
     * check target environment status
     *
     * @param liveMount live mount
     */
    void checkTargetEnvironmentStatus(LiveMountEntity liveMount);

    /**
     * migrate Live mount
     *
     * @param liveMountId live mount id
     * @param mountMigrateRequest migrate live mount request
     */
    void migrateLiveMount(String liveMountId, LiveMountMigrateRequest mountMigrateRequest);

    /**
     * check live mount status
     *
     * @param status live mount status
     * @param operate live mount operate
     */
    void checkLiveMountStatus(String status, String operate);

    /**
     * revoke live mount user id
     *
     * @param userId user id
     */
    void revokeLiveMountUserId(String userId);

    /**
     * 取消即时挂载任务
     *
     * @param liveMountId live mount id
     */
    void cancelLiveMount(String liveMountId);

    /**
     * 根据副本ID统计livemount数量
     *
     * @param copyId 副本id
     * @return livemount数量
     */
    int countLiveMountByCopyId(String copyId);

    /**
     * create live mount
     *
     * @param liveMountObject liveMountObject
     * @return uuids
     */
    List<String> createLiveMountCommon(LiveMountObject liveMountObject);

    /**
     * create windows user
     *
     * @param windowsUserParam windowsUserParam
     * @return result
     */
    CreateWindowsUserResponse createHcsCifsUser(WindowsUserParam windowsUserParam);

    /**
     * query windows user
     *
     * @param windowsUsername 用户名
     * @param vstoreId 租户id
     * @param accountName 账户名
     * @return result
     */
    QueryWindowsUserResponse queryHcsCifsUser(String windowsUsername, String vstoreId, String accountName);

    /**
     * delete windows user
     *
     * @param windowsUsername 用户名
     * @param vstoreId 租户id
     * @param accountName 账户名
     */
    void deleteHcsCifsUser(String windowsUsername, String vstoreId, String accountName);

    /**
     * check live mount copy
     *
     * @param copy copy
     */
    void checkSourceCopy(Copy copy);
}
