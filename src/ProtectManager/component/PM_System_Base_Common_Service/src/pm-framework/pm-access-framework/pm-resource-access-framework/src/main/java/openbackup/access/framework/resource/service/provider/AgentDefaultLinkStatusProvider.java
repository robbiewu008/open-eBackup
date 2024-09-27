package openbackup.access.framework.resource.service.provider;

import openbackup.data.protection.access.provider.sdk.agent.AgentLinkStatusProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import org.springframework.stereotype.Component;

/**
 * agent 连接状态默认获得
 *
 * @author y30044273
 * @since 2023-08-16
 */
@Component
public class AgentDefaultLinkStatusProvider implements AgentLinkStatusProvider {
    @Override
    public boolean applicable(ProtectedResource object) {
        return false;
    }
}
