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
package openbackup.data.access.framework.livemount.data;

import openbackup.data.access.framework.livemount.common.enums.ScheduledType;
import openbackup.data.access.framework.livemount.common.model.LiveMountModel;
import openbackup.data.access.framework.livemount.common.model.LiveMountObject;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountEnableStatus;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountParam;
import openbackup.data.access.framework.livemount.controller.livemount.model.LiveMountStatus;
import openbackup.data.access.framework.livemount.entity.LiveMountPolicyEntity;
import openbackup.system.base.common.constants.TokenBo;
import openbackup.system.base.common.model.livemount.LiveMountEntity;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyResourceSummary;
import openbackup.system.base.sdk.livemount.model.LiveMountTargetLocation;
import openbackup.system.base.sdk.resource.model.Environment;
import openbackup.system.base.sdk.resource.model.FileSetEntity;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.schedule.model.ScheduleResponse;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * live mount service test
 *
 * @author h30003246
 * @since 2021-02-25
 */
public class LiveMountServiceImplTestData extends LiveMountCommonTestData {
    private static final String LIVE_MOUNT_ID = "83445bf0-f601-4509-b6c1-05534318206d";
    private static final String MOUNTED_COPY_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String USER_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String ROLE_ID = "83445bf0-f601-4509-b6c1-05534318206h";
    private static final String POLICY_ID = "83445bf0-f601-4509-b6c1-05534318206h";
    private static final String SCHEDULE_ID = "83445bf0-f601-4509-b6c1-05534318206h";
    private static final String ROLE_BO_NAME = "Role_sys_admin";
    private static final String USER_BO_NAME = "sysadmin";
    private static final String SOURCE_RESOURCE_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String COPY_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String RESOURCE_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String FILE_SET_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String MODEL_ID = "83445bf0-f601-4509-b6c1-05534318206a";
    private static final String PARAMETER_NAME = "parameter_name";
    private static final String FILE_SET_NAME = "fileset_name";
    private static final String FILE_SET_PATH = "fileset_path";
    private static final String ENVIRONMENT_ID = "1";
    private static final String SCHEDULER_INTERVAL_UNIT = "d";

    /**
     * get live mount entity
     *
     * @return live mount entity
     */
    public static LiveMountEntity getLiveMountEntity() {
        LiveMountEntity liveMountEntity = new LiveMountEntity();
        liveMountEntity.setId(LIVE_MOUNT_ID);
        liveMountEntity.setMountedCopyId(MOUNTED_COPY_ID);
        liveMountEntity.setStatus(LiveMountStatus.AVAILABLE.getName());
        liveMountEntity.setResourceId(RESOURCE_ID);
        liveMountEntity.setEnableStatus(LiveMountEnableStatus.ACTIVATED.getName());
        liveMountEntity.setStatus(LiveMountStatus.AVAILABLE.getName());
        liveMountEntity.setMountedCopyId(MOUNTED_COPY_ID);
        liveMountEntity.setPolicyId(POLICY_ID);
        liveMountEntity.setScheduleId(SCHEDULE_ID);
        liveMountEntity.setTargetLocation(LiveMountTargetLocation.ORIGINAL.getValue());
        liveMountEntity.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        return liveMountEntity;
    }

    /**
     * get token bo
     *
     * @return TokenBo
     */
    public static TokenBo getTokenBo() {
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        roleBo.setId(ROLE_ID);
        roleBo.setName(ROLE_BO_NAME);
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        userBo.setId(USER_ID);
        userBo.setRoles(Collections.singletonList(roleBo));
        userBo.setName(USER_BO_NAME);
        long exp = 144444444444L;
        long created = 144444444444L;
        return TokenBo.builder().user(userBo).exp(exp).created(created).build();
    }

    /**
     * get LiveMountObject
     *
     * @return LiveMountObject
     */
    public static LiveMountObject getLiveMountObject() {
        LiveMountObject liveMountObject = new LiveMountObject();
        liveMountObject.setSourceResourceId(SOURCE_RESOURCE_ID);
        liveMountObject.setCopyId(COPY_ID);
        liveMountObject.setPolicyId(POLICY_ID);
        liveMountObject.setTargetLocation(LiveMountTargetLocation.ORIGINAL);
        liveMountObject.setTargetResourceUuidList(Arrays.asList("1", "3"));
        Map<String, Object> parameters = new HashMap<>();
        parameters.put("name", PARAMETER_NAME);
        liveMountObject.setParameters(parameters);
        return liveMountObject;
    }

