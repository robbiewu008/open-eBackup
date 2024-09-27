/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.exchange.protection.access.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupChangeCauseEnum;
import openbackup.data.protection.access.provider.sdk.backup.NextBackupModifyReq;
import openbackup.data.protection.access.provider.sdk.backup.v2.PostBackupTask;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.enums.BackupTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.exchange.protection.access.constant.ExchangeConstant;
import openbackup.exchange.protection.access.service.ExchangeService;
import com.huawei.oceanprotect.job.sdk.JobService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

/**
 * ExchangeServiceImpl
 *
 * @author s30036254
 * @since 2023-04-28
 */
@Service
@Slf4j
public class ExchangeServiceImpl implements ExchangeService {
    // 资源扫描任务类型
    private static final List<String> SCAN_JOB_TYPES = Arrays.asList(
        JobTypeEnum.RESOURCE_SCAN.getValue(), JobTypeEnum.RESOURCE_RESCAN.getValue());

    @Autowired
    private ResourceService resourceService;

    @Autowired
    private AgentUnifiedService agentUnifiedService;

    @Autowired
    private CopyRestApi copyRestApi;

    @Autowired
    private JobService jobService;

    /**
     * 获取Agent环境信息
     *
     * @param envId 环境uuid
     * @return Agent环境信息
     */
    @Override
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return resourceService.getResourceById(envId)
            .filter(env -> env instanceof ProtectedEnvironment)
            .map(env -> (ProtectedEnvironment) env)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected environment is not exists!"));
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param uuid 资源uuid
     * @return ProtectedResource 资源信息
     */
    @Override
    public ProtectedResource getResourceById(String uuid) {
        return resourceService.getResourceById(uuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
    }

    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId
     * @param status status
     */
    @Override
    public void updateResourceLinkStatus(String resourceId, String status) {
        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resourceId);
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
    }

    /**
     * 检查连通性
     *
     * @param environment ProtectedEnvironment
     */
    @Override
    public void checkConnection(ProtectedEnvironment environment) {
        environment.getDependencies()
            .get(ExchangeConstant.EXCHANGE_AGENTS)
            .forEach(agent -> singleConnectCheck(agent.getUuid(), environment));
    }

    /**
     * 调用插件接口查询相关信息
     *
     * @param environment protectedResource
     * @param oldEnvironment oldEnvironment
     * @return 集群信息
     */
    @Override
    public AppEnvResponse queryClusterInfo(ProtectedEnvironment environment, ProtectedEnvironment oldEnvironment) {
        List<ProtectedEnvironment> agents = environment.getDependencies()
            .get(ExchangeConstant.EXCHANGE_AGENTS)
            .stream()
            .map(agent -> getEnvironmentById(agent.getUuid()))
            .collect(Collectors.toList());
        // 单机
        if (ExchangeConstant.EXCHANGE_SINGLE.equals(environment.getExtendInfoByKey("isGroup"))) {
            return checkSingle(environment, agents);
        } else if (ExchangeConstant.EXCHANGE_GROUP.equals(environment.getExtendInfoByKey("isGroup"))) {
            // dag组
            return checkGroup(environment, agents, oldEnvironment);
        } else {
            return new AppEnvResponse();
        }
    }

