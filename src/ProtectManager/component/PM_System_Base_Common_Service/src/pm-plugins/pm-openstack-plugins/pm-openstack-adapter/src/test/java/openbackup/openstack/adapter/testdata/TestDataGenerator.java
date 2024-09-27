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
package openbackup.openstack.adapter.testdata;

import openbackup.openstack.adapter.constants.OpenStackConstants;
import openbackup.openstack.adapter.dto.OpenStackBackupJobDto;
import openbackup.openstack.adapter.dto.OpenStackRestoreJobDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ProtectedObjectInfo;

import com.fasterxml.jackson.databind.ObjectMapper;

import java.io.IOException;
import java.net.URL;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.HashMap;
import java.util.Map;

/**
 * 测试数据生成类
 *
 * @author w00616953
 * @version [OceanProtect X8000 1.3.0]
 * @since 2022-12-14
 */
public class TestDataGenerator {
    private static final ObjectMapper MAPPER = new ObjectMapper();

    public static OpenStackBackupJobDto createTimeRetentionDaysScheduleJob() throws IOException {
        Path path = Paths.get("TimeRetentionDaysScheduleJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackBackupJobDto.class);
    }

    public static OpenStackBackupJobDto createNumberRetentionWeekScheduleJob() throws IOException {
        Path path = Paths.get("NumberRetentionWeekScheduleJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackBackupJobDto.class);
    }

    public static OpenStackBackupJobDto createZeroCountRetentionWeekScheduleJob() throws IOException {
        Path path = Paths.get("ZeroCountRetentionWeekScheduleJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackBackupJobDto.class);
    }

    public static OpenStackBackupJobDto createDaysScheduleWithoutIntervalDaysJob() throws IOException {
        Path path = Paths.get("DaysScheduleWithoutIntervalDaysJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackBackupJobDto.class);
    }

    public static OpenStackBackupJobDto createWeekScheduleWithoutDaysOfWeekJob() throws IOException {
        Path path = Paths.get("WeekScheduleWithoutDaysOfWeekJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackBackupJobDto.class);
    }

    public static OpenStackBackupJobDto createVolumeTypeWithoutJobsScheduleJob() throws IOException {
        Path path = Paths.get("VolumeTypeWithoutJobsScheduleJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackBackupJobDto.class);
    }

    public static ProtectedObjectInfo createProtectedObject() {
        ProtectedObjectInfo object = new ProtectedObjectInfo();
        object.setStatus(ProtectionStatusEnum.PROTECTED.getType());
        object.setResourceId("test resource id");
        Map<String, Object> extParams = new HashMap<>();
        extParams.put(OpenStackConstants.NAME, "backup name");
        extParams.put(OpenStackConstants.DESCRIPTION, "description");
        extParams.put(OpenStackConstants.BACKUP_TYPE, "server");
        extParams.put(OpenStackConstants.INSTANCE_ID, "instance id");
        object.setExtParameters(extParams);
        return object;
    }

    public static ProtectedObjectInfo createUnprotectedObject() {
        ProtectedObjectInfo object = createProtectedObject();
        object.setStatus(ProtectionStatusEnum.UNPROTECTED.getType());
        return object;
    }

    public static SlaDto createGlobalSla() throws IOException {
        Path path = Paths.get("GlobalSla.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, SlaDto.class);
    }

    public static SlaDto createIntervalAndPermanentSla() throws IOException {
        Path path = Paths.get("IntervalSchedulePermanentRetentionSla.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, SlaDto.class);
    }

    public static SlaDto createWeekAndTemporarySla() throws IOException {
        Path path = Paths.get("WeekScheduleTemporaryRetentionSla.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, SlaDto.class);
    }

    public static SlaDto createWeekAndQuantitySla() throws IOException {
        Path path = Paths.get("WeekScheduleQuantityRetentionSla.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, SlaDto.class);
    }

    public static OpenStackRestoreJobDto createServerRestoreJob() throws IOException {
        Path path = Paths.get("ServerRestoreJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackRestoreJobDto.class);
    }

    public static OpenStackRestoreJobDto createVolumeRestoreJob() throws IOException {
        Path path = Paths.get("VolumeRestoreJob.json");
        URL url = TestDataGenerator.class.getClassLoader().getResource(path.toString());
        return MAPPER.readValue(url, OpenStackRestoreJobDto.class);
    }

    public static JobBo createSuccessJob() {
        JobBo job = new JobBo();
        job.setStatus(JobStatusEnum.SUCCESS.name());
        return job;
    }

    public static JobBo createFailJob() {
        JobBo job = new JobBo();
        job.setStatus(JobStatusEnum.FAIL.name());
        return job;
    }

    public static JobBo createRunningJob() {
        JobBo job = new JobBo();
        job.setStatus(JobStatusEnum.RUNNING.name());
        return job;
    }
}
