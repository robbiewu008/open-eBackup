package openbackup.opengauss.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.agent.AgentLinkStatusProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.Arrays;
import java.util.List;

/**
 * OpenGauss AgentLinkStatusProvider
 *
 * @author dwx1009286
 * @version [DataBackup 1.5.0]
 * @since 2023-08-04
 */
public class OpenGaussAgentLinkStatusProvider implements AgentLinkStatusProvider {
    @Override
    public boolean applicable(ProtectedResource protectedResource) {
        return ResourceSubTypeEnum.OPENGAUSS.equalsSubType(protectedResource.getSubType());
    }

    @Override
    public List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.DEGRADED, LinkStatusEnum.UNAVAILABLE);
    }
}