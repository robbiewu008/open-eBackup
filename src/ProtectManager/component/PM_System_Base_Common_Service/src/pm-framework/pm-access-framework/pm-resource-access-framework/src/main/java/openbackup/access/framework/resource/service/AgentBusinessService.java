/*
 *
 *  Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 */

package openbackup.access.framework.resource.service;

import openbackup.access.framework.resource.dto.DeliverTaskReq;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;

import java.util.List;

/**
 * 对接agent的业务service
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2022-12-27
 */
public interface AgentBusinessService {
    /**
     * 传递任务状态
     *
     * @param deliverTaskReq 传递任务状态请求体
     */
    void deliverTaskStatus(DeliverTaskReq deliverTaskReq);

    /**
     * 查询内置agent
     *
     * @return internal agent list
     */
    List<Endpoint> queryInternalAgents();

    /**
     * 查询内置agent
     *
     * @return internal agent env list
     */
    List<ProtectedEnvironment> queryInternalAgentEnv();
}
