/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.obs.plugin.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import java.util.List;

/**
 * OBS Agent相关service
 *
 */
public interface ObjectStorageAgentService {
    /**
     * 获取对象存储下对象集合
     *
     * @param environment 对象存储信息
     * @param protectedResource 资源信息
     * @param agents agent
     * @param subTypeEnum 资源类型
     * @return bucket列表
     */
    PageListResponse<ProtectedResource> getDetail(ProtectedEnvironment environment, ProtectedResource protectedResource,
        String agents, ResourceSubTypeEnum subTypeEnum);

    /**
     * 检查连通性
     *
     * @param protectedResource 对象存储信息
     * @return 校验结果
     */
    ActionResult[] checkConnection(ProtectedResource protectedResource);

    /**
     * 获取agents对应的agent列表
     *
     * @param agents agent id列表；3d6ab3a6-3aa6-4051-8e48-c028ac4de213;208d062b-02d4-4225-9048-3be19ee40340
     * @return agent列表
     */
    List<Endpoint> getObjectStorageEndpoint(String agents);

    /**
     * 查询内置agent
     *
     * @return internal agent list
     */
    List<ProtectedResource> queryBuiltInAgents();

    /**
     * 根据id查询agent
     *
     * @param ids agent的id列表
     * @return internal agent list
     */
    List<ProtectedResource> queryAgents(List<String> ids);
}
