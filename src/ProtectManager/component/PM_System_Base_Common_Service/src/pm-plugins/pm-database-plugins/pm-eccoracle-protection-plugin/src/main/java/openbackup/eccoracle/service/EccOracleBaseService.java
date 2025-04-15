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
package openbackup.eccoracle.service;

import com.google.common.collect.ImmutableList;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryProtocolEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.eccoracle.constant.EccOracleConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.enums.DeployTypeEnum;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.dee.DeeDbAnonymizationRest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.stream.Collectors;

/**
 * ecc oracle通用service
 *
 */
@Component
@Slf4j
public class EccOracleBaseService {
    private static final ImmutableList<String> SUPPORT_ANONYMIZATION_DEPLOY_TYPE = ImmutableList.of(
        DeployTypeEnum.X8000.getValue(), DeployTypeEnum.X6000.getValue(), DeployTypeEnum.A8000.getValue(),
        DeployTypeEnum.X9000.getValue());

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final DeeDbAnonymizationRest deeDbAnonymizationRest;

    private final DeployTypeService deployTypeService;

    private final InstanceProtectionService instanceProtectionService;

    /**
     * 构造器
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param deeDbAnonymizationRest deeDbAnonymizationRest
     * @param deployTypeService deployTypeService
     * @param instanceProtectionService instanceProtectionService
     */
    public EccOracleBaseService(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        DeeDbAnonymizationRest deeDbAnonymizationRest, DeployTypeService deployTypeService,
        InstanceProtectionService instanceProtectionService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.deeDbAnonymizationRest = deeDbAnonymizationRest;
        this.deployTypeService = deployTypeService;
        this.instanceProtectionService = instanceProtectionService;
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
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected environment is not exists!"));
    }

    /**
     * 根据资源提取nodes
     *
     * @param resource 资源信息
     * @return 返回nodes
     */
    public List<TaskEnvironment> getEnvNodes(ProtectedResource resource) {
        return instanceProtectionService.extractEnvNodesBySingleInstance(resource);
    }

    /**
     * 根据资源提取agents
     *
     * @param resource 资源信息
     * @return 返回agents
     */
    public List<Endpoint> getAgents(ProtectedResource resource) {
        List<TaskEnvironment> nodeList = getEnvNodes(resource);
        log.info("SAP_ON_ORACLE_SINGLE backup single interceptor nodeList: {}", JsonUtil.json(nodeList));
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }

    /**
     * 从单实例的dependencies里获取主机
     *
     * @param uuid 资源id
     * @return 主机信息
     */
    public ProtectedEnvironment getAgentBySingleInstanceUuid(String uuid) {
        ProtectedResource singleInstanceResources = getResourceById(uuid);
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
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param agentEnv Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    public Endpoint getAgentEndpoint(ProtectedEnvironment agentEnv) {
        if (VerifyUtil.isEmpty(agentEnv.getUuid()) || VerifyUtil.isEmpty(agentEnv.getEndpoint()) || VerifyUtil.isEmpty(
            agentEnv.getPort())) {
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
        if (EccOracleConstants.WINDOWS.equalsIgnoreCase(agentEnv.getOsType())) {
            osType = agentEnv.getOsType().toLowerCase();
        }
        return osType;
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param resourceId 资源ID
     * @return 资源信息
     */
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "This protected resource is not exist, id: " + resourceId));
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
     * @param isCheckInstanceExist true表示注册时检查实例是否存在
     */
    public void refreshClusterInstanceActiveStandby(ProtectedResource resource, ProtectedEnvironment environment,
        boolean isCheckInstanceExist) {
        // 调用Agent接口，获取集群状态信息
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(resource, environment);
        if (VerifyUtil.isEmpty(appEnvResponse) || VerifyUtil.isEmpty(appEnvResponse.getExtendInfo())) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION, new String[] {},
                "get SAP_ON_ORACLE_SINGLE cluster error.");
        }
        log.info("oracle refresh cluster instance active standby success. ip: {}", environment.getEndpoint());
        Map<String, String> envExtendInfo = appEnvResponse.getExtendInfo();
        if (isCheckInstanceExist) {
            checkInstanceExist(resource, envExtendInfo);
        }

        Map<String, String> resExtendInfo = resource.getExtendInfo();
        // "sapsid": "EH7",
        // "accessOracleBase": "",
        // "runUserInfo": "[{\"ip\":\"192.168.120.123\",\"low_auth_user\":\"administrator\"}]",
        // "hostId": "764c63f6-eb60-4cd2-bda4-b5ec86b33b5d",
        // "brtools_version": "7.40",
        // "DBSID": "EH7",
        // "queryType": "asm",
        // "linkStatus": "0",
        // "SAPDATA_HOME": "C:\\oracle\\EH7",
        // "database_version": "11203",
        // "verify_status": "true",
        // "accessOracleHome": "",
        // "ORACLE_HOME": "C:\\oracle\\EH7\\11203",
        // "installUsername": "administrator",
        resExtendInfo.putAll(envExtendInfo);
        resExtendInfo.put(EccOracleConstants.VERIFY_STATUS, "true");
        resource.setVersion(envExtendInfo.get(DatabaseConstants.ORACLE_VERSION));
    }

    private void checkInstanceExist(ProtectedResource resource, Map<String, String> envExtendInfo) {
        String hostId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        String port = resource.getExtendInfo().get(DatabaseConstants.PORT);
        Map<String, Object> cons = new HashMap<>();
        String instName = envExtendInfo.get(EccOracleConstants.INST_NAME);
        cons.put(DatabaseConstants.HOST_ID, hostId);
        cons.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAP_ON_ORACLE_SINGLE.getType());
        cons.put(DatabaseConstants.PORT, port);
        cons.put(EccOracleConstants.INST_NAME, instName);
        List<ProtectedResource> records = resourceService.query(0, 1, cons).getRecords();
        log.info("EccOracleResourceProvider records{}#", records.size());
        if (records.size() != 0) {
            throw new DataProtectionAccessException(CommonErrorCode.PROTECTED_ENV_REPEATED, new String[] {},
                "SAP_ON_ORACLE_SINGLE instance is already exist.");
        }
    }

    /**
     * Windows系统，下发到ubc的仓库协议需要设置为CIFS。data/log repo的extendInfo加一个参数，everyoneAuth
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
        if (EccOracleConstants.WINDOWS.equalsIgnoreCase(environment.getOsType())) {
            repositories.stream().filter(repo -> repoTypeList.contains(repo.getType())).forEach(repo -> {
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
}