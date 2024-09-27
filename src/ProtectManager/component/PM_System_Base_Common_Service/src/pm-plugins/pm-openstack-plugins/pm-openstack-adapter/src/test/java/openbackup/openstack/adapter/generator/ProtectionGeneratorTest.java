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

import static org.assertj.core.api.Assertions.assertThat;
import static org.assertj.core.api.Assertions.entry;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.testdata.TestDataGenerator;
import openbackup.openstack.protection.access.constant.OpenstackConstant;

import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;
import openbackup.system.base.sdk.resource.model.ProtectionCreationDto;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;
import openbackup.system.base.sdk.resource.model.ProtectionResourceDto;

import org.junit.Test;

import java.io.IOException;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * {@link ProtectionGenerator} 测试类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-06
 */
public class ProtectionGeneratorTest {
    /**
     * 用例名称：验证生成ProtectionCreationDto对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_correctValue_when_generateProtectionCreation_givenServerBackupJob() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createTimeRetentionDaysScheduleJob();
        String resourceId = "test resource id";
        String slaId = "test sla id";
        ProtectionCreationDto protectionCreation = ProtectionGenerator.generateProtectionCreation(slaId, resourceId,
            backupJob);
        assertThat(protectionCreation.getSlaId()).isEqualTo(slaId);
        List<ProtectionResourceDto> resources = protectionCreation.getResources();
        assertThat(resources).isNotEmpty();
        List<String> resourceIds = resources.stream()
            .map(ProtectionResourceDto::getResourceId)
            .collect(Collectors.toList());
        assertThat(resourceIds).usingRecursiveComparison().isEqualTo(Collections.singletonList(resourceId));
        Map<String, Object> extParameters = protectionCreation.getExtParameters();
        assertThat(extParameters).isNotNull();
        assertThat(extParameters.get(OpenStackConstants.NAME)).hasToString(backupJob.getName());
        assertThat(extParameters.get(OpenStackConstants.DESCRIPTION)).hasToString(backupJob.getDescription());
        assertThat(extParameters.get(OpenStackConstants.BACKUP_TYPE)).hasToString(backupJob.getType().getType());
        assertThat(extParameters.get(OpenStackConstants.INSTANCE_ID)).hasToString(backupJob.getInstanceId());
        assertThat(extParameters).containsEntry(OpenstackConstant.ALL_DISK, true);
        assertThat(extParameters.get(OpenstackConstant.DISK_IDS)).asList().isEmpty();
    }

    /**
     * 用例名称：验证生成ProtectionCreationDto对象正确<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_return_correctValue_when_generateProtectionCreation_givenVolumeBackupJob() throws IOException {
        OpenStackBackupJobDto backupJob = TestDataGenerator.createVolumeTypeWithoutJobsScheduleJob();
        String resourceId = "test resource id";
        String slaId = "test sla id";
        ProtectionCreationDto protectionCreation = ProtectionGenerator.generateProtectionCreation(slaId, resourceId,
            backupJob);
        assertThat(protectionCreation.getSlaId()).isEqualTo(slaId);
        List<ProtectionResourceDto> resources = protectionCreation.getResources();
        assertThat(resources).isNotEmpty();
        List<String> resourceIds = resources.stream()
            .map(ProtectionResourceDto::getResourceId)
            .collect(Collectors.toList());
        assertThat(resourceIds).usingRecursiveComparison().isEqualTo(Collections.singletonList(resourceId));
        Map<String, Object> extParameters = protectionCreation.getExtParameters();
        assertThat(extParameters).isNotNull();
        assertThat(extParameters.get(OpenStackConstants.NAME)).hasToString(backupJob.getName());
        assertThat(extParameters.get(OpenStackConstants.DESCRIPTION)).hasToString(backupJob.getDescription());
        assertThat(extParameters.get(OpenStackConstants.BACKUP_TYPE)).hasToString(backupJob.getType().getType());
        assertThat(extParameters.get(OpenStackConstants.INSTANCE_ID)).hasToString(backupJob.getInstanceId());
        assertThat(extParameters).containsEntry(OpenstackConstant.ALL_DISK, false)
            .containsEntry(OpenstackConstant.DISK_IDS, Collections.singletonList(backupJob.getInstanceId()));
    }

    /**
     * 用例名称：验证将ProtectedObject转为ProtectedObjectInfo正常<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_convertProtectedObjectToObjectInfo() {
        ProtectedObject protectedObject = new ProtectedObject();
        protectedObject.setSlaId(UUIDGenerator.getUUID());
        protectedObject.setStatus(ProtectionStatusEnum.PROTECTED.getType());
        protectedObject.setResourceId(UUIDGenerator.getUUID());
        Map<String, Object> params = new HashMap<>();
        params.put("params1", "value1");
        protectedObject.setExtParameters(params);

        ProtectedObjectInfo objectInfo = ProtectionGenerator.convert2ProtectedObjectInfo(protectedObject);
        assertThat(objectInfo.getSlaId()).isEqualTo(protectedObject.getSlaId());
        assertThat(objectInfo.getStatus()).isEqualTo(protectedObject.getStatus());
        assertThat(objectInfo.getResourceId()).isEqualTo(protectedObject.getResourceId());
        assertThat(objectInfo.getExtParameters().size()).isEqualTo(params.size());
        assertThat(objectInfo.getExtParameters().containsKey("params1")).isTrue();
        assertThat(objectInfo.getExtParameters().get("params1").toString()).isEqualTo(params.get("params1"));
    }

    /**
     * 用例名称：验证生成ProtectionModifyReq正常<br/>
     * 前置条件：无<br/>
     * check点：1.对象不为空  2.字段属性值设置正确<br/>
     */
    @Test
    public void should_returnCorrectValue_when_generateProtectionModifyReq() {
        String slaId = UUIDGenerator.getUUID();
        String resourceId = UUIDGenerator.getUUID();
        OpenStackBackupJobDto backupJob = new OpenStackBackupJobDto();
        backupJob.setDescription("new description.");
        ProtectedObjectInfo protectedObject = new ProtectedObjectInfo();
        Map<String, Object> extParams = new HashMap<>();
        extParams.put("description", "test description.");
        protectedObject.setExtParameters(extParams);

        ProtectionModifyDto modifyDto = ProtectionGenerator.generateProtectionModifyReq(slaId, resourceId, backupJob,
            protectedObject);

        assertThat(modifyDto).isNotNull();
        assertThat(modifyDto.getSlaId()).isEqualTo(slaId);
        assertThat(modifyDto.getResourceId()).isEqualTo(resourceId);
        assertThat(modifyDto.getExtParameters()).doesNotContainEntry("description", "test description.");
        assertThat(modifyDto.getExtParameters()).contains(entry("description", "new description."));
    }
}
