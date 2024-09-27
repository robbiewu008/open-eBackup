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
package openbackup.data.access.framework.protection.service.job;

import com.huawei.oceanprotect.coordinator.constans.DispatchConstant;
import openbackup.data.access.framework.core.dao.ProtectedObjectMapper;
import openbackup.data.access.framework.core.entity.ProtectedObjectPo;
import openbackup.data.access.framework.protection.common.util.JobExtendInfoUtil;
import openbackup.data.protection.access.provider.sdk.job.JobProvider;
import com.huawei.oceanprotect.job.constants.JobExtendInfoKeys;
import com.huawei.oceanprotect.job.dto.JobSlaDetail;
import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import openbackup.system.base.common.model.job.Job;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import com.baomidou.mybatisplus.core.conditions.query.LambdaQueryWrapper;
import com.fasterxml.jackson.databind.JsonNode;
import com.fasterxml.jackson.databind.node.ObjectNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * UnifiedBackupJobProvider
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-23
 */
@Slf4j
@Component("unifiedBackupJobProvider")
public class UnifiedBackupJobProvider implements JobProvider {
    private static final List<String> EXCLUDE_KEYS = Collections.unmodifiableList(
        Arrays.asList(DispatchConstant.FIRST_BACKUP_ESN, DispatchConstant.LAST_BACKUP_ESN,
            DispatchConstant.PRIORITY_BACKUP_ESN, DispatchConstant.FIRST_BACKUP_TARGET,
            DispatchConstant.LAST_BACKUP_TARGET, DispatchConstant.PRIORITY_BACKUP_TARGET));

    @Autowired
    @Qualifier("unifiedJobProvider")
    private JobProvider unifiedJobProvider;

    @Autowired
    private SlaQueryService slaQueryService;

    @Autowired
    private ProtectedObjectMapper protectedObjectMapper;

    @Override
    public boolean applicable(String jobType) {
        return JobTypeEnum.BACKUP.getValue().equals(jobType);
    }

    @Override
    public void stopJob(String associativeId) {
        unifiedJobProvider.stopJob(associativeId);
    }

    @Override
    public void fillJobInfo(Job insertJob) {
        JobExtendInfoUtil.fillJobPolicyInfo(insertJob, slaQueryService::querySlaById,
            ext -> JobExtendInfoUtil.getExtInfo(ext, JobExtendInfoKeys.SLA_ID), JobSlaDetail.class,
            jobSlaDetail -> fillProtectedObjEx(insertJob, jobSlaDetail));
    }

    private void fillProtectedObjEx(Job insertJob, JobSlaDetail jobSlaDetail) {
        LambdaQueryWrapper<ProtectedObjectPo> lambdaQueryWrapper = new LambdaQueryWrapper<>();
        lambdaQueryWrapper.select(ProtectedObjectPo::getExtParameters);
        lambdaQueryWrapper.eq(ProtectedObjectPo::getUuid, insertJob.getSourceId());
        ProtectedObjectPo protectedObjectPo = protectedObjectMapper.selectOne(lambdaQueryWrapper);
        getProtectedObjExt(protectedObjectPo.getExtParameters()).ifPresent(jobSlaDetail::setProtectedObjExtParam);
    }

    private Optional<JsonNode> getProtectedObjExt(String extString) {
        JsonNode extNode = JsonUtil.read(extString, JsonNode.class);
        while (extNode != null && extNode.isTextual()) {
            extNode = JsonUtil.read(extNode.textValue(), JsonNode.class);
        }
        if (extNode == null) {
            return Optional.empty();
        }
        ObjectNode objectNode = JsonUtil.cast(extNode, ObjectNode.class);
        objectNode.remove(EXCLUDE_KEYS);
        return Optional.of(extNode);
    }
}
