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
package openbackup.oracle.service;

import com.huawei.oceanprotect.client.resource.manager.service.dto.RequestBodyDTO;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.ImmutableList;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentDetailDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppResource;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.FinalizeClearReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.oracle.bo.OracleStorage;
import openbackup.oracle.constants.OracleConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.dee.DeeDbAnonymizationRest;
import openbackup.system.base.sdk.host.model.AsmInfo;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * oracle通用service
 *
 */
@Component
@Slf4j
public class OracleBaseService {
    // agent插件返回的必要参数
    private static final List<String> REQUIRED_PARAMS = Arrays.asList(OracleConstants.ORACLE_HOME,
            OracleConstants.IS_ASM_INST, OracleConstants.DB_ROLE, OracleConstants.INST_NAME, DatabaseConstants.VERSION,
            OracleConstants.ORACLE_IP_INFOS, OracleConstants.ORACLE_GROUP, OracleConstants.ORACLE_BASE);

    private static final ImmutableList<String> SUPPORT_ANONYMIZATION_DEPLOY_TYPE =
        ImmutableList.of(DeployTypeEnum.X8000.getValue(), DeployTypeEnum.X6000.getValue(),
            DeployTypeEnum.A8000.getValue(), DeployTypeEnum.X9000.getValue());

    private final ResourceService resourceService;
    private final AgentUnifiedService agentUnifiedService;
    private final DeeDbAnonymizationRest deeDbAnonymizationRest;
    private final DeployTypeService deployTypeService;
    private final CommonAlarmService commonAlarmService;

    /**
     * 构造器
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param deeDbAnonymizationRest deeDbAnonymizationRest
     * @param deployTypeService deployTypeService
     * @param commonAlarmService commonAlarmService
     */
    public OracleBaseService(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        DeeDbAnonymizationRest deeDbAnonymizationRest, DeployTypeService deployTypeService,
        CommonAlarmService commonAlarmService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.deeDbAnonymizationRest = deeDbAnonymizationRest;
        this.deployTypeService = deployTypeService;
        this.commonAlarmService = commonAlarmService;
    }

