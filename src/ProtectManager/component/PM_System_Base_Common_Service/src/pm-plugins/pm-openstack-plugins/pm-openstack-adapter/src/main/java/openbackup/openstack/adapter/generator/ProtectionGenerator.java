/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.openstack.adapter.generator;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.enums.OpenStackJobType;
import openbackup.openstack.protection.access.constant.OpenstackConstant;

import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ProtectionCreationDto;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;
import openbackup.system.base.sdk.resource.model.ProtectionResourceDto;

import org.springframework.beans.BeanUtils;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 保护相关对象生成器
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-17
 */
public final class ProtectionGenerator {
    private ProtectionGenerator() {}

    /**
     * 生成ProtectionCreationDto
     *
     * @param slaId 绑定的SLA的id
     * @param resourceId 资源id
     * @param backupJob {@link OpenStackBackupJobDto} backupJob
     * @return {@link ProtectionCreationDto} ProtectionCreationDto
     */
    public static ProtectionCreationDto generateProtectionCreation(
            String slaId, String resourceId, OpenStackBackupJobDto backupJob) {
        ProtectionCreationDto creationDto = new ProtectionCreationDto();
        creationDto.setSlaId(slaId);
        creationDto.setResources(buildProtectionResources(resourceId));
        creationDto.setExtParameters(buildExtParameters(backupJob));
        return creationDto;
    }

    private static List<ProtectionResourceDto> buildProtectionResources(String resourceId) {
        // 对云主机保护，无资源过滤规则
        ProtectionResourceDto resourceDto = new ProtectionResourceDto();
        resourceDto.setResourceId(resourceId);
        return Collections.singletonList(resourceDto);
    }

    private static Map<String, Object> buildExtParameters(OpenStackBackupJobDto backupJob) {
        // 设置保护高级参数，只做保留，不在业务中使用
        Map<String, Object> params = new HashMap<>();
        params.put(OpenStackConstants.NAME, backupJob.getName());
        params.put(OpenStackConstants.DESCRIPTION, backupJob.getDescription());
        params.put(OpenStackConstants.BACKUP_TYPE, backupJob.getType().getType());
        params.put(OpenStackConstants.INSTANCE_ID, backupJob.getInstanceId());
        if (Objects.equals(backupJob.getType(), OpenStackJobType.SERVER)) {
            params.put(OpenstackConstant.ALL_DISK, true);
            params.put(OpenstackConstant.DISK_IDS, Collections.emptyList());
        } else {
            params.put(OpenstackConstant.ALL_DISK, false);
            params.put(OpenstackConstant.DISK_IDS, Collections.singletonList(backupJob.getInstanceId()));
        }
        return params;
    }

    /**
     * 将ProtectedObject转为ProtectedObjectInfo
     *
     * @param protectedObject {@link ProtectedObject} ProtectedObject
     * @return {@link ProtectedObjectInfo} ProtectedObjectInfo
     */
    public static ProtectedObjectInfo convert2ProtectedObjectInfo(ProtectedObject protectedObject) {
        ProtectedObjectInfo objectInfo = new ProtectedObjectInfo();
        BeanUtils.copyProperties(protectedObject, objectInfo);
        Map<String, Object> extParameters = protectedObject.getExtParameters();
        objectInfo.setExtParameters(extParameters);
        return objectInfo;
    }

    /**
     * 生成修改保护请求体
     *
     * @param slaId SLA id
     * @param resourceId 资源id
     * @param backupJob {@link OpenStackBackupJobDto} backupJob
     * @param oldProtectedObject {@link ProtectedObjectInfo} 修改前保护对象信息
     * @return 修改保护请求体
     */
    public static ProtectionModifyDto generateProtectionModifyReq(
            String slaId, String resourceId, OpenStackBackupJobDto backupJob, ProtectedObjectInfo oldProtectedObject) {
        ProtectionModifyDto protection = new ProtectionModifyDto();
        protection.setSlaId(slaId);
        protection.setResourceId(resourceId);

        Map<String, Object> extParameters = oldProtectedObject.getExtParameters();
        extParameters.put(OpenStackConstants.DESCRIPTION, backupJob.getDescription());

        protection.setExtParameters(extParameters);

        return protection;
    }
}