    private AppEnvResponse checkSingle(ProtectedEnvironment environment, List<ProtectedEnvironment> agents) {
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(environment, agents.get(0));
        if (appEnvResponse.getExtendInfo().get("access") != null) {
            throw new LegoCheckedException(CommonErrorCode.EXCHANGE_USER_NO_ACCESS, "Exchange user has no access.");
        }
        String serverNums = appEnvResponse.getExtendInfo().get("member_server_sum");
        String dagUuid = appEnvResponse.getExtendInfo().get("dag_uuid");
        if (!StringUtils.isEmpty(serverNums) || !StringUtils.isEmpty(dagUuid)) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_TYPE_INCONSISTENT,
                "The host belongs to the dag group.");
        }
        return appEnvResponse;
    }

    private AppEnvResponse checkGroup(ProtectedEnvironment environment, List<ProtectedEnvironment> agents,
        ProtectedEnvironment oldEnvironment) {
        Set<String> uniqueGuid = new HashSet<>();
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        int agentSize = environment.getDependencies().get(ExchangeConstant.EXCHANGE_AGENTS).size();
        if (agentSize < 2 || agentSize > 16) {
            throw new LegoCheckedException(CommonErrorCode.EXCHANGE_DAG_HOSTS_INCONSISTENT,
                "The number of hosts in the DAG group does not meet the requirements.");
        }
        for (ProtectedEnvironment agent : agents) {
            appEnvResponse = agentUnifiedService.getClusterInfo(environment, agent);
            if (appEnvResponse.getExtendInfo().get("access") != null) {
                throw new LegoCheckedException(CommonErrorCode.EXCHANGE_USER_NO_ACCESS, "Exchange user has no access.");
            }
            String serverNums = appEnvResponse.getExtendInfo().get("member_server_sum");
            String dagUuid = appEnvResponse.getExtendInfo().get("dag_uuid");
            if (StringUtils.isEmpty(serverNums) || StringUtils.isEmpty(dagUuid)) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_TYPE_INCONSISTENT,
                    "The host belongs to the dag group.");
            }

            if (agentSize != Integer.parseInt(serverNums)) {
                log.info("the exchange dag has {} servers but have {} agents", serverNums, agentSize);
                throw new LegoCheckedException(CommonErrorCode.EXCHANGE_DAG_HOSTS_INCONSISTENT,
                    "The number of selected hosts is inconsistent.");
            }
            uniqueGuid.add(dagUuid);
        }
        if (uniqueGuid.size() != 1) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_DAG,
                "Selected hosts not in the same DAG group.");
        }
        // 修改DAG组时不允许替换DAG
        if (oldEnvironment != null && !uniqueGuid.contains(oldEnvironment.getExtendInfoByKey("dag_uuid"))) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_DAG,
                "Selected hosts not in the same DAG group.");
        }
        return appEnvResponse;
    }

    /**
     * 获取单个节点的连通性
     *
     * @param agentId agent uuid
     * @param environment 受保护环境
     */
    public void singleConnectCheck(String agentId, ProtectedEnvironment environment) {
        // 通过agentId得到具体要使用的agentEnvironment来获得ip+port
        ProtectedEnvironment agentEnvironment = getEnvironmentById(agentId);
        AgentBaseDto check = agentUnifiedService.check(environment.getSubType(), agentEnvironment,
            CheckAppReq.buildFrom(environment));
        if (Long.parseLong(check.getErrorCode()) != 0) {
            log.error("Exchange check fail. ip: {}", agentEnvironment.getEndpoint());
            ActionResult actionResult = JsonUtil.read(check.getErrorMessage(), ActionResult.class);
            long errorCode = Long.parseLong(actionResult.getBodyErr());
            throw new LegoCheckedException(errorCode, "Exchange check fail.");
        }
    }

    /**
     * 判断是否已经存在副本
     *
     * @param resourceId 资源id
     * @return 是否存在副本
     */
    @Override
    public Boolean isExistCopy(String resourceId) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.COPY_GENERATED_BY, CopyGeneratedByEnum.BY_BACKUP.value());
        Copy copy = copyRestApi.queryLatestBackupCopy(resourceId, null, filter);
        // 首次备份没有副本
        if (copy == null) {
            log.info("Last copy is not exists!");
            return Boolean.FALSE;
        }
        return Boolean.TRUE;
    }

    /**
     * 若上一次日志备份失败, 设置下一次备份为全量备份
     *
     * @param postBackupTask 任务参数
     */
    @Override
    public void setNextBackupTypeWhenLogBackFail(PostBackupTask postBackupTask) {
        // 如若上一次日志备份失败, 设置下一次备份为全量备份
        if (BackupTypeEnum.LOG.getAbbreviation() == postBackupTask.getBackupType().getAbbreviation()
            && ProviderJobStatusEnum.FAIL.equals(postBackupTask.getJobStatus())) {
            log.info("Tpops gauss db last log backup task failed! Start to set next backup type");
            NextBackupModifyReq nextBackupModifyReq = NextBackupModifyReq.build(
                postBackupTask.getProtectedObject().getResourceId(),
                NextBackupChangeCauseEnum.VERIFY_FAILED_TO_FULL_LABEL);
            resourceService.modifyNextBackup(nextBackupModifyReq);
        }
    }

    /**
     * 查询最新的副本
     *
     * @param resourceId 资源id
     * @return 副本
     */
    @Override
    public Optional<Copy> getLatestCopy(String resourceId) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.COPY_GENERATED_BY, CopyGeneratedByEnum.BY_BACKUP.value());
        Copy copy = copyRestApi.queryLatestBackupCopy(resourceId, null, filter);
        return Optional.ofNullable(copy);
    }

    @Override
    public List<ProtectedResource> scanMailboxes(ProtectedEnvironment environment, ProtectedResource agentEnvironment,
        ProtectedResource database) {
        List<ProtectedResource> mailboxes = new ArrayList<>();
        AtomicInteger page = new AtomicInteger(0);
        Map<String, String> conditions = new HashMap<>();
        conditions.put("database", database.getName());
        AtomicReference<PageListResponse<ProtectedResource>> response = new AtomicReference<>();
        do {
            response.set(agentUnifiedService.getDetailPageList(ResourceSubTypeEnum.EXCHANGE_GROUP.getType(),
                agentEnvironment.getEndpoint(), agentEnvironment.getPort(),
                generateListResourceV2ReqWithConditions(page.getAndIncrement(), ExchangeConstant.QUERY_SIZE,
                    environment, agentEnvironment, JsonUtil.json(conditions))));
            mailboxes.addAll(response.get().getRecords());
        } while (response.get().getRecords().size() == ExchangeConstant.QUERY_SIZE);
        // 处理返回的结果
        for (ProtectedResource resource : mailboxes) {
            // 邮箱的uuid使用 guid + 环境uuid生成，避免不同环境相同的guid
            String uuidSource = resource.getExtendInfoByKey("ExchangeGuid") + environment.getUuid();
            resource.setUuid(UUID.nameUUIDFromBytes(uuidSource.getBytes(Charset.defaultCharset())).toString());
            resource.setRootUuid(environment.getUuid());
            resource.setParentName(database.getName());
            resource.setParentUuid(database.getUuid());
            resource.setVersion(database.getVersion());
            resource.setPath(environment.getEndpoint());
        }
        log.info("scan exchange database is {}, mailbox result size is {}", database.getName(), mailboxes.size());
        return mailboxes;
    }

    @Override
    public void checkDomainUser(ProtectedEnvironment environment) {
        if (!environment.getAuth().getAuthKey().contains("@") && !environment.getAuth().getAuthKey().contains("\\")) {
            log.info("exchange register user doesn't contain both @ and \\");
            throw new LegoCheckedException(CommonErrorCode.EXCHANGE_DOMAIN_USERNAME_INVALID,
                "Exchange domain username invalid.");
        }
    }

    private ListResourceV2Req generateListResourceV2ReqWithConditions(int page, int size,
        ProtectedEnvironment environment, ProtectedResource agentResource, String conditions) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        environment.setSubType(ResourceSubTypeEnum.EXCHANGE_GROUP.getType());
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(size);
        listResourceV2Req.setPageNo(page);
        agentResource.setSubType(ResourceSubTypeEnum.EXCHANGE_GROUP.getType());
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(agentResource, Application::new)));
        listResourceV2Req.setConditions(conditions);
        return listResourceV2Req;
    }

    @Override
    public void checkMaxConcurrentJobNumber(ProtectedEnvironment env) {
        Map<String, String> extendInfo = env.getExtendInfo();
        String maxConcurrentJobNumber = extendInfo.get(ExchangeConstant.MAX_CONCURRENT_JOB_NUMBER);
        if (VerifyUtil.isEmpty(maxConcurrentJobNumber)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "exchange maxConcurrentJobNumber is empty");
        }
        try {
            int num = Integer.parseInt(maxConcurrentJobNumber);
            if (num < ExchangeConstant.MIN_JOB_NUMBER || num > ExchangeConstant.MAX_JOB_NUMBER) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "exchange maxConcurrentJobNumber is over limit");
            }
        } catch (NumberFormatException e) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "exchange maxConcurrentJobNumber is not a number");
        }
        log.info("env {} check maxConcurrentJobNumber: {}, passed!", env.getUuid(), maxConcurrentJobNumber);
    }

    /**
     * 获取已经注册的主机uuid和agent端口集合
     *
     * @param environment 注册的单机或集群环境
     * @return 主机uuid和实例端口集合
     */
    @Override
    public Set<String> getExistingUuid(ProtectedEnvironment environment) {
        // 获取已经存在的Exchange环境信息
        List<ProtectedEnvironment> existingEnv = new ArrayList<>();
        existingEnv.addAll(getExistingEnvironments(ResourceSubTypeEnum.EXCHANGE_GROUP.getType()));
        existingEnv.addAll(getExistingEnvironments(ResourceSubTypeEnum.EXCHANGE_SINGLE_NODE.getType()));

        if (!VerifyUtil.isEmpty(environment.getUuid())) {
            existingEnv.removeIf(env -> Objects.equals(environment.getUuid(), env.getUuid()));
        }
        Set<String> agents = new HashSet<>();
        for (ProtectedEnvironment protectedEnvironment : existingEnv) {
            String agentUUid = protectedEnvironment.getExtendInfo().get("agentUuid");
            if (agentUUid.contains(";")) {
                agents.addAll(Arrays.stream(agentUUid.split(";")).collect(Collectors.toList()));
            } else {
                agents.add(agentUUid);
            }
        }
        return agents;
    }

    private List<ProtectedEnvironment> getExistingEnvironments(String subType) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("type", ResourceTypeEnum.DATABASE.getType());
        conditions.put("subType", subType);
        List<ProtectedResource> existingResources = resourceService.query(0, ExchangeConstant.EXCHANGE_MAX_COUNT,
            conditions).getRecords();

        // 转成ProtectedEnvironment环境对象
        return existingResources.stream()
            .filter(existingResource -> existingResource instanceof ProtectedEnvironment)
            .map(existingResource -> (ProtectedEnvironment) existingResource)
            .collect(Collectors.toList());
    }

    /**
     * 将nodes转换为NodeInfo
     *
     * @param environment 环境信息
     * @return 节点信息列表
     */
    @Override
    public List<NodeInfo> getNodeInfoFromNodes(ProtectedEnvironment environment) {
        JSONArray jsonArray = JSONArray.fromObject(environment.getExtendInfo().get(ExchangeConstant.NODES));
        return JSONArray.toCollection(jsonArray, NodeInfo.class);
    }

    /**
     * 拼接主机uuid和实例端口
     *
     * @param uuid 主机uuid
     * @param port 实例端口
     * @return 拼接后的uuid和实例端口
     */
    @Override
    public String connectUuidAndPort(String uuid, String port) {
        return uuid + ExchangeConstant.UUID_INSTANCE_PORT_SPLIT_CHAR + port;
    }

    /**
     * 从单实例的dependency里，获取对应的Agent主机
     *
     * @param instance 单实例
     * @return Agent主机信息
     */
    @Override
    public ProtectedEnvironment queryAgentEnvironment(ProtectedResource instance) {
        return getEnvironmentById(instance.getUuid());
    }

    /**
     * 删除资源时校验是否存在正在运行的任务，存在的话抛出错误码
     *
     * @param resource 资源
     */
    @Override
    public void checkCanDeleteResource(ProtectedResource resource) {
        List<String> unfinishedJobStatus = new ArrayList<>();
        JobStatusEnum.getUnfinishedStatusList().forEach(jobStatusEnum -> unfinishedJobStatus.add(jobStatusEnum.name()));
        Integer jobCount = jobService.getJobCount(SCAN_JOB_TYPES, unfinishedJobStatus,
            Collections.singletonList(resource.getUuid()));
        if (jobCount > 0) {
            log.error("exchange delete resource {} query running job count: {}", resource.getUuid(), jobCount);
            throw new LegoCheckedException(CommonErrorCode.HAVE_RUNNING_JOB, "resource " + resource.getUuid()
                + " has running job");
        }
    }
}
