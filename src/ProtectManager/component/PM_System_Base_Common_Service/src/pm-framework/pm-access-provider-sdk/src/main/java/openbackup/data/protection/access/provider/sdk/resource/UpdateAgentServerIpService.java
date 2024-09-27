/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.system.base.common.model.host.ManagementIp;

/**
 * 内部接口 更新AgentServerIp
 *
 * @author swx1010572
 * @since 2022-07-22
 */
public interface UpdateAgentServerIpService {
    /**
     * 更新AgentServerIp
     *
     * @param managementIp serverIp列表
     */
    void updateAgentServer(ManagementIp managementIp);
}
