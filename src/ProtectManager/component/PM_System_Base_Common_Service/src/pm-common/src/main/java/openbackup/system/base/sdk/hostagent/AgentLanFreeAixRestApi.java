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
package openbackup.system.base.sdk.hostagent;

import openbackup.system.base.common.rest.CommonFeignConfiguration;
import openbackup.system.base.sdk.hostagent.model.AgentLanFreeAixDTO;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.ResponseBody;

import javax.validation.constraints.NotEmpty;

/**
 * system backup service
 *
 */
@FeignClient(name = "AgentLanFreeAixRestApi", url = "${service.url.pm-system-base}/v1/internal/host-agent",
        configuration = CommonFeignConfiguration.class)
public interface AgentLanFreeAixRestApi {
    /**
     * 根据agent id获取LAN-FREE信息
     *
     * @param resourceId 主机资源id
     * @return 页面信息
     */
    @ExterAttack
    @GetMapping(value = "/{agentId}/aix/lan-free/configuration")
    @ResponseBody
    ResponseEntity<AgentLanFreeAixDTO> getLanFreeByAgentId(@NotEmpty @PathVariable(name = "agentId") String resourceId);
}
