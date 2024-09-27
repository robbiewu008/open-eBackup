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
package openbackup.data.access.framework.copy.mng.provider;

import openbackup.data.protection.access.provider.sdk.job.JobCallbackProvider;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.CopyInfo;
import openbackup.system.base.sdk.copy.model.CopyStatus;
import openbackup.system.base.sdk.copy.model.CopyStatusUpdateParam;
import openbackup.system.base.sdk.job.model.JobTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 副本删除、过期的长时间任务失败的回调
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-26
 */
@Component
@Slf4j
public class CopyDeleteJobCallbackProvider implements JobCallbackProvider {
    private static final List<String> COPY_DELETE_JOB_LIST = Arrays.asList(JobTypeEnum.COPY_DELETE.getValue(),
        JobTypeEnum.COPY_EXPIRE.getValue());

    private final CopyRestApi copyRestApi;

    public CopyDeleteJobCallbackProvider(CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }


    @Override
    public boolean applicable(String jobType) {
        return COPY_DELETE_JOB_LIST.contains(jobType);
    }

    @Override
    public void doCallback(JobBo job) {
        if (VerifyUtil.isEmpty(job.getCopyId())) {
            return;
        }
        CopyInfo copyInfo = copyRestApi.queryCopyByID(job.getCopyId(), false);
        if (copyInfo == null) {
            log.info("No copy: {} exist, job_id: {}", job.getCopyId(), job.getJobId());
            return;
        }
        CopyStatusUpdateParam updateParam = new CopyStatusUpdateParam();
        updateParam.setStatus(CopyStatus.INVALID);
        copyRestApi.updateCopyStatus(job.getCopyId(), updateParam);
    }
}
