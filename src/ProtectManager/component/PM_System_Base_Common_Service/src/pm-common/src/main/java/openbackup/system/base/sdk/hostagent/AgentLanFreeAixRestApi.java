/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
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
 * @author hwx1164326
 * @since 2023-08-22
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
