package openbackup.oracle.interceptor;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.HostDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.restore.service.RestoreTaskHelper;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.DatabaseErrorCode;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import com.huawei.oceanprotect.job.sdk.JobService;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.provider.OracleAgentProvider;
import openbackup.oracle.service.OracleBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.model.PagingParamRequest;
import openbackup.system.base.common.model.job.JobBo;
import openbackup.system.base.common.model.job.request.QueryJobRequest;
import openbackup.system.base.common.utils.RandomPwdUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.BasePage;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.job.model.JobStatusEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.logging.log4j.util.Strings;
import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.OptionalInt;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * oracle备份
 *
 * @version [OceanProtect DataBackup 1.3.0]
 * @author c30038333
 * @since 2023-01-05
 */
@Slf4j
@Component
@AllArgsConstructor
public class OracleBackupInterceptor extends AbstractDbBackupInterceptor {
    private static final String ENC_KEY = "EncKey";

    // 密文密码
    private static final String ENCRYPTED = "Encrypted";

    private static final String ENCRYPTION_ALGORITHM = "encryption_algorithm";

    private static final String ENCRYPTION = "encryption";

    private static final String BALANCE_AGENTS = "balance_agents";

    private static final String SNAPSHOT_NODES = "snapshot_nodes";

    private static final String SNAPSHOT_AGENTS = "snapshot_agents";

    private static final StringBuffer SYMBOLS = new StringBuffer("#,_,@,-");

    private OracleBaseService oracleBaseService;

    private AgentUnifiedService agentUnifiedService;

    private CopyRestApi copyRestApi;

    private EncryptorService encryptorService;

    private ResourceService resourceService;

    private OracleAgentProvider oracleAgentProvider;

    private JobService jobService;

    /**
     * Oracle数据库拦截器操作实现
     * <p>
     * 1. 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     * 2. 设置备份任务拆分时所需要的部署类型
     *
     * @param backupTask 初始的备份对象
     * @return 经过应用拦截器后的备份对象
     */
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        log.info("Oracle backup task start supply backup task, task id: {}", backupTask.getTaskId());
        checkExistRunningInstanceRestoreJob(backupTask.getProtectObject().getUuid(),
            backupTask.getProtectObject().getName());
        updateRepositories(backupTask);

        checkIsLogBackup(backupTask);

        checkStorageSnapshotBackupType(backupTask);

