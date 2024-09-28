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
package openbackup.data.access.client.sdk.api.framework.dme.replicate;

import openbackup.data.access.client.sdk.api.config.achive.DmeArchiveFeignConfiguration;
import openbackup.data.access.client.sdk.api.config.achive.DmeResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.ResponseBody;

/**
 * DME 高级复制接口
 *
 */
@FeignClient(name = "dmeReplicateService", url = "${services.endpoints.protectengine.replication}",
        configuration = DmeArchiveFeignConfiguration.class)
public interface DmeReplicateService {
    /**
     * 终止复制任务
     *
     * @param advanceReplicationJob advanceReplicationStopJob
     * @return response
     */
    @ExterAttack
    @PostMapping("/v1/dme_replication/task/abort")
    @ResponseBody
    DmeResponse<String> abortReplicationTask(@RequestBody AdvanceReplicationJob advanceReplicationJob);
}
