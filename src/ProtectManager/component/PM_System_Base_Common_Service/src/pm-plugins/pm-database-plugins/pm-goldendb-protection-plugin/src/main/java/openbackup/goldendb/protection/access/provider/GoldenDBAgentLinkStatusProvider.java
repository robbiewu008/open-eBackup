/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.provider;

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
public class GoldenDBAgentLinkStatusProvider implements AgentLinkStatusProvider {
    @Override
    public List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.PARTLY_ONLING, LinkStatusEnum.OFFLINE);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType().equals(object.getSubType());
    }
}
