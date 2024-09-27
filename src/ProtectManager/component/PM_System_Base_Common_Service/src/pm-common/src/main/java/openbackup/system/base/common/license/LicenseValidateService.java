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
package openbackup.system.base.common.license;

import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.license.LicenseServiceApi;
import openbackup.system.base.sdk.license.enums.FunctionEnum;
import openbackup.system.base.sdk.resource.ResourceRestApi;
import openbackup.system.base.sdk.resource.model.ResourceEntity;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Optional;

/**
 * License Validate Service
 *
 * @author l00272247
 * @since 2021-02-03
 */
@Component
public class LicenseValidateService {
    @Autowired
    private LicenseServiceApi licenseServiceApi;

    @Autowired
    private JobCenterRestApi jobCenterRestApi;

    @Autowired
    private ResourceRestApi resourceRestApi;
    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * validate
     *
     * @param function function
     * @param resourceUuid resource uuid
     * @param jobId job id
     */
    public void validate(FunctionEnum function, String resourceUuid, String jobId) {
        validate(function, resourceRestApi.queryResource(resourceUuid), jobId);
    }

    /**
     * validate
     *
     * @param function function
     * @param resource resource
     * @param jobId job id
     */
    public void validate(FunctionEnum function, ResourceEntity resource, String jobId) {
        ResourceSubTypeEnum type = ResourceSubTypeEnum.get(resource.getSubType());
        validate(function, type, jobId);
    }

    /**
     * validate license
     *
     * @param resourceType resource type
     * @param function function
     */
    public void validate(String resourceType, FunctionEnum function) {
        validate(resourceType, function, null);
    }

    /**
     * validate license
     *
     * @param resourceType resource type
     * @param function function
     * @param jobId job id
     */
    public void validate(String resourceType, FunctionEnum function, String jobId) {
        validate(function, ResourceSubTypeEnum.get(resourceType), jobId);
    }

    /**
     * validate license
     *
     * @param function function
     * @param resourceType resource type
     * @param jobId job id
     */
    public void validate(FunctionEnum function, ResourceSubTypeEnum resourceType, String jobId) {
        validate(function, resourceType, jobId, true);
    }

    /**
     * validate license
     *
     * @param function function
     * @param resourceType resource type
     * @param jobId job id
     * @param isStrict isStrict
     * @return validate result
     */
    public boolean validate(FunctionEnum function, ResourceSubTypeEnum resourceType, String jobId, boolean isStrict) {
        LegoCheckedException exception;
        try {
            // 安全一体机适配，license校验屏蔽
            if (!deployTypeService.isCyberEngine()) {
                licenseServiceApi.functionLicense(function.name(), resourceType.getType());
            }
            exception = null;
        } catch (LegoCheckedException ex) {
            exception = ex;
        }
        boolean isSuccess = exception == null;
        if (jobId != null) {
            recordJobLog(jobId, exception);
        }
        if (isSuccess || !isStrict) {
            return isSuccess;
        }
        throw exception;
    }

    private void recordJobLog(String jobId, LegoCheckedException exception) {
        JobLogBo jobLogBo = new JobLogBo();
        jobLogBo.setJobId(jobId);
        jobLogBo.setStartTime(System.currentTimeMillis());
        jobLogBo.setLogInfo("job_log_license_check_label");
        jobLogBo.setUnique(true);
        if (exception != null) {
            jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
            jobLogBo.setLogInfoParam(Collections.singletonList("job_status_fail_label"));
            jobLogBo.setLogDetail("" + exception.getErrorCode());
            String[] parameters = exception.getParameters();
            List<String> params = Optional.ofNullable(parameters).map(Arrays::asList).orElse(Collections.emptyList());
            jobLogBo.setLogDetailParam(params);
        } else {
            jobLogBo.setLevel(JobLogLevelEnum.INFO.getValue());
            jobLogBo.setLogInfoParam(Collections.singletonList("job_status_success_label"));
        }
        UpdateJobRequest request = new UpdateJobRequest();
        request.setJobLogs(Collections.singletonList(jobLogBo));
        jobCenterRestApi.updateJob(jobId, request);
    }
}
