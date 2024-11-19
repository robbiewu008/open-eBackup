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
package openbackup.openstack.adapter.generator;

import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.openstack.protection.access.dto.VolInfo;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import openbackup.openstack.adapter.enums.OpenStackJobType;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 恢复相关对象生成器
 *
 */
@Slf4j
public class RestoreGenerator {
    private RestoreGenerator() {}

    /**
     * 生成下发备份任务请求结构体
     *
     * @param restoreJob {@link OpenStackRestoreJobDto} 北向接口创建恢复任务请求体
     * @param resource {@link ProtectedResource} 受保护资源
     * @return {@link CreateRestoreTaskRequest} 下发备份任务请求体
     */
    public static CreateRestoreTaskRequest generateCreateRestoreReq(
            OpenStackRestoreJobDto restoreJob, ProtectedResource resource) {
        CreateRestoreTaskRequest request = new CreateRestoreTaskRequest();

        request.setCopyId(restoreJob.getCopyId());
        request.setAgents(Collections.emptyList());
        request.setTargetEnv(resource.getRootUuid());
        // OpenStack只有普通恢复
        request.setRestoreType(RestoreTypeEnum.CR);
        // OpenStack只有原位置恢复
        request.setTargetLocation(RestoreLocationEnum.ORIGINAL);
        request.setTargetObject(resource.getUuid());
        request.setSubObjects(
                buildSubObjects(restoreJob, resource.getExtendInfoByKey(OpenstackConstant.VOLUME_INFO_KEY)));
        request.setExtendInfo(buildExtendInfo(restoreJob, resource));

        return request;
    }

    private static List<String> buildSubObjects(OpenStackRestoreJobDto restoreJob, String volInfos) {
        List<VolInfo> volumes = JSONArray.fromObject(volInfos).toBean(VolInfo.class);
        List<String> subObjects;
        if (Objects.equals(restoreJob.getType(), OpenStackJobType.SERVER)) {
            subObjects = extractAllVolumes(volumes);
        } else {
            subObjects = extractSingleVolume(restoreJob.getInstanceId(), volumes);
        }
        log.info("Build restore sub objects success, size: {}.", subObjects.size());
        return subObjects;
    }

    private static List<String> extractAllVolumes(List<VolInfo> volumes) {
        List<String> subObjects = new ArrayList<>();
        volumes.forEach(volume -> subObjects.add(buildSubObject(volume)));
        return subObjects;
    }

    private static List<String> extractSingleVolume(String volumeId, List<VolInfo> volumes) {
        List<String> subObjects = new ArrayList<>();
        volumes.stream()
                .filter(volume -> Objects.equals(volume.getId(), volumeId))
                .forEach(volume -> subObjects.add(buildSubObject(volume)));
        return subObjects;
    }

    private static String buildSubObject(VolInfo volume) {
        Map<String, Object> subObject = new HashMap<>();
        subObject.put(OpenstackConstant.UUID_KEY, volume.getId());

        Map<String, String> subObjectExtInfo = new HashMap<>();
        Map<String, Object> targetVolume = new HashMap<>();
        targetVolume.put(OpenstackConstant.ID_KEY, volume.getId());
        targetVolume.put(OpenstackConstant.VOLUME_SIZE_KEY, volume.getSize());
        subObjectExtInfo.put(OpenstackConstant.TARGET_VOLUME_KEY,
            JSONObject.fromObject(targetVolume).toString());

        subObject.put(OpenstackConstant.EXTEND_INFO_KEY, JSONObject.fromObject(subObjectExtInfo));
        return JSONObject.fromObject(subObject).toString();
    }

    private static Map<String, String> buildExtendInfo(OpenStackRestoreJobDto restoreJob, ProtectedResource resource) {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put(OpenstackConstant.COPY_VERIFY, Boolean.FALSE.toString());
        extendInfo.put(OpenstackConstant.RESTORE_LOCATION, resource.getPath());
        // 是否上下电根据原资源上下电情况设置
        JSONObject serverExtendInfo =
                JSONObject.fromObject(resource.getExtendInfoByKey(OpenstackConstant.RES_TYPE_SERVER));
        String serverStatus =
                serverExtendInfo.getString(OpenstackConstant.SERVER_STATUS, OpenstackConstant.SERVER_STATUS_ACTIVE);
        extendInfo.put(
                OpenstackConstant.POWER_STATE,
                Objects.equals(serverStatus, OpenstackConstant.SERVER_STATUS_ACTIVE)
                        ? OpenstackConstant.POWER_ON
                        : OpenstackConstant.POWER_OFF);
        // 恢复任务请求中值，只做保留，不在实际业务中使用
        extendInfo.put(OpenStackConstants.RESTORE_NAME, restoreJob.getName());
        extendInfo.put(OpenStackConstants.DESCRIPTION, restoreJob.getDescription());
        extendInfo.put(OpenStackConstants.RESTORE_TYPE, restoreJob.getType().getType());
        extendInfo.put(OpenStackConstants.INSTANCE_ID, restoreJob.getInstanceId());
        return extendInfo;
    }
}
