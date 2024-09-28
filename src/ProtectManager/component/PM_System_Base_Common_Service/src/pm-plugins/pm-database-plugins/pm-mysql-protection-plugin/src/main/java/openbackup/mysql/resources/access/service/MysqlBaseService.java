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
package openbackup.mysql.resources.access.service;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.asserts.PowerAssert;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import io.jsonwebtoken.lang.Collections;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.regex.Pattern;
import java.util.stream.Collectors;

/**
 * mysql应用基本的Service
 *
 */
@Slf4j
@Service("mysqlBaseService")
public class MysqlBaseService {
    private static final List<String> SYS_DATABASES = Arrays.asList("information_schema", "mysql", "performance_schema",
        "sys");

    /**
     * 数据库重命名时，不能包含这些特殊字符
     */
    private final Pattern charsetReg = Pattern.compile("[|;&@$><`!+\n#\\[\\]]");

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final EncryptorService encryptorService;

    private final InstanceResourceService instanceResourceService;

    /**
     * mysql应用基本的Service有参构造方法
     *
     * @param resourceService 资源服务接口
     * @param agentUnifiedService Agent Unified Service
     * @param encryptorService encryptorService
     * @param instanceResourceService 实例资源服务
     */
    public MysqlBaseService(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        EncryptorService encryptorService,
        InstanceResourceService instanceResourceService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.encryptorService = encryptorService;
        this.instanceResourceService = instanceResourceService;
    }

