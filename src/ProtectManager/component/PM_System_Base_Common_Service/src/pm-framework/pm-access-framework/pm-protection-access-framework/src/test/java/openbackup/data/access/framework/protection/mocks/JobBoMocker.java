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
package openbackup.data.access.framework.protection.mocks;

import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.job.model.request.JobMessage;

/**
 * Job数据模拟器
 *
 * @author: y00559272
 * @version: [OceanProtect A8000 1.1.0]
 * @since: 2021/12/15
 **/
public class JobBoMocker {
    public static JobBo buildRestoreJobBo(RestoreTask restoreTask){
        JobBo jobBo = new JobBo();
        jobBo.setJobId(restoreTask.getRequestId());
        JobMessage jobMessage = new JobMessage();
        jobMessage.setPayload(JSONObject.fromObject(restoreTask));
        jobBo.setMessage(JSONObject.fromObject(jobMessage).toString());
        return jobBo;
    }
}
