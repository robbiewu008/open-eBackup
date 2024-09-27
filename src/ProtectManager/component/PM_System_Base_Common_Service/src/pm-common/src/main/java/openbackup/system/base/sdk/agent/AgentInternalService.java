/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.sdk.agent;

import java.util.List;

/**
 * Agent服务
 *
 * @author w00504341
 * @since 2023-07-21
 */
public interface AgentInternalService {
    /**
     * 查询节点上配了LAN-FREE的Agent
     *
     * @param esn esn
     * @return 集群节点数量
     */
    List<String> queryLanFreeAgentByClusterNode(String esn);
}
