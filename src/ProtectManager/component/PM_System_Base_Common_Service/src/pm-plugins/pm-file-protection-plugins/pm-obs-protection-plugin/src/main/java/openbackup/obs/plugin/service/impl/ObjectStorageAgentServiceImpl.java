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
package openbackup.obs.plugin.service.impl;

import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.obs.plugin.service.ObjectStorageAgentService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.SymbolConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.query.SessionService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * OBS Agent相关service
 *
 * @author c30035089
 * @since 2023-11-16
 */

@Slf4j
@Service
public class ObjectStorageAgentServiceImpl implements ObjectStorageAgentService {
    private static final int SIZE = 100;

    private static final String TYPE = "type";

    // 内置agent的key
    private static final String INTERNAL_AGENT_KEY = "scenario";

    // 内置agent的value
    private static final String INTERNAL_AGENT_VALUE = "1";

    @Autowired
    private SessionService sessionService;

    @Autowired
    private AgentUnifiedService agentService;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private AgentBusinessService agentBusinessService;

    @Override
    public PageListResponse<ProtectedResource> getDetail(ProtectedEnvironment environment,
        ProtectedResource protectedResource, String agents, ResourceSubTypeEnum subTypeEnum) {
        ListResourceV2Req listResourceReq = new ListResourceV2Req();
        listResourceReq.setPageNo(Integer.parseInt(protectedResource.getExtendInfoByKey("pageNo")));
        listResourceReq.setPageSize(Integer.parseInt(protectedResource.getExtendInfoByKey("pageSize")));
        listResourceReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceReq.setApplications(Lists.newArrayList(BeanTools.copy(protectedResource, Application::new)));

        List<Endpoint> endpoints = getObjectStorageEndpoint(agents);
        List<PageListResponse<ProtectedResource>> allResult = new ArrayList<>();
        for (Endpoint endpoint : endpoints) {
            try {
                PageListResponse<ProtectedResource> result = agentService.getDetailPageList(subTypeEnum.getType(),
                    endpoint.getIp(), endpoint.getPort(), listResourceReq);
                if (result.getRecords().size() > 0) {
                    return result;
                } else {
                    allResult.add(result);
                }
            } catch (LegoCheckedException | LegoUncheckedException e) {
                log.error("get buckets error, endpoint: {}", endpoint.getIp(), ExceptionUtil.getErrorMessage(e));
            }
        }
        if (CollectionUtils.isNotEmpty(allResult)) {
            return allResult.get(0);
        }
        throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "get buckets detail failed");
    }

    @Override
    public List<Endpoint> getObjectStorageEndpoint(String agents) {
        log.info("object storage agent:{}", agents);
        if (StringUtils.isEmpty(agents)) {
            return agentBusinessService.queryInternalAgents();
        } else {
            return getEndpointList(agents);
        }
    }

    private List<Endpoint> getEndpointList(String agents) {
        return Arrays.stream(agents.split(SymbolConstant.SEMICOLON))
            .map(this::getEndpoint)
            .filter(Optional::isPresent)
            .filter(endpoint -> !(VerifyUtil.isEmpty(endpoint.get().getId())
                || VerifyUtil.isEmpty(endpoint.get().getPort()) || VerifyUtil.isEmpty(endpoint.get().getIp())))
            .map(Optional::get)
            .collect(Collectors.toList());
    }

    private Optional<Endpoint> getEndpoint(String agentUuid) {
        Optional<ProtectedResource> agentResourceOpt = resourceService.getResourceByIdIgnoreOwner(agentUuid);
        if (!agentResourceOpt.isPresent()) {
            log.error("agent not exist. agent uuid: {}.", agentUuid);
            return Optional.empty();
        }
        return agentResourceOpt.filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()));
    }

    /**
     * 检查连通性
     *
     * @param protectedResource 保护资源信息
     * @return 校验结果
     */
    @Override
    public ActionResult[] checkConnection(ProtectedResource protectedResource) {
        return resourceService.check(protectedResource);
    }

    /**
     * 查询内置agent
     *
     * @return internal agent list
     */
    public List<ProtectedResource> queryBuiltInAgents() {
        PageListResponse<ProtectedResource> response = sessionService.call(
            () -> resourceService.query(0, SIZE,
                ImmutableMap.of(TYPE, ResourceTypeEnum.HOST.getType(), INTERNAL_AGENT_KEY, INTERNAL_AGENT_VALUE)),
            Constants.Builtin.ROLE_SYS_ADMIN);
        return response.getRecords();
    }

    /**
     * 根据id查询agent
     *
     * @param ids agent的id列表
     * @return internal agent list
     */
    public List<ProtectedResource> queryAgents(List<String> ids) {
        List<Object> uuids = new ArrayList<>(ids);
        uuids.add(0, Collections.singleton("in"));
        Map<String, Object> filter = Collections.singletonMap("uuid", uuids);
        PageListResponse<ProtectedResource> response =
            sessionService.call(() -> resourceService.query(0, SIZE, filter), Constants.Builtin.ROLE_SYS_ADMIN);
        return response.getRecords();
    }
}