    /**
     * get copy
     *
     * @return Copy
     */
    public static Copy getCopy() {
        Copy copy = new Copy();
        copy.setUuid(COPY_ID);
        copy.setResourceId(RESOURCE_ID);
        copy.setAmount(1);
        copy.setGn(1);
        copy.setPrevCopyId(COPY_ID);
        copy.setNextCopyId(COPY_ID);
        copy.setNextCopyGn(1);
        copy.setPrevCopyGn(1);
        copy.setTimestamp("11111111");
        return copy;
    }

    /**
     * get ResourceEntity
     *
     * @return ResourceEntity
     */
    public static ResourceEntity getResourceEntity() {
        ResourceEntity ResourceEntity = new ResourceEntity();
        ResourceEntity.setPath("test");
        ResourceEntity.setUuid(COPY_ID);
        ResourceEntity.setName(PARAMETER_NAME);
        ResourceEntity.setType(ResourceSubTypeEnum.VMWARE.getType());
        return ResourceEntity;
    }

    /**
     * get CopyResourceSummary
     *
     * @return CopyResourceSummary
     */
    public static CopyResourceSummary getCopyResourceSummary() {
        CopyResourceSummary copyResourceSummary = new CopyResourceSummary();
        copyResourceSummary.setResourceId(RESOURCE_ID);
        copyResourceSummary.setResourceSubType(ResourceSubTypeEnum.VMWARE.getType());
        JSONObject jsonObject = new JSONObject();
        jsonObject.set("uuid", COPY_ID);
        jsonObject.set("name", PARAMETER_NAME);
        jsonObject.set("sub_type", ResourceSubTypeEnum.VMWARE.getType());
        copyResourceSummary.setResourceProperties(jsonObject.toString());
        return copyResourceSummary;
    }

    /**
     * get FileSetEntity
     *
     * @return FileSetEntity
     */
    public static FileSetEntity getFileSetEntity() {
        FileSetEntity fileSetEntity = new FileSetEntity();
        fileSetEntity.setUuid(FILE_SET_ID);
        fileSetEntity.setPath(FILE_SET_PATH);
        fileSetEntity.setName(FILE_SET_NAME);
        fileSetEntity.setEnvironmentEndPoint("environmentEndPoint");
        fileSetEntity.setEnvironmentUuid(ENVIRONMENT_ID);
        return fileSetEntity;
    }

    /**
     * get LiveMountPolicyEntity
     *
     * @return LiveMountPolicyEntity
     */
    public static LiveMountPolicyEntity getLiveMountPolicyEntity() {
        LiveMountPolicyEntity liveMountPolicyEntity = new LiveMountPolicyEntity();
        liveMountPolicyEntity.setPolicyId(POLICY_ID);
        liveMountPolicyEntity.setCopyDataSelectionPolicy(POLICY_ID);
        liveMountPolicyEntity.setSchedulePolicy(ScheduledType.PERIOD_SCHEDULE.getName());
        liveMountPolicyEntity.setScheduleInterval(11);
        liveMountPolicyEntity.setScheduleIntervalUnit(SCHEDULER_INTERVAL_UNIT);
        return liveMountPolicyEntity;
    }

    /**
     * get LiveMountModel
     *
     * @return LiveMountModel
     */
    public static LiveMountModel getLiveMountModel() {
        LiveMountModel liveMountModel = new LiveMountModel();
        liveMountModel.setId(MODEL_ID);
        return liveMountModel;
    }

    /**
     * get ScheduleResponse
     *
     * @return ScheduleResponse
     */
    public static ScheduleResponse getScheduleResponse() {
        ScheduleResponse scheduleResponse = new ScheduleResponse();
        scheduleResponse.setScheduleId(SCHEDULE_ID);
        return scheduleResponse;
    }

    /**
     * get LiveMountParam
     *
     * @return LiveMountParam
     */
    public static LiveMountParam getLiveMountParam() {
        LiveMountParam liveMountParam = new LiveMountParam();
        liveMountParam.setPolicyId(POLICY_ID);
        return liveMountParam;
    }

    /**
     * get esx vmware resource base page
     *
     * @param environment environment
     * @return base page of virtual resource schema
     */
    public static BasePage<Environment> getEnvironmentBasePage(Environment environment) {
        BasePage<Environment> basePage = new BasePage<>();
        basePage.setItems(Collections.singletonList(environment));
        basePage.setPageNo(1);
        basePage.setPageSize(1);
        basePage.setTotal(1);
        basePage.setPages(2);
        return basePage;
    }

    /**
     * getEnvironment
     *
     * @return environment
     */
    public static Environment getEnvironment() {
        return new Environment();
    }
}
