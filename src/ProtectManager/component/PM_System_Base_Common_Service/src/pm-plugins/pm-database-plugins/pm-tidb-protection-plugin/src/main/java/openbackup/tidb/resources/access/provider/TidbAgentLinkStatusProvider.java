/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.provider;

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
public class TidbAgentLinkStatusProvider implements AgentLinkStatusProvider {
    @Override
    public List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.OFFLINE);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.TIDB_CLUSTER.getType().equals(object.getSubType())
            || ResourceSubTypeEnum.TIDB_DATABASE.getType().equals(object.getSubType())
            || ResourceSubTypeEnum.TIDB_TABLE.getType().equals(object.getSubType());
    }
}
