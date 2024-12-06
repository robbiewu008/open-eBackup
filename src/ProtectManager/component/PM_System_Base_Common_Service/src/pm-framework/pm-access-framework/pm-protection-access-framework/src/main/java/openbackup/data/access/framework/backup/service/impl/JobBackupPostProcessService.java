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
package openbackup.data.access.framework.backup.service.impl;

import static openbackup.system.base.common.constants.ProtectObjectExtKeyConstant.FAILED_NODE_ESN;

import com.huawei.oceanprotect.job.sdk.JobService;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.copy.mng.service.CopyService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.model.PageListResponse;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.SortingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.resource.ProtectObjectRestApi;
import openbackup.system.base.sdk.resource.model.ProtectionModifyDto;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 备份任务框架侧（不区分应用）后置处理
 *
 */
@Service
@Slf4j
public class JobBackupPostProcessService {
    private static final List<String> NODE_UNAVAILABLE_JOB_LABEL =
            Arrays.asList("agent_access_remote_storage_fail_label");

    private static final List<Long> NODE_UNAVAILABLE_ERROR_CODE = Arrays.asList(1577209958L, 1677929227L, 1593988646L);

    @Autowired
    private ProtectObjectRestApi protectObjectRestApi;

    @Autowired
    private JobService jobService;

    @Autowired
    private CopyService copyService;

    @Autowired
    private CopyRestApi copyRestApi;

    /**
     * 备份任务成功后置处理（框架侧处理，所有应用备份任务都需要）
     *
     * @param jobId 任务id
     */
    public void onBackupJobSuccess(String jobId) {
        JobBo job = jobService.queryJob(jobId);
        deleteProtectObjectExtParam(job.getSourceId());
        log.info("Job({}) success, post process finished", jobId);
    }

    /**
     * 备份任务失败后置处理（框架侧处理，所有应用备份任务都需要）
     *
     * @param jobId 任务id
     */
    public void onBackupJobFail(String jobId) {
        log.info("Job({}) failed, start post process", jobId);
        JobBo job = jobService.queryJob(jobId);
        PagingParamRequest page = new PagingParamRequest();
        page.setStartPage(IsmNumberConstant.ONE);
        page.setPageSize(IsmNumberConstant.THOUSAND);
        SortingParamRequest sorting = new SortingParamRequest();
        PageListResponse<JobLogBo> response = jobService.queryJobLogs(job.getJobId(), page, sorting,
                Collections.singletonList(JobLogLevelEnum.ERROR.getValue()));
        List<JobLogBo> records = response.getRecords();
        if (VerifyUtil.isEmpty(records)) {
            log.info("Job({}) failed, no logs found", jobId);
            return;
        }
        for (JobLogBo logBo : records) {
            if (hasNeedSkipErrorInfo(logBo)) {
                log.info("Current backup job has error(errorCode:{} ,label: {}) that need skip node({}), job id: {}",
                    logBo.getLogDetail(), logBo.getLogInfo(), job.getDeviceEsn(), jobId);
                updateProtectObjectExtParam(job.getSourceId(), job.getDeviceEsn());
                break;
            }
        }
    }

    private void updateProtectObjectExtParam(String sourceId, String deviceEsn) {
        ProtectionModifyDto protectionModifyDto = new ProtectionModifyDto();
        protectionModifyDto.setResourceId(sourceId);
        Map<String, Object> extParameters = new HashMap<>();
        extParameters.put(FAILED_NODE_ESN, deviceEsn);
        protectionModifyDto.setExtParameters(extParameters);
        protectObjectRestApi.modifyProtectedObjectExtParam(protectionModifyDto);
    }

    private void deleteProtectObjectExtParam(String sourceId) {
        ProtectionModifyDto protectionModifyDto = new ProtectionModifyDto();
        protectionModifyDto.setResourceId(sourceId);
        protectionModifyDto.setDeleteKeys(Collections.singletonList(FAILED_NODE_ESN));
        protectionModifyDto.setExtParameters(new HashMap<>());
        protectObjectRestApi.modifyProtectedObjectExtParam(protectionModifyDto);
    }

    private boolean hasNeedSkipErrorInfo(JobLogBo logBo) {
        if (NODE_UNAVAILABLE_JOB_LABEL.contains(logBo.getLogInfo())) {
            return true;
        }
        if (VerifyUtil.isEmpty(logBo.getLogDetail())) {
            return false;
        }
        return NODE_UNAVAILABLE_ERROR_CODE.stream().anyMatch(code -> code.equals(Long.parseLong(logBo.getLogDetail())));
    }
}