    /**
     * 获取Agent环境信息
     *
     * @param envId 环境uuid
     * @return Agent环境信息
     */
    public ProtectedEnvironment getEnvironmentById(String envId) {
        return resourceService.getResourceById(envId)
                .filter(env -> env instanceof ProtectedEnvironment)
                .map(env -> (ProtectedEnvironment) env)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                        "Protected environment is not exists!"));
    }

    /**
     * 查询集群依赖的主机列表
     *
     * @param database 集群数据库
     * @return 主机列表
     */
    public List<ProtectedEnvironment> getOracleClusterHosts(ProtectedResource database) {
        return getEnvironmentById(database.getParentUuid())
                .getDependencies().get(DatabaseConstants.AGENTS).stream()
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .collect(Collectors.toList());
    }

    /**
     * 从单实例的dependencies里获取主机
     *
     * @param uuid 资源id
     * @return 主机信息
     */
    public ProtectedEnvironment getAgentBySingleInstanceUuid(String uuid) {
        ProtectedResource singleInstanceResources = getResource(uuid);
        Map<String, List<ProtectedResource>> dependencies = singleInstanceResources.getDependencies();
        if (!VerifyUtil.isEmpty(dependencies)) {
            List<ProtectedResource> agentResources = dependencies.get(DatabaseConstants.AGENTS);
            ProtectedResource agentResource = agentResources.get(0);
            PowerAssert.state(agentResource instanceof ProtectedEnvironment,
                () -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "oracle agent resource is not env."));
            return (ProtectedEnvironment) agentResource;
        }
        if (singleInstanceResources instanceof ProtectedEnvironment) {
            return (ProtectedEnvironment) singleInstanceResources;
        }
        ProtectedEnvironment protectedEnvironment = new ProtectedEnvironment();
        BeanTools.copy(singleInstanceResources, protectedEnvironment);
        return protectedEnvironment;
    }

    /**
     * 针对集群实例uuid，从dependency里，获取集群实例下面的所有子实例
     *
     * @param uuid 集群实例uuid
     * @return List ProtectedResource 集群实例依赖的子实例信息
     */
    public List<ProtectedResource> getSingleInstanceByClusterInstance(String uuid) {
        ProtectedResource clusterInstanceResource = getResource(uuid);
        Map<String, List<ProtectedResource>> dependencies = clusterInstanceResource.getDependencies();
        return dependencies.get(DatabaseConstants.AGENTS);
    }

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param agentEnv Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    public Endpoint getAgentEndpoint(ProtectedEnvironment agentEnv) {
        if (VerifyUtil.isEmpty(agentEnv.getUuid()) || VerifyUtil.isEmpty(agentEnv.getEndpoint())
                || VerifyUtil.isEmpty(agentEnv.getPort())) {
            throw new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "oracle agent env lack require msg.");
        }
        String osType = getOsType(agentEnv);
        return new Endpoint(agentEnv.getUuid(), agentEnv.getEndpoint(), agentEnv.getPort(), osType);
    }

    /**
     * oracle支持windows，agent配置osType为windows
     *
     * @param agentEnv 保护环境
     * @return agent的osType
     */
    public String getOsType(ProtectedEnvironment agentEnv) {
        String osType = null;
        if (OracleConstants.WINDOWS.equalsIgnoreCase(agentEnv.getOsType())) {
            osType = agentEnv.getOsType().toLowerCase();
        }
        return osType;
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param resourceId 资源uuid
     * @return ProtectedResource 资源信息
     */
    public ProtectedResource getResource(String resourceId) {
        return resourceService.getResourceById(resourceId)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource not found"));
    }

    /**
     * 针对Oracle集群实例里的所有单实例信息，将单实例信息的auth设置到单实例对应的nodes的auth中
     * 将oracleHome放置在extendInfo中
     *
     * @param nodes 备份资源对应的nodes
     * @param singleInstanceResources 集群实例对应的单实例信息
     */
    public void setNodesAuth(List<TaskEnvironment> nodes, List<ProtectedResource> singleInstanceResources) {
        HashMap<String, List<TaskEnvironment>> nodeMap = nodes.stream()
                .collect(Collectors.groupingBy(env -> env.getUuid(), HashMap::new, Collectors.toList()));
        for (ProtectedResource singleInstanceResource : singleInstanceResources) {
            ProtectedEnvironment agentEnv = getAgentBySingleInstanceUuid(singleInstanceResource.getUuid());
            final List<TaskEnvironment> taskEnvironments = nodeMap.get(agentEnv.getUuid());

            // 如果集群实例的某个单实例的agent挂掉了，那么node里就没有这个信息
            if (VerifyUtil.isEmpty(taskEnvironments)) {
                continue;
            }
            TaskEnvironment nodeEnv = taskEnvironments.get(0);
            nodeEnv.setAuth(singleInstanceResource.getAuth());
            if (nodeEnv.getExtendInfo() == null) {
                nodeEnv.setExtendInfo(new HashMap<>());
            }
            Map<String, String> nodeEnvExtendInfo = nodeEnv.getExtendInfo();
            nodeEnvExtendInfo.put(OracleConstants.ORACLE_HOME,
                    singleInstanceResource.getExtendInfo().get(OracleConstants.ORACLE_HOME));
            log.info("update node oracle instance msg success. node ip: {}", nodeEnv.getEndpoint());
        }
    }

    /**
     * 将版本信息填充到保护对象扩展信息中
     *
     * @param version 版本信息
     * @param protectObject 保护对象
     */
    public void fillVersionToExtendInfo(String version, TaskResource protectObject) {
        Map<String, String> extendInfo = Optional.ofNullable(protectObject.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(DatabaseConstants.VERSION, version);
        protectObject.setExtendInfo(extendInfo);
    }

    /**
     * 设置恢复node
     *
     * @param restoreTask 恢复对象
     */
    public void supplyNodes(RestoreTask restoreTask) {
        List<TaskEnvironment> hostList = restoreTask.getAgents()
                .stream()
                .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
                .map(AgentDtoUtil::toTaskEnvironment)
                .collect(Collectors.toList());
        restoreTask.getTargetEnv().setNodes(hostList);
    }

    /**
     * 刷新集群数据库下子数据库的信息
     *
     * @param resource 实例信息
     * @param environment 环境信息
     */
    public void refreshClusterInstanceActiveStandby(ProtectedResource resource, ProtectedEnvironment environment) {
        // 调用Agent接口，获取集群状态信息
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(resource, environment);
        if (VerifyUtil.isEmpty(appEnvResponse) || VerifyUtil.isEmpty(appEnvResponse.getExtendInfo())) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION, new String[]{},
                    "get oracle cluster error.");
        }

        log.info("oracle refresh cluster instance active standby success. ip: {}", environment.getEndpoint());
        Map<String, String> envExtendInfo = appEnvResponse.getExtendInfo();
        if (!OracleConstants.SERVICE_RUNNING.equals(envExtendInfo.get(OracleConstants.STATUS))) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION, new String[]{},
                    "oracle is not running.");
        }

        Map<String, String> resExtendInfo = resource.getExtendInfo();
        resExtendInfo.putAll(envExtendInfo);
        resExtendInfo.put(OracleConstants.VERIFY_STATUS, "true");
        resource.setVersion(envExtendInfo.get(DatabaseConstants.VERSION));
        fillAsmInfo(resource, environment);
    }

    /**
     * 更新主机asm信息
     * asm改为从resource的extendInfo中获取
     *
     * @param resource 实例信息
     * @param environment 环境信息
     */
    public void fillAsmInfo(ProtectedResource resource, ProtectedEnvironment environment) {
        ListResourceReq listResourceReq = new ListResourceReq();
        resource.getExtendInfo().put(OracleConstants.QUERY_TYPE, "asm");
        listResourceReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceReq.setApplication(BeanTools.copy(resource, Application::new));
        AgentDetailDto detail = agentUnifiedService.getDetail(resource.getSubType(), environment.getEndpoint(),
                environment.getPort(), listResourceReq);
        List<AppResource> resourceList = detail.getResourceList();
        if (VerifyUtil.isEmpty(resourceList)) {
            log.warn("Get oracle asm info fail,resource id:{}", resource.getUuid());
            return;
        }
        Map<String, String> extendInfo = resourceList.get(0).getExtendInfo();
        AsmInfo asmInfo = JSON.parseObject(JSON.toJSONString(extendInfo), AsmInfo.class);
        resource.getExtendInfo().put(OracleConstants.ASM_INFO, JSON.toJSONString(asmInfo));
    }

    /**
     * oracle适配Windows系统，下发到ubc的仓库协议需要设置为CIFS。data/log repo的extendInfo加一个参数，everyoneAuth
     * 字符串类型的"true"
     *
     * @param repositories 存储仓
     * @param environment 保护环境
     */

    public void repositoryAdaptsWindows(List<StorageRepository> repositories, ProtectedEnvironment environment) {
        List<Integer> repoTypeList = new ArrayList<>();
        repoTypeList.add(RepositoryTypeEnum.LOG.getType());
        repoTypeList.add(RepositoryTypeEnum.DATA.getType());
        repoTypeList.add(RepositoryTypeEnum.CACHE.getType());

        // 仓库为浅拷贝，引用类型需要新建对象重新赋值
        Map<String, Object> extendInfo = new HashMap<>();
        extendInfo.put("everyoneAuth", "true");
        if (OracleConstants.WINDOWS.equalsIgnoreCase(environment.getOsType())) {
            repositories.stream()
                    .filter(repo -> repoTypeList.contains(repo.getType()))
                    .forEach(repo -> {
                        if (!VerifyUtil.isEmpty(repo.getExtendInfo())) {
                            extendInfo.putAll(repo.getExtendInfo());
                        }
                        repo.setExtendInfo(extendInfo);
                    });
            repositories.forEach(repo -> repo.setProtocol(RepositoryProtocolEnum.CIFS.getProtocol()));
        }
    }

    /**
     * 查询资源是否可以删除
     *
     * @param resId 资源ID
     * @return 是否可以删除
     */
    public boolean isAnonymizationDeletable(String resId) {
        try {
            return !SUPPORT_ANONYMIZATION_DEPLOY_TYPE.contains(deployTypeService.getDeployType().getValue())
                    || !deeDbAnonymizationRest.isAnonymizationRunning(resId);
        } catch (FeignException | LegoCheckedException | LegoUncheckedException e) {
            log.error("invoke dee db anonymization api failed, resource can not be delete");
            return false;
        }
    }

    private RequestBodyDTO getRequestBodyWhenCancelProtect(ProtectedResource resource) {
        Application application = new Application();
        application.setUuid(resource.getUuid());
        application.setName(resource.getName());
        application.setParentUuid(resource.getParentUuid());
        application.setParentName(resource.getParentName());
        application.setType(resource.getType());
        application.setSubType(resource.getSubType());
        application.setExtendInfo(resource.getExtendInfo());
        application.setAuth(resource.getAuth());
        String usedStorageKey = application.getAuth().getExtendInfo().get(DatabaseConstants.USED_STORAGE_KEY);
        if (StringUtils.isNotBlank(usedStorageKey)) {
            application.getAuth().getExtendInfo().put(DatabaseConstants.STORAGES, usedStorageKey);
            application.getAuth().getExtendInfo().put(DatabaseConstants.STORAGE_PWD_KEY, application.getAuth()
                .getExtendInfo().get(DatabaseConstants.USED_STORAGE_PWD_KEY));
        }
        RequestBodyDTO reqBody = new RequestBodyDTO();
        reqBody.setApplication(application);
        return reqBody;
    }

    /**
     * 查询资源是否可以删除
     *
     * @param resource 资源信息
     */
    public void updateUsedStorageInfo(ProtectedResource resource) {
        List<OracleStorage> storages = getAuthStorages(resource);
        if (storages.size() > 0) {
            resource.getAuth().getExtendInfo().put(DatabaseConstants.USED_STORAGE_KEY,
                JSONObject.writeValueAsString(storages));
            resource.getAuth().getExtendInfo().put(DatabaseConstants.USED_STORAGE_PWD_KEY, resource.getAuth()
                .getExtendInfo().getOrDefault(DatabaseConstants.STORAGE_PWD_KEY, ""));
        }
    }

    /**
     * 清除生产存储环境
     *
     * @param resource 资源信息
     */
    public void cleanDoradoEnv(ProtectedResource resource) {
        // 解除保护的时候，存储层备份需删除生产环境上保护组
        if (StringUtils.isNotBlank(resource.getAuth().getExtendInfo().get(DatabaseConstants.STORAGES))
            || StringUtils.isNotBlank(resource.getAuth().getExtendInfo().get(DatabaseConstants.USED_STORAGE_KEY))) {
            String endpoint;
            int port;
            if (ResourceSubTypeEnum.ORACLE.getType().equals(resource.getSubType())) {
                ProtectedEnvironment agent = getAgentBySingleInstanceUuid(resource.getUuid());
                if (!String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(agent.getLinkStatus())) {
                    return;
                }
                endpoint = agent.getEndpoint();
                port = agent.getPort();
            } else {
                List<ProtectedEnvironment> agents = getOracleClusterHosts(resource);
                List<ProtectedEnvironment> onlineAgents = agents.stream()
                    .filter(agent -> String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(agent.getLinkStatus()))
                    .collect(Collectors.toList());
                if (onlineAgents.size() == 0) {
                    return;
                }
                endpoint = agents.get(0).getEndpoint();
                port = agents.get(0).getPort();
            }
            log.info("clean oracle env, endpoint: {}, port:{}", endpoint, port);
            try {
                RequestBodyDTO reqBody = getRequestBodyWhenCancelProtect(resource);
                agentUnifiedService.removeProtectUnmountRepoNoRetry(endpoint, port, resource.getSubType(),
                    JSONObject.fromObject(reqBody).toString());
            } catch (LegoCheckedException e) {
                commonAlarmService.generateAlarm(genRemoveProtectGroupAlarm(resource.getUuid()));
                log.error("remove oracle protect failed, endpoint: {}, port: {}", endpoint, port);
            }
        }
    }

    private LegoInternalAlarm genRemoveProtectGroupAlarm(String resourceId) {
        LegoInternalAlarm legoInternalAlarm = new LegoInternalAlarm();
        legoInternalAlarm.setAlarmId(OracleConstants.REMOVE_PROTECT_GROUP_FAILED_ID);
        legoInternalAlarm.setMoName(OracleConstants.REMOVE_PROTECT_GROUP_FAILED_ID);
        legoInternalAlarm.setAlarmParam(new String[] {"protect_group_" + resourceId});
        legoInternalAlarm.setAlarmSequence(IsmNumberConstant.ONE);
        legoInternalAlarm.setAlarmLevel(FaultEnum.AlarmSeverity.MAJOR);
        legoInternalAlarm.setSourceType(FaultEnum.AlarmResourceType.PROTECTION.getValue());
        return legoInternalAlarm;
    }

    private List<OracleStorage> getAuthStorages(ProtectedResource resource) {
        return Optional.of(resource)
            .map(ProtectedResource::getAuth)
            .map(Authentication::getExtendInfo)
            .map(extendInfo -> extendInfo.get(DatabaseConstants.STORAGES))
            .map(storage -> JSONArray.toCollection(JSONArray.fromObject(storage), OracleStorage.class))
            .orElse(new ArrayList<>());
    }

    /**
     * 清理生产环境日志文件
     *
     * @param resource 资源信息
     * @param extendInfo 扩展字段
     * @return AgentBaseDto
     */
    public AgentBaseDto deleteLogFromProductEnv(ProtectedResource resource, HashMap<String, String> extendInfo) {
        // oracle日志备份，UBC入库后置任务，删除生产环境的日志
        AgentBaseDto agentBaseDto = new AgentBaseDto();
        ProtectedEnvironment agentEnv;
        if (ResourceSubTypeEnum.ORACLE.getType().equals(resource.getSubType())) {
            ProtectedEnvironment agent = getAgentBySingleInstanceUuid(resource.getUuid());
            if (!String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(agent.getLinkStatus())) {
                agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NOT_EXIST));
                return agentBaseDto;
            }
            agentEnv = agent;
        } else {
            List<ProtectedEnvironment> agents = getOracleClusterHosts(resource);
            List<ProtectedEnvironment> onlineAgents = agents.stream()
                .filter(agent -> String.valueOf(LinkStatusEnum.ONLINE.getStatus()).equals(agent.getLinkStatus()))
                .collect(Collectors.toList());
            if (onlineAgents.size() == 0) {
                agentBaseDto.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NOT_EXIST));
                return agentBaseDto;
            }
            agentEnv = agents.get(0);
            String agentId = agentEnv.getUuid();
            extendInfo.put(OracleConstants.AGENT_ID, agentId);
        }
        log.info("finalize clear log, endpoint: {}, port:{}", agentEnv.getEndpoint(), agentEnv.getPort());
        FinalizeClearReq finalizeClearReq = getFinalizeClearReq(resource, extendInfo);
        agentBaseDto = agentUnifiedService.finalizeClear(agentEnv, resource.getSubType(), finalizeClearReq);
        return agentBaseDto;
    }

    private FinalizeClearReq getFinalizeClearReq(ProtectedResource resource, HashMap<String, String> extendInfo) {
        Application application = new Application();
        application.setUuid(resource.getUuid());
        application.setName(resource.getName());
        application.setParentUuid(resource.getParentUuid());
        application.setParentName(resource.getParentName());
        application.setType(resource.getType());
        application.setSubType(resource.getSubType());
        application.setExtendInfo(resource.getExtendInfo());
        application.setAuth(resource.getAuth());
        AppEnv appEnv = new AppEnv();
        return new FinalizeClearReq(appEnv, application, extendInfo);
    }

    /**
     * 检查刷新pdb的信息
     *
     * @param resource    实例信息
     * @param environment 环境信息
     */
    public void checkPdbInstanceActiveStandby(ProtectedResource resource, ProtectedEnvironment environment) {
        // 调用Agent接口，获取集群状态信息
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(resource, environment);
        if (VerifyUtil.isEmpty(appEnvResponse) || VerifyUtil.isEmpty(appEnvResponse.getExtendInfo())) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION,
                    new String[]{}, "get oracle cluster error.");
        }

        log.info("oracle refresh pdb instance active standby success. ip: {}", environment.getEndpoint());
        Map<String, String> envExtendInfo = appEnvResponse.getExtendInfo();
        // 校验cdb pdb
        if (OracleConstants.ERROR_ORACLE_DB_NOT_OPEN_CDB.equals(envExtendInfo.get(OracleConstants.PDB_STATUS))) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION,
                    new String[]{}, "The database is not open cdb.");
        }
        if (OracleConstants.ERROR_ORACLE_PDB_NOT_READ_WRITE.equals(envExtendInfo.get(OracleConstants.PDB_STATUS))) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION,
                    new String[]{}, "The status of pdb is not READ WRITE.");
        }
        if (!OracleConstants.SERVICE_RUNNING.equals(envExtendInfo.get(OracleConstants.STATUS))) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION,
                    new String[]{}, "oracle is not running.");
        }

        Map<String, String> resExtendInfo = resource.getExtendInfo();
        resExtendInfo.put(OracleConstants.VERIFY_STATUS, "true");
        resource.setVersion(envExtendInfo.get(DatabaseConstants.VERSION));
    }
}
