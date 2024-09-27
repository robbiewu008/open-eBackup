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
package openbackup.data.access.framework.copy.verify.service;

import openbackup.data.protection.access.provider.sdk.verify.CopyVerifyTask;
import openbackup.system.base.sdk.copy.model.Copy;

/**
 * 副本校验任务上下文
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/30
 **/
public class CopyVerifyManagerContext {
    private Copy copy;

    private String requestId;

    private boolean isSubTask;

    private CopyVerifyTask task;

    public void setCopy(Copy copyInfo) {
        this.copy = copyInfo;
    }

    public Copy getCopy() {
        return copy;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setSubTask(boolean isSubTask) {
        this.isSubTask = isSubTask;
    }

    public boolean isSubTask() {
        return isSubTask;
    }

    public void setTask(CopyVerifyTask task) {
        this.task = task;
    }

    public CopyVerifyTask getTask() {
        return task;
    }
}
