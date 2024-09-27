package openbackup.tpops.protection.access.provider;

import openbackup.data.protection.access.provider.sdk.agent.AgentLinkStatusProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-14
 */
public class TpopsGaussDBAgentLinkStatusProvider implements AgentLinkStatusProvider {
    @Override
    public List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.OFFLINE);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.TPOPS_GAUSSDB_INSTANCE.getType().equals(object.getSubType());
    }
}
