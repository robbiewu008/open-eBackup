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
package openbackup.oracle.interceptor;

import openbackup.access.framework.resource.service.ProtectedEnvironmentListener;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.MessageTemplate;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * oracle副本恢复
 *
 */
@Slf4j
@Component
public class OracleRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private static final String ENC_KEY = "EncKey";

    // 副本密码
    private static final String BACKUP_ALGO_VALUE = "backup_algo_value";
    private static final String ADDITIONAL_LOG_COPY_LIST = "additional_log_copy_list";
    private static final String SOURCE_COPY_TYPE = "source_copy_type";

    // Oracle下发非日志副本恢复时，跳过agent对日志副本信息的组装
    private static final String SKIP_LOG_REPO_COMPOSE = "skipLogRepoCompose";

    // 此次恢复是通过数据副本 data、日志副本 log、任意时间点 pit 还是scn 恢复
    private static final String RESTORE_FROM = "restoreFrom";
    private static final String LOG = "log";

    private final OracleBaseService oracleBaseService;
    private final CopyRestApi copyRestApi;
    private final OracleSingleRestoreProvider singleProvider;
    private final OracleClusterRestoreProvider clusterProvider;
    private final ResourceService resourceService;
    private MessageTemplate<String> messageTemplate;
    private EncryptorService encryptorService;
    private AgentUnifiedService agentUnifiedService;

    /**
     * 构造器
     *
     * @param copyRestApi       副本restApi
     * @param oracleBaseService oracle应用基本的Service
     * @param singleProvider    单机恢复provider
     * @param clusterProvider   集群恢复provider
     * @param resourceService   resourceService
     */
    public OracleRestoreProvider(CopyRestApi copyRestApi, OracleBaseService oracleBaseService,
            OracleSingleRestoreProvider singleProvider, OracleClusterRestoreProvider clusterProvider,
            ResourceService resourceService) {
        this.copyRestApi = copyRestApi;
        this.oracleBaseService = oracleBaseService;
        this.singleProvider = singleProvider;
        this.clusterProvider = clusterProvider;
        this.resourceService = resourceService;
    }

    @Autowired
    public void setEncryptorService(EncryptorService encryptorService) {
        this.encryptorService = encryptorService;
    }

    @Autowired
    public void setMessageTemplate(MessageTemplate<String> messageTemplate) {
        this.messageTemplate = messageTemplate;
    }

    @Autowired
    public void setAgentUnifiedService(AgentUnifiedService agentUnifiedService) {
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    protected void checkConnention(RestoreTask task) {
        log.debug("oracle restore no need check connection. task id: {}", task.getTaskId());
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        log.info("oracle restore check start task id: {}.", task.getTaskId());
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        checkRestore(task, copy);
        fillRestoreMode(task, copy);
        fillAdvanceParams(task, copy);
        fillLogCopy(task, copy);
        supplyRestoreTaskBySubtype(task, copy);
        Map<String, Object> property = JSON.parseObject(copy.getProperties(), Map.class);
        boolean isSnapshot = false;
        Object transportMode = property.get(OracleConstants.COPY_TRANSPORT_MODE);
        if (ObjectUtils.isNotEmpty(transportMode) && transportMode instanceof String) {
            isSnapshot = StringUtils.equals(OracleConstants.STORAGE_LAYER, (String) transportMode);
        }
        if (isSnapshot) {
            task.getAdvanceParams().put(OracleConstants.STORAGE_SNAPSHOT_FLAG, "true");
            supplySnapshotAgents(task);
        }
        oracleBaseService.supplyNodes(task);
        if (isSnapshot) {
            supplySnapshotNodes(task);
        }
        fillRepositoriesForWindows(task);
        log.info("oracle restore check finish task id: {}.", task.getTaskId());
        return task;
    }

    /**
     * supplySnapshotNodes
     *
     * @param restoreTask restoreTask
     */
    public void supplySnapshotNodes(RestoreTask restoreTask) {
        log.info("start supplySnapshotNodes");
        String snapshotAgents = restoreTask.getAdvanceParams().get(OracleConstants.PROXY_HOST);
        log.info("supplySnapshotNodes snapshotAgents:{}", snapshotAgents);
        if (Strings.isBlank(snapshotAgents)) {
            return;
        }
        List<TaskEnvironment> nodes = restoreTask.getTargetEnv().getNodes();
        if (nodes == null) {
            nodes = new ArrayList<>();
        }
        log.info("supplySnapshotNodes nodes beforeSize:{}", nodes.size());
        JSONArray snapshotAgentArray = JSONArray.parseArray(snapshotAgents);
        for (int i = 0; i < snapshotAgentArray.size(); i++) {
            com.alibaba.fastjson.JSONObject snapshotNode = snapshotAgentArray.getJSONObject(i);
            String uuid = snapshotNode.getString("uuid");
            Optional<TaskEnvironment> node = nodes.stream().filter(p -> p.getUuid().equals(uuid)).findFirst();
            if (node.isPresent()) {
                // 如果生产agent同时被选为存储快照执行agent，打存储快照标志
                node.get().getExtendInfo().put(OracleConstants.STORAGE_SNAPSHOT_AGENT_FLAG, "true");
                continue;
            }
            ProtectedResource tmp = oracleBaseService.getResource(uuid);
            if (tmp == null) {
                log.warn("agent doesn't exist,id:{}", uuid);
                continue;
            }
            // 只添加在线的linux系统agent
            ProtectedEnvironment envAgent = oracleBaseService.getEnvironmentById(uuid);
            if (!LinkStatusEnum.ONLINE.getStatus().toString().equals(envAgent.getLinkStatus())
                || OracleConstants.WINDOWS.equalsIgnoreCase(envAgent.getOsType())) {
                log.warn("selected agent is invalid,id:{}", uuid);
                continue;
            }
            // agent离线，则过滤掉
            try {
                agentUnifiedService.getHost(envAgent.getEndpoint(), envAgent.getPort());
            } catch (LegoCheckedException e) {
                log.warn("job id: {}, agent: {}, is offline", restoreTask.getTaskId(), envAgent.getEndpoint());
                continue;
            }
            TaskEnvironment environment = JsonUtil.read(JsonUtil.json(tmp), TaskEnvironment.class);
            Map<String, String> extendInfo = new HashMap<>();
            extendInfo.put(OracleConstants.STORAGE_SNAPSHOT_AGENT_FLAG, "true");
            if (tmp.getExtendInfo() != null) {
                extendInfo.put(ResourceConstants.AGENT_IP_LIST,
                    tmp.getExtendInfo().getOrDefault(ResourceConstants.AGENT_IP_LIST, ""));
            }
            environment.setExtendInfo(extendInfo);
            nodes.add(environment);
        }
        log.info("supplySnapshotNodes nodes endSize:{}", nodes.size());
        restoreTask.getTargetEnv().setNodes(nodes);
    }

    private void fillRepositoriesForWindows(RestoreTask task) {
        String envUuid = task.getTargetEnv().getUuid();
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(envUuid);
        oracleBaseService.repositoryAdaptsWindows(task.getRepositories(), environment);
    }

    private void fillAdvanceParams(RestoreTask task, Copy copy) {
        // 从副本里查询SLA里的并发数，设置到恢复的高级参数里
        Map<String, String> advanceParams = task.getAdvanceParams();
        advanceParams.put(DatabaseConstants.SLA_KEY, copy.getSlaProperties());
        Map<String, Object> property = JSON.parseObject(copy.getProperties(), Map.class);
        if (!LOG.equals(advanceParams.get(RESTORE_FROM))) {
            advanceParams.put(SKIP_LOG_REPO_COMPOSE, "true");
        }
        if (!property.containsKey(BACKUP_ALGO_VALUE) || VerifyUtil.isEmpty(property.get(BACKUP_ALGO_VALUE))) {
            return;
        }
        advanceParams.put(ENC_KEY, encryptorService.decrypt(property.get(BACKUP_ALGO_VALUE).toString()));
    }

    private void fillRestoreMode(RestoreTask task, Copy copy) {
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(copy.getGeneratedBy())
                || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(copy.getGeneratedBy())
        || CopyGeneratedByEnum.BY_IMPORTED.value().equals(copy.getGeneratedBy())) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("Oracle set restore mode success, taskId: {}, copy id: {}, generatedBy: {}",
                task.getTaskId(), task.getCopyId(), copy.getGeneratedBy());
    }

    private void checkRestore(RestoreTask task, Copy copy) {
        TaskResource targetResource = task.getTargetObject();
        checkSubType(copy, targetResource);
        checkVersion(targetResource, copy.getResourceProperties());
    }

    private void checkVersion(TaskResource targetResource, String resourceProperties) {
        JSONObject resourceJson = JSONObject.fromObject(resourceProperties);
        String copyVersion = Optional.ofNullable(resourceJson.get(DatabaseConstants.VERSION).toString())
                .orElse(StringUtils.EMPTY);
        ProtectedResource resource = oracleBaseService.getResource(targetResource.getUuid());
        String targetVersion = resource.getVersion();
        if (!copyVersion.equals(targetVersion)) {
            log.error("Oracle restore check version failed, source version: {}, target version: {}",
                    copyVersion, targetVersion);
            throw new LegoCheckedException(DatabaseErrorCode.RESTORE_RESOURCE_VERSION_INCONSISTENT,
                    "Inconsistent Version!");
        }
    }

    // Oracle不允许从单机恢复到集群
    private void checkSubType(Copy copy, TaskResource target) {
        if (ResourceSubTypeEnum.ORACLE.equalsSubType(copy.getResourceSubType())
                && !ResourceSubTypeEnum.ORACLE.equalsSubType(target.getSubType())) {
            log.error("Oracle restore copy type: {}, target type: {}", copy.getResourceSubType(), target.getSubType());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Can not restore from single to cluster!");
        }
    }

    private void fillLogCopy(RestoreTask task, Copy copy) {
        if (copy.getSourceCopyType() == BackupTypeConstants.LOG.getAbBackupType()) {
            return;
        }
        Optional<String> logCopy = getLastLogCopy(copy.getResourceId());
        task.getAdvanceParams().put(ADDITIONAL_LOG_COPY_LIST, logCopy.map(s ->
                JSON.toJSONString(Collections.singletonList(s)))
                .orElseGet(() -> JSON.toJSONString(Collections.emptyList())));
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        return Collections.singletonList(new LockResourceBo(copy.getResourceName() + task.getTargetEnv().getUuid(),
                LockType.WRITE));
    }

    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("name", copy.getResourceName());
        conditions.put("parentUuid", task.getTargetEnv().getUuid());
        conditions.put("subType", task.getTargetObject().getSubType());
        PageListResponse<ProtectedResource> response = resourceService.query(0, 1, conditions);
        List<ProtectedResource> resources = response.getRecords();
        if (VerifyUtil.isEmpty(resources)) {
            return Collections.emptyList();
        }
        log.info("set oracle next backup type to full. task id: {}, resources: {} next backup is full.",
                task.getTaskId(), resources.get(0).getUuid());
        return Collections.singletonList(resources.get(0).getUuid());
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.ORACLE_CLUSTER.getType().equals(subType)
                || ResourceSubTypeEnum.ORACLE.getType().equals(subType);
    }

    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature restoreFeature = super.getRestoreFeature();
        restoreFeature.setShouldCheckEnvironmentIsOnline(false);
        return restoreFeature;
    }

    @Override
    public void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
        if (!jobStatus.checkSuccess()) {
            return;
        }
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("parentUuid", task.getTargetEnv().getUuid());
        conditions.put("subType", task.getTargetObject().getSubType());
        List<ProtectedResource> resources;
        int page = 0;
        do {
            PageListResponse<ProtectedResource> resourceResponse =
                resourceService.query(page, IsmNumberConstant.HUNDRED, conditions);
            resources = resourceResponse.getRecords();
            Optional<ProtectedResource> matchResource = resources.stream()
                .filter(resource -> copy.getResourceName().equalsIgnoreCase(resource.getName()))
                .findAny();
            if (matchResource.isPresent()) {
                sendScanMessage(matchResource.get().getUuid());
                log.info("Oracle restore postProcess finished. taskId:{}, databaseUuid:{}", task.getTaskId(),
                    matchResource.get().getUuid());
                return;
            }
            page++;
        } while (resources.size() >= IsmNumberConstant.HUNDRED);
        log.info("No oracle data base named {} in environment(evnId: {}),scan message will not be sent",
            copy.getResourceName(), task.getTargetEnv().getUuid());
    }

    private void sendScanMessage(String databaseUuid) {
        JSONObject messageData = new JSONObject();
        messageData.set("uuid", databaseUuid);
        messageTemplate.send(ProtectedEnvironmentListener.SCANNING_ENVIRONMENT_V2, messageData);
    }

    private void supplyRestoreTaskBySubtype(RestoreTask task, Copy copy) {
        if (ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.equalsSubType(task.getTargetEnv().getSubType())) {
            clusterProvider.supplyCluster(task, copy);
        } else {
            singleProvider.supplySingle(task);
        }
    }

    private Optional<String> getLastLogCopy(String resourceId) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put("resource_id", resourceId);
        conditions.put(SOURCE_COPY_TYPE, BackupTypeConstants.LOG.getAbBackupType());
        List<String> orders = new ArrayList<>();
        orders.add("-display_timestamp");
        BasePage<Copy> copies = copyRestApi.queryCopies(0, 1, conditions, orders);
        if (VerifyUtil.isEmpty(copies.getItems())) {
            return Optional.empty();
        }
        return Optional.ofNullable(copies.getItems().get(0).getUuid());
    }

    /**
     * supplySnapshotAgents
     *
     * @param task RestoreTask
     */
    public void supplySnapshotAgents(RestoreTask task) {
        log.info("start supplySnapshotAgents");
        // 存储快照副本恢复，把用户选择的代理主机添加进去
        List<Endpoint> endpointList = new ArrayList<>(task.getAgents());
        String productAgentOsType = null;
        if (endpointList.stream().findFirst().isPresent()) {
            productAgentOsType = endpointList.stream().findFirst().get().getAgentOS();
        }
        String snapshotAgents = task.getAdvanceParams().get(OracleConstants.PROXY_HOST);
        log.info("supplySnapshotAgents snapshotAgents:{}", snapshotAgents);
        if (Strings.isBlank(snapshotAgents)) {
            // 如果自身的生产agent是windows操作环境且没选择执行存储快照agent就报错。
            if (OracleConstants.WINDOWS.equalsIgnoreCase(productAgentOsType)) {
                throw new LegoCheckedException(DatabaseErrorCode.LINUX_AGENT_NOT_EXIST,
                    "No Linux OS proxy host is selected.");
            }
            return;
        }
        int beforeSize = endpointList.size();
        log.info("supplySnapshotAgents endpointList beforeSize:{}", beforeSize);
        JSONArray snapshotAgentArray = JSONArray.parseArray(snapshotAgents);
        for (int i = 0; i < snapshotAgentArray.size(); i++) {
            com.alibaba.fastjson.JSONObject node = snapshotAgentArray.getJSONObject(i);
            String uuid = node.getString("uuid");
            if (endpointList.stream().anyMatch(p -> p.getId().equals(uuid))) {
                continue;
            }
            // 只添加在线的linux系统agent
            ProtectedEnvironment envAgent = oracleBaseService.getEnvironmentById(uuid);
            if (!LinkStatusEnum.ONLINE.getStatus().toString().equals(envAgent.getLinkStatus())
                || OracleConstants.WINDOWS.equalsIgnoreCase(envAgent.getOsType())) {
                log.warn("selected agent is invalid,id:{}", uuid);
                continue;
            }
            // agent离线，则过滤掉
            try {
                agentUnifiedService.getHost(envAgent.getEndpoint(), envAgent.getPort());
            } catch (LegoCheckedException e) {
                log.warn("job id: {}, agent: {}, is offline", task.getTaskId(), envAgent.getEndpoint());
                continue;
            }
            Endpoint agentEndpoint = oracleBaseService.getAgentEndpoint(envAgent);
            endpointList.add(agentEndpoint);
        }
        // 如果自身的生产agent是windows操作系统，且没有选择在线的linux主机则报错，
        if (OracleConstants.WINDOWS.equalsIgnoreCase(productAgentOsType) && beforeSize == endpointList.size()) {
            log.error("No Linux nodes is selected.");
            throw new LegoCheckedException(DatabaseErrorCode.LINUX_AGENT_NOT_EXIST,
                "No Linux OS proxy host is selected.");
        }
        task.setAgents(endpointList);
        log.info("supplySnapshotAgents endpointList endSize:{}", endpointList.size());
    }
}
