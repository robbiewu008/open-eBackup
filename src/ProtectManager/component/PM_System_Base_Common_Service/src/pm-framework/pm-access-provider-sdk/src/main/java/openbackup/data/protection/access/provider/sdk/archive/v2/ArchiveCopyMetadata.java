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
package openbackup.data.protection.access.provider.sdk.archive.v2;

/**
 * 更新归档副本元数据请求对象
 *
 **/
public class ArchiveCopyMetadata {
    /**
     * 任务id
     */
    private String taskId;

    /**
     * 副本元数据，副本对象的json串
     */
    private String pmMetadata;

    public String getTaskId() {
        return taskId;
    }

    public void setTaskId(String taskId) {
        this.taskId = taskId;
    }

    public String getPmMetadata() {
        return pmMetadata;
    }

    public void setPmMetadata(String pmMetadata) {
        this.pmMetadata = pmMetadata;
    }
}
