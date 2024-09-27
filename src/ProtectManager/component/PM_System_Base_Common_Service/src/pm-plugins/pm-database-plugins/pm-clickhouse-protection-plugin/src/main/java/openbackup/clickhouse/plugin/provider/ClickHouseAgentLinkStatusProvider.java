/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.clickhouse.plugin.provider;

import openbackup.data.protection.access.provider.sdk.agent.AgentLinkStatusProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-14
 */
@Component
public class ClickHouseAgentLinkStatusProvider implements AgentLinkStatusProvider {
    @Override
    public List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.PARTLY_ONLING, LinkStatusEnum.OFFLINE);
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.CLICK_HOUSE.equalsSubType(object.getSubType());
    }
}
