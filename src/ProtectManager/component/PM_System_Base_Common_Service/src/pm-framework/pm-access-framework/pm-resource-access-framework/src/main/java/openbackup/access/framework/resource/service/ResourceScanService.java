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
package openbackup.access.framework.resource.service;

import openbackup.system.base.common.model.job.JobBo;

import java.util.List;

/**
 * 资源扫描类
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-23
 */
public interface ResourceScanService {
    /**
     * 根据资源ID查询手动扫描的进行状态的任务
     *
     * @param resId 资源ID
     * @return 任务
     */
    List<JobBo> queryManualScanRunningJobByResId(String resId);

    /**
     * 分页查询手动扫描的进行状态的任务
     *
     * @param page page
     * @param size size
     * @return 任务
     */
    List<JobBo> queryManualScanRunningPage(int page, int size);

    /**
     * 任务是否完结
     *
     * @param jobId jobId
     * @return boolean
     */
    boolean jobIsFinished(String jobId);
}
