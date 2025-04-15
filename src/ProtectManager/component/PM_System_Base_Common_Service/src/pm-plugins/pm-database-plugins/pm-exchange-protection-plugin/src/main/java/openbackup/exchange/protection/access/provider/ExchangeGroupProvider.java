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
package openbackup.exchange.protection.access.provider;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.exchange.protection.access.service.ExchangeService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * ExchangeGroupProvider
 *
 */
@Slf4j
@Component
public class ExchangeGroupProvider extends DatabaseEnvironmentProvider {
    private final AgentUnifiedService agentUnifiedService;

    private final ExchangeService exchangeService;

    /**
     * 构造方法
     *
     * @param providerManager provider管理器
     * @param pluginConfigManager 插件配置管理器
     * @param agentUnifiedService agentUnifiedService
     * @param exchangeService exchangeService
     */
    public ExchangeGroupProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        AgentUnifiedService agentUnifiedService, ExchangeService exchangeService) {
        super(providerManager, pluginConfigManager);
        this.agentUnifiedService = agentUnifiedService;
        this.exchangeService = exchangeService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.EXCHANGE_GROUP.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check exchange group:{}", environment.getName());

        if (StringUtils.isBlank(environment.getName())
                || !Pattern.matches(ExchangeConstant.NAME_FORMAT, environment.getName())) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Illegal name.");
        }
        // 校验用户名合法性
        exchangeService.checkDomainUser(environment);
        // 校验更新已注册的资源信息的参数合法性
        ProtectedEnvironment oldEnvironment = null;
        if (StringUtils.isNotEmpty(environment.getUuid())) {
            oldEnvironment = checkUpdateResourceInfo(environment);
        }
        // 校验agent是否被注册
        checkClusterExist(environment);
        // 校验任务并发数参数
        exchangeService.checkMaxConcurrentJobNumber(environment);

        // 设置集群endpoint
        Set<String> agentEndpoints = new HashSet<>();
        Set<String> agentUuids = new HashSet<>();
        List<ProtectedResource> agents = environment.getDependencies().get("agents");
        agents.forEach(agent -> {
            ProtectedEnvironment agentEnvironment = exchangeService.getEnvironmentById(agent.getUuid());
            agentEndpoints.add(agentEnvironment.getEndpoint());
            agentUuids.add(agentEnvironment.getUuid());
        });
        if (agents.size() > 1 && ExchangeConstant.EXCHANGE_SINGLE.equals(environment.getExtendInfoByKey("isGroup"))) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "The resource type is single-node system, but there is more than one agent.");
        }

        String clusterEndpoint = agentEndpoints.stream().sorted().collect(Collectors.joining(","));
        environment.setEndpoint(clusterEndpoint);

        // 注册的时候校验集群唯一性
        if (StringUtils.isEmpty(environment.getUuid())) {
            // 生成环境资源唯一UUID,检查uuid是否已经存在
            String uuid = buildUniqueUuid(agentUuids);
            checkDuplicatedEnvironment(uuid);
            environment.setUuid(uuid);
        }
        // 连通性校验
        exchangeService.checkConnection(environment);

        // 查询接口信息 判断主机是不是在可用DAG组里面
        AppEnvResponse appEnvResponse = exchangeService.queryClusterInfo(environment, oldEnvironment);
        log.info("exchange group dag uuid is {}", appEnvResponse.getExtendInfo().get("dag_uuid"));
        environment.setExtendInfoByKey("dag_uuid", appEnvResponse.getExtendInfo().get("dag_uuid"));

        // 检查注册的管理节点是否属于同一个集群
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());

        // 设置exchange的agentUuid
        String agentUuid = agentUuids.stream().sorted().collect(Collectors.joining(";"));
        log.info("exchange agent uuid is {}", agentUuid);
        environment.setExtendInfoByKey("agentUuid", agentUuid);

        // 设置路径
        environment.setPath(environment.getEndpoint());
    }

    private void checkClusterExist(ProtectedEnvironment environment) {
        List<ProtectedResource> instances = environment.getDependencies().get(DatabaseConstants.AGENTS);
        Set<String> uuids = exchangeService.getExistingUuid(environment);
        List<String> childrenList = new ArrayList<>();
        for (ProtectedResource instance : instances) {
            String uuid = instance.getUuid();
            if (uuids.contains(uuid)) {
                log.error("The Exchange DAG exists registered instance:{}.", uuid);
                throw new LegoCheckedException(ExchangeConstant.CLUSTER_NODE_IS_REGISTERED,
                    "The Exchange DAG exists registered instance.");
            }
            if (childrenList.contains(uuid)) {
                log.error("The Exchange DAG params exists same instance:{}.", uuid);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "The Exchange DAG params exists same instance.");
            }
            childrenList.add(uuid);
        }
    }

    /**
     * 根据管理节点agent主机的uuid和资源类型生成环境的唯一UUID
     *
     * @param agentsUuids agent主机的uuid集合
     * @return 唯一UUID
     */
    private String buildUniqueUuid(Set<String> agentsUuids) {
        // 设置唯一UUID
        String uuidTag = agentsUuids.stream().sorted().collect(Collectors.joining(";"));

        /**/
        String envIdentity = ResourceSubTypeEnum.EXCHANGE_GROUP.getType() + uuidTag;
        String uuid = UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
        log.info("start to register new Exchange environment, uuid: {}", uuid);
        return uuid;
    }

    private void checkDuplicatedEnvironment(String uuid) {
        ProtectedResource resource = new ProtectedResource();
        try {
            resource = exchangeService.getResourceById(uuid);
        } catch (LegoCheckedException exception) {
            log.info("cluster is unique");
        }
        if (resource.getUuid() != null) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "The cluster has been registered.");
        }
    }

    private ProtectedEnvironment checkUpdateResourceInfo(ProtectedEnvironment environment) {
        // 不允许修改接入类型
        ProtectedEnvironment oldEnvironment = exchangeService.getEnvironmentById(environment.getUuid());
        if (!environment.getExtendInfo().get("isGroup").equals(oldEnvironment.getExtendInfo().get("isGroup"))) {
            log.error("The resource type is not allowed to be changed, the new resource type is :{},"
                    + "the old resource type is :{}", environment.getExtendInfo().get("isGroup"),
                oldEnvironment.getExtendInfo().get("isGroup"));
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "The resource type is not allowed to be changed.");
        }

        // 单节点不允许修改agent
        if (ExchangeConstant.EXCHANGE_SINGLE.equals(oldEnvironment.getExtendInfoByKey("isGroup"))) {
            if (!environment.getDependencies()
                .get("agents")
                .get(0)
                .getUuid()
                .equals(oldEnvironment.getDependencies().get("agents").get(0).getUuid())) {
                throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                    "The resource type is single-node system, agent is not allowed to be changed.");
            }
        }
        return oldEnvironment;
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        // 对于所有节点均需要查询
        log.info("start to scan exchange: {}", environment.getUuid());

        AtomicReference<PageListResponse<ProtectedResource>> response = new AtomicReference<>();
        AtomicInteger page = new AtomicInteger();
        List<ProtectedResource> allResult = new ArrayList<>();
        environment.getDependencies().get(ExchangeConstant.EXCHANGE_AGENTS).forEach(agentEnvironment -> {
            List<ProtectedResource> result = new ArrayList<>();
            List<ProtectedResource> mailboxes = new ArrayList<>();
            do {
                response.set(agentUnifiedService.getDetailPageList(ResourceSubTypeEnum.EXCHANGE_GROUP.getType(),
                    agentEnvironment.getEndpoint(), agentEnvironment.getPort(), generateListResourceV2Req(
                        page.getAndIncrement(), ExchangeConstant.QUERY_SIZE, environment, agentEnvironment)));
                result.addAll(response.get().getRecords());
            } while (response.get().getRecords().size() == ExchangeConstant.QUERY_SIZE);
            log.info("scan exchange ip is {},result size is {}", agentEnvironment.getEndpoint(), result.size());

            // 处理返回的结果
            for (ProtectedResource resource : result) {
                resource.setUuid(resource.getExtendInfoByKey("db_uuid"));
                resource.setParentUuid(environment.getUuid());
                resource.setRootUuid(environment.getUuid());
                resource.setName(resource.getExtendInfoByKey("db_name"));
                resource.setVersion(resource.getExtendInfoByKey("version"));
                resource.setParentName(environment.getName());
                resource.setPath(environment.getEndpoint());
                mailboxes.addAll(exchangeService.scanMailboxes(environment, agentEnvironment, resource));
            }
            allResult.addAll(result);
            allResult.addAll(mailboxes);
        });
        // 扫描完成后查询exchange环境是否还存在，不存在的话扫描结果无效
        try {
            exchangeService.getEnvironmentById(environment.getUuid());
        } catch (LegoCheckedException exception) {
            log.error("exchange environment {} is not exists, scan is invalid, err: {}",
                environment.getUuid(), exception.getMessage());
            return new ArrayList<>();
        }
        return allResult;
    }

    /**
     * 生成ListResourceV2Req
     *
     * @param page page
     * @param size size
     * @param environment environment
     * @param agentResource agentResource
     * @return ListResourceV2Req ListResourceV2Req
     */
    private ListResourceV2Req generateListResourceV2Req(int page, int size, ProtectedEnvironment environment,
        ProtectedResource agentResource) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        environment.setSubType(ResourceSubTypeEnum.EXCHANGE_GROUP.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(size);
        listResourceV2Req.setPageNo(page);
        agentResource.setSubType(ResourceSubTypeEnum.EXCHANGE_GROUP.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agentResource, Application::new)));
        return listResourceV2Req;
    }


    @Override
    public void validate(ProtectedEnvironment environment) {
        // 在线设置为在线（若抛出异常框架会自动设置为离线）
        log.info("Start to periodically check the health status of the Exchange.");
        exchangeService.checkConnection(environment);
    }
}