        // 填充auth信息和副本密码
        fillBackupParams(backupTask);

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 设置副本格式
        ProtectionTaskUtils.setCopyFormat(backupTask);
        setFileHandleByteAlignmentSwitch(backupTask);
        setNeedDeleteDtree(backupTask);
        log.info("Oracle backup task finished supply backup task, task id: {}", backupTask.getTaskId());
        return backupTask;
    }

    /**
     * 存储快照备份模式下校验备份类型，如果是差异备份则报异常。
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void checkStorageSnapshotBackupType(BackupTask backupTask) {
        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        String flag = advanceParams.get(OracleConstants.STORAGE_SNAPSHOT_FLAG);
        if (!VerifyUtil.isEmpty(flag) && "true".equalsIgnoreCase(flag)
            && DatabaseConstants.DIFF_BACKUP_DIFF.equals(backupTask.getBackupType())) {
            throw new LegoCheckedException(DatabaseErrorCode.ERROR_EXEC_DIFF_BACKUP,
                "storage snapshot backup does not support differential backup");
        }
    }

    /**
     * PM侧校验Oracle的日志备份，并将校验结果设置到任务的额外参数中
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void checkIsLogBackup(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            backupTask.getAdvanceParams().put(OracleConstants.IS_CHECK_BACKUP_JOB_TYPE, "true");
        } else {
            backupTask.getAdvanceParams().put(OracleConstants.IS_CHECK_BACKUP_JOB_TYPE, "false");
        }
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return Objects.nonNull(subType) && isBelongToOracle(subType);
    }

    /**
     * 根据备份任务类型的不同，更新仓库
     * 如果是日志备份，则新增日志仓库，移除数据仓库；不管是哪种备份，都需要新增cache仓
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            backupTask.addRepository(logRepository);
            repositories.remove(0);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        // oracle适配Windows系统，下发到ubc的仓库协议需要设置为CIFS
        String envUuid = backupTask.getProtectEnv().getUuid();
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(envUuid);
        oracleBaseService.repositoryAdaptsWindows(repositories, environment);
        backupTask.setRepositories(repositories);
    }

    /**
     * 如果是数据库粒度的资源备份，需要根据数据库的parentUuid找到对应的单实例资源，
     * 然后再根据单实例资源的parentUuid，找到对应的主机
     * 然后根据Agent主机，拿到Agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        ProtectedResource protectedResource = new ProtectedResource();
        BeanUtils.copyProperties(backupTask.getProtectObject(), protectedResource);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(protectedResource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .parameters(backupTask.getAdvanceParams())
            .build();
        List<Endpoint> endpointList = oracleAgentProvider.getSelectedAgents(agentSelectParam);
        backupTask.setAgents(endpointList);
        String productAgentOsType = null;
        if (endpointList.stream().findFirst().isPresent()) {
            productAgentOsType = endpointList.stream().findFirst().get().getAgentOS();
        }
        String flag = backupTask.getAdvanceParams().get(OracleConstants.STORAGE_SNAPSHOT_FLAG);
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType()) || VerifyUtil.isEmpty(flag)
            || "false".equalsIgnoreCase(flag)) {
            return;
        }
        String snapshotAgents = backupTask.getAdvanceParams().get(SNAPSHOT_AGENTS);
        if (Strings.isBlank(snapshotAgents)) {
            // 如果自身的生产agent是windows操作环境且没选择执行存储快照agent就报错。
            if (OracleConstants.WINDOWS.equalsIgnoreCase(productAgentOsType)) {
                throw new LegoCheckedException(DatabaseErrorCode.LINUX_AGENT_NOT_EXIST,
                    "No Linux OS proxy host is selected.");
            }
            return;
        }
        // 将存储快照对应的agent添加进去
        supplySnapshotAgents(backupTask, snapshotAgents, productAgentOsType);
    }

    private void supplySnapshotAgents(BackupTask backupTask, String snapshotAgents, String productAgentOsType) {
        log.info("start supply snapshot agents");
        List<Endpoint> endpointList = backupTask.getAgents();
        if (endpointList == null) {
            endpointList = new ArrayList<>();
        }
        int beforeSize = endpointList.size();
        log.info("endpointList size:{}", beforeSize);
        String[] ids = snapshotAgents.split(OracleConstants.AGENTS_SEPARATOR);
        Set<String> idSet = new HashSet<>(Arrays.asList(ids));
        for (String id : idSet) {
            if (endpointList.stream().anyMatch(p -> p.getId().equals(id))) {
                continue;
            }
            // 只添加在线的linux系统agent
            ProtectedEnvironment envAgent = oracleBaseService.getEnvironmentById(id);
            if (!LinkStatusEnum.ONLINE.getStatus().toString().equals(envAgent.getLinkStatus())
                || OracleConstants.WINDOWS.equalsIgnoreCase(envAgent.getOsType())) {
                log.warn("selected agent is invalid,id:{}", id);
                continue;
            }
            // agent离线，则过滤掉
            try {
                agentUnifiedService.getHost(envAgent.getEndpoint(), envAgent.getPort());
            } catch (LegoCheckedException e) {
                log.warn("job id: {}, agent: {}, is offline", backupTask.getTaskId(), envAgent.getEndpoint());
                continue;
            }
            Endpoint agentEndpoint = oracleBaseService.getAgentEndpoint(envAgent);
            endpointList.add(agentEndpoint);
        }
        // 如果自身的生产agent是windows操作系统，且没有新增在线的linux主机则报错，
        if (OracleConstants.WINDOWS.equalsIgnoreCase(productAgentOsType) && beforeSize == endpointList.size()) {
            log.error("No Linux nodes is selected.");
            throw new LegoCheckedException(DatabaseErrorCode.LINUX_AGENT_NOT_EXIST,
                "No Linux OS proxy host is selected.");
        }
    }

    /**
     * 检查连通性
     *
     * @param backupTask backupTask
     */
    @Override
    protected void checkConnention(BackupTask backupTask) {
        if (!ResourceSubTypeEnum.ORACLE_CLUSTER.getType().equals(backupTask.getProtectObject().getSubType())) {
            super.checkConnention(backupTask);
            return;
        }
        // 针对集群实例备份的情况，对agents里每一个节点做oracle服务的连通性检查
        // 只要有一个可用的，就可以把备份任务下发下去，并且需要移除不可用的agent节点
        ProtectedResource clusterDatabase = oracleBaseService.getResource(backupTask.getProtectObject().getUuid());
        List<ProtectedEnvironment> agents = oracleBaseService.getEnvironmentById(clusterDatabase.getParentUuid())
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .collect(Collectors.toList());
        for (ProtectedEnvironment agent : agents) {
            AgentBaseDto response = agentUnifiedService.checkApplication(clusterDatabase, agent);
            if (!OracleConstants.SUCCESS.equals(response.getErrorCode())) {
                backupTask.getAgents().removeIf(endpoint -> agent.getEndpoint().equals(endpoint.getIp()));
            }
        }
        if (backupTask.getAgents().isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "All agent network error");
        }
        log.info("oracle check connection finished. task id is: {}, cluster instance available agents is: {}",
            backupTask.getTaskId(),
            backupTask.getAgents().stream().map(Endpoint::getIp).collect(Collectors.joining(",", "[", "]")));
    }

    /**
     * 副本格式
     *
     * @param backupTask backupTask
     * @return 副本格式
     */
    @Override
    protected OptionalInt obtainFormat(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            return OptionalInt.of(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        } else {
            return OptionalInt.of(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        }
    }

    private boolean isBelongToOracle(String subType) {
        return ResourceSubTypeEnum.ORACLE.equalsSubType(subType)
            || ResourceSubTypeEnum.ORACLE_CLUSTER.equalsSubType(subType);
    }

    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<Endpoint> agents = backupTask.getAgents();
        String productAgentOsType = null;
        if (!CollectionUtils.isEmpty(agents) && backupTask.getProtectEnv() != null) {
            List<HostDto> hostList = agents.stream()
                .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
                .collect(Collectors.toList());
            if (hostList.stream().findFirst().isPresent()) {
                productAgentOsType = hostList.stream().findFirst().get().getOsType();
            }
            List<TaskEnvironment> environments =
                hostList.stream().map(AgentDtoUtil::toTaskEnvironment).collect(Collectors.toList());
            backupTask.getProtectEnv().setNodes(environments);
        }

        String flag = backupTask.getAdvanceParams().get(OracleConstants.STORAGE_SNAPSHOT_FLAG);
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType()) || VerifyUtil.isEmpty(flag)
            || "false".equalsIgnoreCase(flag)) {
            return;
        }
        String snapshotAgents = backupTask.getAdvanceParams().get(SNAPSHOT_AGENTS);
        if (Strings.isBlank(snapshotAgents)) {
            return;
        }
        // 将存储快照对应的agent添加进去
        supplySnapshotNodes(backupTask, snapshotAgents, productAgentOsType);
    }

    private void supplySnapshotNodes(BackupTask backupTask, String snapshotAgents, String productAgentOsType) {
        log.info("start supply snapshot nodes");
        List<TaskEnvironment> nodes = backupTask.getProtectEnv().getNodes();
        if (nodes == null) {
            nodes = new ArrayList<>();
        }
        String[] ids = snapshotAgents.split(OracleConstants.AGENTS_SEPARATOR);
        Set<String> idSet = new HashSet<>(Arrays.asList(ids));
        for (String id : idSet) {
            Optional<TaskEnvironment> node = nodes.stream().filter(p -> p.getUuid().equals(id)).findFirst();
            if (node.isPresent()) {
                // 如果生产agent同时被选为存储快照执行agent，打存储快照标志
                node.get().getExtendInfo().put(OracleConstants.STORAGE_SNAPSHOT_AGENT_FLAG, "true");
                continue;
            }
            ProtectedResource tmp = oracleBaseService.getResource(id);
            if (tmp == null) {
                log.warn("agent doesn't exist,id:{}", id);
                continue;
            }
            // 只添加在线的linux系统agent
            ProtectedEnvironment envAgent = oracleBaseService.getEnvironmentById(id);
            if (!LinkStatusEnum.ONLINE.getStatus().toString().equals(envAgent.getLinkStatus())
                || OracleConstants.WINDOWS.equalsIgnoreCase(envAgent.getOsType())) {
                log.warn("selected agent is invalid,id:{}", id);
                continue;
            }
            // agent离线，则过滤掉
            try {
                agentUnifiedService.getHost(envAgent.getEndpoint(), envAgent.getPort());
            } catch (LegoCheckedException e) {
                log.warn("job id: {}, agent: {}, is offline", backupTask.getTaskId(), envAgent.getEndpoint());
                continue;
            }
            TaskEnvironment environment = toTaskEnvironment(tmp);
            Map<String, String> extendInfo = new HashMap<>();
            extendInfo.put(OracleConstants.STORAGE_SNAPSHOT_AGENT_FLAG, "true");
            if (tmp.getExtendInfo() != null) {
                extendInfo.put(ResourceConstants.AGENT_IP_LIST,
                    tmp.getExtendInfo().getOrDefault(ResourceConstants.AGENT_IP_LIST, ""));
            }
            environment.setExtendInfo(extendInfo);
            nodes.add(environment);
        }
        backupTask.getProtectEnv().setNodes(nodes);
    }

    private void fillBackupParams(BackupTask backupTask) {
        Map<String, String> envExtendInfo = backupTask.getProtectEnv().getExtendInfo();
        ProtectedResource resource = oracleBaseService.getResource(backupTask.getProtectObject().getUuid());
        if (backupTask.getProtectEnv().getSubType().equals(ResourceSubTypeEnum.ORACLE_CLUSTER_ENV.getType())) {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        } else {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        }
        // 设置auth信息
        backupTask.getProtectObject().setAuth(resource.getAuth());
        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        // 如果开启了存储快照备份模式，为打快照agent，否则为平衡agent
        String flag = advanceParams.get(OracleConstants.STORAGE_SNAPSHOT_FLAG);
        if (VerifyUtil.isEmpty(flag) || "false".equalsIgnoreCase(flag)) {
            advanceParams.put(BALANCE_AGENTS, advanceParams.get("agents"));
        }

        fillEncKey(backupTask);
        if (ResourceSubTypeEnum.ORACLE_CLUSTER.getType().equals(backupTask.getProtectObject().getSubType())) {
            // 获取集群实例或者是单实例的version，设置到保护对象的extendInfo里
            oracleBaseService.fillVersionToExtendInfo(resource.getVersion(), backupTask.getProtectObject());
            backupTask.getAdvanceParams().put("nodes", resource.getExtendInfo().get(OracleConstants.INSTANCES));
            backupTask.getAdvanceParams().put(DatabaseConstants.MULTI_POST_JOB, "true");
        }
    }

    private void fillEncKey(BackupTask backupTask) {
        Map<String, String> advanceParams = backupTask.getAdvanceParams();
        if (advanceParams.containsKey(ENCRYPTION_ALGORITHM)) {
            advanceParams.put(ENCRYPTION, "true");
            // 设置密码
            String pwd;
            String encPwd;
            String uuid = backupTask.getProtectObject().getUuid();
            Map<String, Object> conditions = new HashMap<>();
            conditions.put("resource_id", uuid);
            BasePage<Copy> copies = copyRestApi.queryCopies(0, 1, conditions);
            // 如果是全量备份或者首次备份就生成新的密码，否则直接取上次全量备份的密码
            if (DatabaseConstants.FULL_BACKUP_TYPE.equals(backupTask.getBackupType())
                || VerifyUtil.isEmpty(copies.getItems())) {
                pwd = RandomPwdUtil.generate(10, SYMBOLS);
                encPwd = encryptorService.encrypt(pwd);
            } else {
                encPwd = oracleBaseService.getResource(uuid).getExtendInfoByKey(ENCRYPTED);
                pwd = encryptorService.decrypt(encPwd);
            }
            backupTask.getProtectObject().getExtendInfo().put(ENC_KEY, pwd);
            backupTask.getProtectObject().getExtendInfo().put(ENCRYPTED, encPwd);
            ProtectedResource updateResource = new ProtectedResource();
            updateResource.setUuid(uuid);
            updateResource.setExtendInfoByKey(ENCRYPTED, encPwd);
            resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
        } else {
            advanceParams.put(ENCRYPTION, "false");
        }
    }

    private void setFileHandleByteAlignmentSwitch(BackupTask task) {
        String version = task.getProtectObject().getVersion();
        if (VerifyUtil.isEmpty(version) || !version.startsWith(OracleConstants.VERSION_12_1)) {
            return;
        }
        for (StorageRepository repo : task.getRepositories()) {
            if (RepositoryTypeEnum.LOG.getType() == repo.getType()
                || RepositoryTypeEnum.DATA.getType() == repo.getType()) {
                Map<String, Object> oldMap = Optional.ofNullable(repo.getExtendInfo()).orElse(Collections.emptyMap());
                Map<String, Object> extendInfo = new HashMap<>(oldMap);
                extendInfo.put(OracleConstants.FILE_HANDLE_BYTE_ALIGNMENT_SWITCH, true);
                repo.setExtendInfo(extendInfo);
            }
        }
    }

    private void setNeedDeleteDtree(BackupTask task) {
        for (StorageRepository repo : task.getRepositories()) {
            if (RepositoryTypeEnum.DATA.getType() == repo.getType()) {
                Map<String, Object> oldMap = Optional.ofNullable(repo.getExtendInfo()).orElse(Collections.emptyMap());
                Map<String, Object> extendInfo = new HashMap<>(oldMap);
                extendInfo.put(OracleConstants.NEED_DELETE_DTREE, false);
                repo.setExtendInfo(extendInfo);
            }
        }
    }

    private TaskEnvironment toTaskEnvironment(ProtectedResource protectedResource) {
        return JsonUtil.read(JsonUtil.json(protectedResource), TaskEnvironment.class);
    }

    private void checkExistRunningInstanceRestoreJob(String resourceId, String dbName) {
        QueryJobRequest conditions = new QueryJobRequest();
        conditions.setTypes(Collections.singletonList(JobTypeEnum.INSTANT_RESTORE.getValue()));
        conditions.setStatusList(Collections.singletonList(JobStatusEnum.RUNNING.name()));
        conditions.setSourceTypes(Arrays.asList(ResourceSubTypeEnum.ORACLE_CLUSTER.getType(),
            ResourceSubTypeEnum.ORACLE.getType()));
        List<JobBo> jobs = jobService.queryJobs(conditions, new PagingParamRequest()).getRecords();
        for (JobBo jobBo : jobs) {
            RestoreTask restoreTask = RestoreTaskHelper.parseFromJobMessage(jobBo.getMessage());
            if (resourceId.equals(restoreTask.getTargetObject().getUuid()) && dbName.toLowerCase(Locale.ROOT)
                .equals(jobBo.getSourceName().toLowerCase(Locale.ROOT))) {
                throw new LegoCheckedException(CommonErrorCode.EXIST_INSTANCE_RESTORE_TASK,
                    "exist instance restore job");
            }
        }
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