    /**
     * 针对单实例uuid，从单实例的dependency里，获取对应的Agent主机
     *
     * @param singleInstanceUuid 单实例uuid
     * @return Agent主机信息
     */
    public ProtectedEnvironment getAgentBySingleInstanceUuid(String singleInstanceUuid) {
        ProtectedResource singleInstanceResources = getResource(singleInstanceUuid);
        List<ProtectedResource> agentResources = singleInstanceResources.getDependencies()
            .get(DatabaseConstants.AGENTS);
        if (agentResources == null || agentResources.size() != 1) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "single instance dependency agent is not one.");
        }
        ProtectedResource agentResource = agentResources.get(0);
        PowerAssert.state(agentResource instanceof ProtectedEnvironment,
            () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "mysql agent resource is not env."));
        return (ProtectedEnvironment) agentResource;
    }

    /**
     * 针对集群实例uuid，从dependency里，获取集群实例下面的所有子实例
     *
     * @param clusterInstanceUuid 集群实例uuid
     * @return List ProtectedResource 集群实例依赖的子实例信息
     */
    public List<ProtectedResource> getSingleInstanceByClusterInstance(String clusterInstanceUuid) {
        ProtectedResource clusterInstanceResource = getResource(clusterInstanceUuid);
        Map<String, List<ProtectedResource>> dependencies = clusterInstanceResource.getDependencies();
        return dependencies.get(DatabaseConstants.CHILDREN);
    }

    /**
     * 根据Agent主机信息，获取Agent主机的Endpoint
     *
     * @param env Agent环境信息
     * @return Agent对应的Endpoint信息
     */
    public Endpoint getAgentEndpoint(ProtectedEnvironment env) {
        if (VerifyUtil.isEmpty(env.getUuid()) || VerifyUtil.isEmpty(env.getEndpoint()) || VerifyUtil.isEmpty(
            env.getPort())) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "mysql agent env lack require msg.");
        }
        return new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort());
    }

    /**
     * 针对Mysql集群实例里的所有单实例信息，将单实例信息的auth设置到单实例对应的nodes的auth中
     * 将dataDir信息、角色信息、MySQL服务的启动信息放置在extendInfo中
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
            if (Collections.isEmpty(taskEnvironments)) {
                continue;
            }
            TaskEnvironment nodeEnv = taskEnvironments.get(0);
            Authentication authentication = new Authentication();
            nodeEnv.setAuth(BeanTools.copy(singleInstanceResource.getAuth(), authentication));
            if (nodeEnv.getExtendInfo() == null) {
                nodeEnv.setExtendInfo(new HashMap<>());
            }
            setNodeEnvExtendInfo(nodeEnv, singleInstanceResource);
            log.info("update node mysql instance msg success. node ip: {}, node role: {}", nodeEnv.getEndpoint(),
                nodeEnv.getExtendInfo().get(DatabaseConstants.ROLE));
        }
    }

    private void setNodeEnvExtendInfo(TaskEnvironment nodeEnv, ProtectedResource singleInstanceResource) {
        Map<String, String> nodeEnvExtendInfo = nodeEnv.getExtendInfo();
        nodeEnvExtendInfo.put(DatabaseConstants.DATA_DIR,
            encryptorService.decrypt(singleInstanceResource.getExtendInfo().get(DatabaseConstants.DATA_DIR)));
        nodeEnvExtendInfo.put(MysqlConstants.LOG_BIN_INDEX_PATH,
            encryptorService.decrypt(singleInstanceResource.getExtendInfo().get(MysqlConstants.LOG_BIN_INDEX_PATH)));
        nodeEnvExtendInfo.put(MysqlConstants.INSTANCE_IP,
            singleInstanceResource.getExtendInfo().get(MysqlConstants.INSTANCE_IP));
        nodeEnvExtendInfo.put(DatabaseConstants.ROLE,
            singleInstanceResource.getExtendInfo().get(DatabaseConstants.ROLE));
        nodeEnvExtendInfo.put(MysqlConstants.MYSQL_SERVICE_NAME,
            singleInstanceResource.getExtendInfo().get(MysqlConstants.MYSQL_SERVICE_NAME));
        nodeEnvExtendInfo.put(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE,
            singleInstanceResource.getExtendInfo().get(MysqlConstants.MYSQL_SYSTEM_SERVICE_TYPE));
        nodeEnvExtendInfo.put(MysqlConstants.MASTER_INFO,
            singleInstanceResource.getExtendInfo().get(MysqlConstants.MASTER_INFO));
        nodeEnvExtendInfo.put(DatabaseConstants.CHARSET,
            singleInstanceResource.getExtendInfo().getOrDefault(DatabaseConstants.CHARSET, ""));
        nodeEnvExtendInfo.put(MysqlConstants.MY_CNF_PATH,
            singleInstanceResource.getExtendInfo().getOrDefault(MysqlConstants.MY_CNF_PATH, ""));
    }

    /**
     * 针对不同的集群类型，需要检验集群节点的role信息是否正确。
     * 比如PXC，那么就必须有一个节点role为1，其它所有节点为2；
     * 比如AP，目前没有细化环形主备，互为主备，目前检验和PXC一致
     *
     * @param nodes 节点列表
     */
    public void checkClusterRole(List<TaskEnvironment> nodes) {
        if (nodes == null || nodes.isEmpty()) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_FAILED, "nodes is null.");
        }
        if (nodes.size() == 1) {
            return;
        }
        int masterNodeNum = 0;
        int slaveNodeNum = 0;
        for (TaskEnvironment node : nodes) {
            String role = node.getExtendInfo().get(DatabaseConstants.ROLE);
            if (NodeType.MASTER.getNodeType().equals(role)) {
                masterNodeNum++;
            } else if (NodeType.SLAVE.getNodeType().equals(role)) {
                slaveNodeNum++;
            } else {
                throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_FAILED,
                    "node role is not master or slave. is " + role);
            }
        }
        if (masterNodeNum != 1) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_FAILED,
                "master node is not 1. is " + masterNodeNum);
        }
        log.info("cluster master node size: {}. slave node size: {}", masterNodeNum, slaveNodeNum);
    }

    /**
     * 根据资源uuid，获取应该存在的资源信息
     *
     * @param resourceUuid 资源uuid
     * @return ProtectedResource 资源信息
     */
    public ProtectedResource getResource(String resourceUuid) {
        return resourceService.getResourceById(resourceUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST));
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
                () -> new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "Protected environment is not exists!"));
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
     * 为实例设置信息到扩展字段中
     *
     * @param version mysql实例的版本号
     * @param extendInfo 扩展对象
     * @return 设置值后的扩展对象
     */
    public Map<String, String> supplyExtendInfo(String version, Map<String, String> extendInfo) {
        if (VerifyUtil.isEmpty(version)) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR, "instance version is null.");
        }
        if (extendInfo == null) {
            Map<String, String> newExtendInfo = new HashMap<>();
            newExtendInfo.put(DatabaseConstants.VERSION, version);
            return newExtendInfo;
        }
        extendInfo.put(DatabaseConstants.VERSION, version);
        return extendInfo;
    }

    /**
     * 查询数据库版本
     *
     * @param resource 实例资源
     * @param environment 环境
     * @return version 版本
     */
    public String queryDatabaseVersion(ProtectedResource resource, ProtectedEnvironment environment) {
        log.info("Get MySQL-instance {} Version!", resource.getUuid());
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(resource, Application::new));
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(
            ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), environment, checkAppReq, true);
        Map<String, String> extendInfo = Optional.ofNullable(appEnvResponse.getExtendInfo())
            .orElse(java.util.Collections.emptyMap());
        String version = extendInfo.get(DatabaseConstants.VERSION);
        if (VerifyUtil.isEmpty(version)) {
            log.error("Get MySQL-instance {} Version failed!", resource.getUuid());
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_AUTH_FAILED, "get MySQL Version failed!");
        }
        return version;
    }

    /**
     * 查询数据库部署的操作系统
     *
     * @param resource 实例资源
     * @param environment 环境
     * @return deployOperatingSystem 部署的操作系统名称
     */
    public String queryDeployOperatingSystem(ProtectedResource resource, ProtectedEnvironment environment) {
        log.info("Get MySQL-instance {} deployOperatingSystem!", resource.getUuid());
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(resource, Application::new));
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(
            ResourceSubTypeEnum.MYSQL_SINGLE_INSTANCE.getType(), environment, checkAppReq, true);
        Map<String, String> extendInfo = Optional.ofNullable(appEnvResponse.getExtendInfo())
            .orElse(java.util.Collections.emptyMap());
        String deployOperatingSystem = extendInfo.get(MysqlConstants.DEPLOY_OPERATING_SYSTEM);
        if (VerifyUtil.isEmpty(deployOperatingSystem)) {
            log.error("Get MySQL-instance {} deployOperatingSystem failed!", resource.getUuid());
            throw new LegoCheckedException(MysqlErrorCode.CHECK_MYSQL_DEPLOYMENT_MODEL_FAILED,
                "get MySQL deployOperatingSystem failed!");
        }
        return deployOperatingSystem;
    }

    /**
     * 检查mysql数据库名字是否合法
     *
     * @param name 数据库名字
     * @return 检测结果
     */
    public Boolean checkMySQLDatabaseNameInvalidCharacters(String name) {
        return !charsetReg.matcher(name).find();
    }

    /**
     * 检查extendInfo中的部署操作系统
     *
     * @param extendInfo 源
     * @param targetResourceExtendInfo 目标端
     */
    public void checkDeployOperatingSystem(Map<String, String> extendInfo,
        Map<String, String> targetResourceExtendInfo) {
        // 操作系统拦截
        if (!targetResourceExtendInfo.get(MysqlConstants.DEPLOY_OPERATING_SYSTEM)
            .equals(extendInfo.get(MysqlConstants.DEPLOY_OPERATING_SYSTEM))) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_OPERATING_SYSTEM_DEPLOYMENT_MODEL_FAILED,
                "Inconsistent deployOperatingSystem!");
        }
    }

    /**
     * 检验subtype是否一致
     *
     * @param copy 源
     * @param targetResource 目标资源
     */
    public void checkSubType(Copy copy, TaskResource targetResource) {
        if (!copy.getResourceSubType().equals(targetResource.getSubType())) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_MYSQL_DEPLOYMENT_MODEL_FAILED, "Inconsistent subType!");
        }
    }

    /**
     * 检验集群的部署模式，pxc，ap
     *
     * @param extendInfo 源端扩展信息
     * @param targetResourceExtendInfo 目标端扩展信息
     */
    public void checkClusterType(Map<String, String> extendInfo, Map<String, String> targetResourceExtendInfo) {
        String copyClusterType = Optional.ofNullable(extendInfo.get(DatabaseConstants.CLUSTER_TYPE))
            .orElse(StringUtils.EMPTY);
        String targetClusterType = Optional.ofNullable(targetResourceExtendInfo.get(DatabaseConstants.CLUSTER_TYPE))
            .orElse(StringUtils.EMPTY);
        if (!copyClusterType.equals(targetClusterType)) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_MYSQL_DEPLOYMENT_MODEL_FAILED, "Inconsistent Cluster!");
        }
    }

    /**
     * 检验version是否一致
     *
     * @param targetResource 目标资源
     * @param resourceJson 源端资源
     */
    public void checkVersion(TaskResource targetResource, JSONObject resourceJson) {
        String copyVersion = Optional.ofNullable(resourceJson.get(DatabaseConstants.VERSION).toString())
            .orElse(StringUtils.EMPTY);
        ProtectedResource resource = getResource(targetResource.getUuid());
        String targetVersion = resource.getVersion();
        if (!copyVersion.equals(targetVersion)) {
            log.warn("Copy version, resource version: {}, {}", copyVersion, targetVersion);
            if (copyVersion.contains("MariaDB") && (copyVersion.endsWith("-" + targetVersion) || targetVersion.endsWith(
                "-" + copyVersion))) {
                return;
            }
            throw new LegoCheckedException(MysqlErrorCode.CHECK_VERSION_FAILED, "Inconsistent Version!");
        }
    }

    /**
     * 检验数据库名称是否合法
     *
     * @param task 目标
     */
    public void checkNewDatabaseName(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams())
            .orElse(java.util.Collections.emptyMap());
        String newDatabaseName = advanceParams.get(MysqlConstants.NEW_DATABASE_NAME);
        // 只有执行数据库恢复时支持重命名
        if (VerifyUtil.isEmpty(newDatabaseName)) {
            return;
        }
        // 检测字符长度
        if (newDatabaseName.length() > MysqlConstants.DATABASE_NAME_LIMIT_LENGTH) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_DATABASE_NAME_FAILED, "Inconsistent databasesName!");
        }
        // 检验特殊字符 |;&$><`!+\n//#\[\]
        if (!checkMySQLDatabaseNameInvalidCharacters(newDatabaseName)) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_DATABASE_NAME_FAILED, "Inconsistent databasesName!");
        }
        // 拦截系统自带的数据库
        if (SYS_DATABASES.contains(newDatabaseName)) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_DATABASE_NAME_FAILED, "Inconsistent databasesName!");
        }
    }

    /**
     * 集群实例状态检查
     *
     * @param environment 环境
     */
    public void healthCheckAllNodes(ProtectedEnvironment environment) {
        log.info("Enter cluster instance of environment node health check id: {}, subType: {}", environment.getUuid(),
            environment.getSubType());
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        PageListResponse<ProtectedResource> data;
        List<ProtectedResource> resources = new ArrayList<>();
        int pageNo = 0;
        do {
            data = resourceService.query(pageNo, IsmNumberConstant.HUNDRED, conditions);
            resources.addAll(data.getRecords());
            pageNo++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
        resources.forEach(this::healthCheckClusterInstance);
    }

    /**
     * 通过agent进行集群实例状态检查
     *
     * @param resource 资源
     */
    private void healthCheckClusterInstance(ProtectedResource resource) {
        ProtectedResource fullResource = resourceService.getResourceById(resource.getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource not exist."));
        List<ProtectedResource> childrenResources = fullResource.getDependencies().get(DatabaseConstants.CHILDREN);
        for (ProtectedResource protectedResource : childrenResources) {
            protectedResource.setEnvironment(getEnvironmentOfInstance(protectedResource));
            instanceResourceService.healthCheckSingleInstance(protectedResource);
        }
    }

    /**
     * 获取实例环境信息
     *
     * @param protectedResource 保护资源
     * @return ProtectedEnvironment 保护环境
     */
    private ProtectedEnvironment getEnvironmentOfInstance(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This environment not exist."));
    }
}
