/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import java.util.Arrays;
import java.util.List;

/**
 * 根据资源选择获取连接状态排序列表
 *
 * @author y30044273
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-08-04
 */
public interface AgentLinkStatusProvider extends DataProtectionProvider<ProtectedResource> {
    /**
     * 获取agent的状态优先级排序
     *
     * @return agent的连接状态优先级排序
     */
    default List<LinkStatusEnum> getLinkStatusOrderList() {
        return Arrays.asList(LinkStatusEnum.ONLINE, LinkStatusEnum.OFFLINE);
    }
}
