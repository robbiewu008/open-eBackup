package openbackup.system.base.sdk.agent;

import openbackup.system.base.sdk.agent.config.AgentConfiguration;
import openbackup.system.base.sdk.agent.model.AgentUpdateRequest;
import openbackup.system.base.sdk.agent.model.AgentUpdateResponse;
import openbackup.system.base.sdk.agent.model.AgentUpdateResultResponse;
import openbackup.system.base.security.exterattack.ExterAttack;

import org.springframework.cloud.openfeign.FeignClient;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import java.net.URI;

/**
 * 功能描述
 *
 * @author s00455050
 * @since 2021-08-06
 */
@FeignClient(name = "AgentUpdateClient", url = "${pm-resource-manager.url}/v1/internal/agent/",
    configuration = AgentConfiguration.class)
public interface AgentUpdateClient {
    /**
     * update agent
     *
     * @param uri 用户指定的请求前缀
     * @param agentUpdateRequest agentUpdateRequest
     * @return AgentUpdateResponse-agent响应
     */
    @ExterAttack
    @PostMapping("/upgrade")
    @ResponseBody
    AgentUpdateResponse updateAgent(URI uri, @RequestBody AgentUpdateRequest agentUpdateRequest);

    /**
     * Query agent update result agent update result response.
     *
     * @param uri 用户指定的请求前缀
     * @param ip the ip
     * @param port the port
     * @param canUseProxy 是否需要代理
     * @return the agent update result response
     */
    @ExterAttack
    @GetMapping("/upgrade-status")
    @ResponseBody
    AgentUpdateResultResponse queryAgentUpdateResult(URI uri, @RequestParam("ip") String ip,
            @RequestParam("port") String port, @RequestParam("canUseProxy") boolean canUseProxy);
}
