package openbackup.gaussdbdws.protection.access.interceptor.restore;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.gaussdbdws.protection.access.constant.DwsConstant;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.gaussdbdws.protection.access.util.DwsBuildRepositoryUtil;
import openbackup.gaussdbdws.protection.access.util.DwsTaskEnvironmentUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.job.JobCenterRestApi;
import openbackup.system.base.sdk.job.model.JobLogBo;
import openbackup.system.base.sdk.job.model.JobLogLevelEnum;
import openbackup.system.base.sdk.job.model.request.UpdateJobRequest;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;

import com.alibaba.fastjson.JSON;

import com.google.common.collect.ImmutableMap;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * GaussDbDws 恢复任务基础数据Provider
 *
 * @author nwx1077006
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-01
 */
@Slf4j
@Component
public class GaussDBDWSBaseRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final GaussDBBaseService gaussDBBaseService;

    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;

    private final CopyRestApi copyRestApi;

    private final ClusterNativeApi clusterNativeApi;

    private EncryptorService encryptorService;

    private DeployTypeService deployTypeService;

    private JobCenterRestApi jobCenterRestApi;

    private final ResourceService resourceService;

    public GaussDBDWSBaseRestoreInterceptor(ProtectedEnvironmentService protectedEnvironmentService,
        GaussDBBaseService gaussDBBaseService, ProtectedEnvironmentRetrievalsService envRetrievalsService,
        CopyRestApi copyRestApi, ClusterNativeApi clusterNativeApi, ResourceService resourceService,
        JobCenterRestApi jobCenterRestApi) {
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.gaussDBBaseService = gaussDBBaseService;
        this.envRetrievalsService = envRetrievalsService;
        this.copyRestApi = copyRestApi;
        this.clusterNativeApi = clusterNativeApi;
        this.resourceService = resourceService;
        this.jobCenterRestApi = jobCenterRestApi;
    }

    @Autowired
    public void setEncryptorService(EncryptorService encryptorService) {
        this.encryptorService = encryptorService;
    }

    @Autowired
    public void setDeployTypeService(DeployTypeService deployTypeService) {
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String object) {
        return Arrays.asList(ResourceSubTypeEnum.GAUSSDB_DWS.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType(), ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType(),
            ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType()).contains(object);
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check dws restore, taskId: {}", task.getTaskId());
        // schema集表级/集群表级新位置恢复重名检查，有重名且禁止恢复的schema下发任务失败
        String resourceSubType = task.getAdvanceParams().getOrDefault("resourceSubType", "");
        boolean isTableLevelRestore = ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType().equals(resourceSubType)
            || (ResourceSubTypeEnum.GAUSSDB_DWS.getType().equals(resourceSubType) && task.getSubObjects() != null);
        if (RestoreLocationEnum.NEW.equals(task.getTargetLocation()) && isTableLevelRestore
            && !checkTargetEnvDuplicateSchemaName(task)) {
            log.error("Schema name already exists in new location and is not allowed to restore.");
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED,
                "Schema name already exists in new location and is not allowed to restore");
        }
        gaussDBBaseService.checkLinkStatus(task.getTargetEnv().getUuid(),
            Optional.of(task.getAdvanceParams().get(DwsConstant.EXTEND_INFO_KEY_SUB_TYPE))
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                    "Protected environment is not exists!")));
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        supplyAgent(task);
        supplyNodes(task);
        supplyTargetEnv(task);
        supplyRestoreMode(task);
        supplyAdvancedParams(task);
        supplySubObjects(task);
        supplyTargetObject(task);
        supplyAuth(task);
        return task;
    }

    @Override
    public void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
        // 数据库应用恢复任务成功后，也需要在后置处理中，指定恢复目标资源的下次备份任务为全量（当本次恢复为细粒度恢复时，下次备份任务不设置为全量）
        log.info("Dws restore type is {}", task.getRestoreType());
        if (StringUtils.equals(RestoreTypeEnum.FLR.getType(), task.getRestoreType())) {
            return;
        }
        super.postProcess(task, jobStatus);
    }

    /**
     * GDS集群需要适配新位置的数据库信息
     *
     * @param task 恢复任务
     */
    private void supplyTargetObject(RestoreTask task) {
        if (!RestoreTypeEnum.FLR.getType().equals(task.getRestoreType()) && task.getTargetLocation()
            .getLocation()
            .equals(RestoreLocationEnum.NEW.getLocation())) {
            Map<String, String> extendInfo = Optional.ofNullable(task.getTargetObject().getExtendInfo())
                .orElse(new HashMap<>());
            Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
            extendInfo.put(DwsConstant.EXTEND_INFO_KEY_DATABASE,
                advanceParams.get(DwsConstant.EXTEND_INFO_KEY_DATABASE));
            task.getTargetObject().setExtendInfo(extendInfo);
        }
    }

    private void supplyAdvancedParams(RestoreTask task) {
        Map<String, String> advancedParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        advancedParams.put(DwsConstant.EXTEND_INFO_KEY_TARGET_LOCATION, task.getTargetLocation().getLocation());
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject properties = JSONObject.fromObject(copy.getProperties());
        String backupToolTypeVaules = properties.getString(DwsConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE);
        advancedParams.put(DwsConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE, backupToolTypeVaules);
        DwsBuildRepositoryUtil.addSpeedStatisticsAdvanceParam(advancedParams, DwsConstant.ADVANCE_PARAMS_KEY_TOOL_TYPE,
            TaskUtil.SPEED_STATISTICS);
        gaussDBBaseService.modifyAdvanceParams(advancedParams, task.getTargetEnv().getUuid());
        task.setAdvanceParams(advancedParams);
    }

    /**
     * 细粒度恢复设置subObjects
     *
     * @param task 恢复任务
     */
    private void supplySubObjects(RestoreTask task) {
        if (RestoreTypeEnum.FLR.getType().equals(task.getRestoreType())) {
            List<TaskResource> subObjects = Optional.ofNullable(task.getSubObjects())
                .orElse(new ArrayList<>())
                .stream()
                .map(item -> JSON.parseObject(item.getName(), TaskResource.class))
                .collect(Collectors.toList());
            log.info("build restore task subObjects success, request id:{}", task.getRequestId());
            task.setSubObjects(subObjects);
        }
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void supplyRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.REMOTE_RESTORE.getMode());
        } else if (CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build GaussDBDWS copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    private void supplyNodes(RestoreTask task) {
        task.getTargetEnv().setNodes(gaussDBBaseService.supplyNodes(task.getTargetEnv().getUuid()));
    }

    private void supplyAgent(RestoreTask task) {
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap =
            envRetrievalsService.collectConnectableResources(task.getTargetEnv().getUuid());
        List<Endpoint> endpointList = protectedResourceMap.values()
            .stream()
            .flatMap(List::stream)
            .map(this::toEndpoint)
            .collect(Collectors.toList());
        gaussDBBaseService.addSupplyAgent(endpointList, task.getTargetEnv().getUuid(), "DWS-clusterPlugin");
        log.info("DWS restore(requestId:{}) supply agent count:{}", task.getRequestId(), endpointList.size());
        task.setAgents(endpointList);
    }

    private void supplyTargetEnv(RestoreTask task) {
        String uuid = task.getTargetObject().getUuid();
        Optional<ProtectedResource> resourceById = resourceService.getResourceById(task.getTargetObject().getUuid());
        if (!resourceById.isPresent()) {
            uuid = task.getTargetEnv().getUuid();
        }
        String isAllowRestoreFlag = resourceService.judgeResourceRestoreLevel(uuid);
        log.info("Dws restore get isAllowRestoreFlag: {}, resource: {}", isAllowRestoreFlag,
            task.getTargetEnv().getUuid());
        if (StringUtils.equals(isAllowRestoreFlag, ResourceConstants.NOT_ALLOW_RESTORE)) {
            JobLogBo jobLogBo = new JobLogBo();
            jobLogBo.setJobId(task.getTaskId());
            jobLogBo.setStartTime(System.currentTimeMillis());
            jobLogBo.setLogInfo(DwsConstant.DATABASE_RESTORE_FAIL_NOT_ALLOW_LABEL);
            jobLogBo.setLevel(JobLogLevelEnum.ERROR.getValue());
            UpdateJobRequest request = new UpdateJobRequest();
            request.setJobLogs(Collections.singletonList(jobLogBo));
            jobCenterRestApi.updateJob(task.getTaskId(), request);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
                "Dws-GaussDb  " + task.getTargetObject().getUuid() + " not allow restore");
        }
        ProtectedEnvironment environment = protectedEnvironmentService.getEnvironmentById(
            task.getTargetEnv().getUuid());
        DwsTaskEnvironmentUtil.initNodeType(task.getTargetEnv(), environment);
        DwsTaskEnvironmentUtil.initProtectEnvOfDwsUser(task.getTargetEnv(), environment.getAuth().getAuthKey());
        DwsTaskEnvironmentUtil.initProtectEnvOfEnvFile(task.getTargetEnv(),
            environment.getExtendInfo().get(DwsConstant.EXTEND_INFO_KEY_ENV_FILE));
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
    }

    private Endpoint toEndpoint(ProtectedEnvironment protectedEnvironment) {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(protectedEnvironment.getUuid());
        log.debug("DWS cluster restore add agent(ID:{}).", protectedEnvironment.getUuid());
        endpoint.setIp(protectedEnvironment.getEndpoint());
        endpoint.setPort(protectedEnvironment.getPort());
        return endpoint;
    }

    private void supplyAuth(RestoreTask task) {
        DwsBuildRepositoryUtil.addRepositoriesAuth(task.getRepositories(), clusterNativeApi, encryptorService,
            deployTypeService, false);
    }

    private boolean checkTargetEnvDuplicateSchemaName(RestoreTask task) {
        log.info("Check target env duplicate schema name...");
        List<String> tables = Optional.ofNullable(task.getSubObjects())
            .orElse(Collections.emptyList())
            .stream()
            .map(item -> JSON.parseObject(item.getName(), TaskResource.class).getName())
            .collect(Collectors.toList());
        log.info("Restore tables: {}", tables);
        if (tables.isEmpty()) {
            return false;
        }
        TaskEnvironment targetEnv = task.getTargetEnv();
        if (targetEnv == null) {
            return false;
        }
        String targetRootUuid = targetEnv.getUuid();
        Set<String> schemas = tables.stream().map(table -> table.split("/")[0] + "/" + table.split("/")[1])
            .collect(Collectors.toSet());
        log.info("Restore schemas: {}", schemas);
        Set<String> notAllowRestoreSchemaSet = getNotAllowRestoreSchemaSet(targetRootUuid);
        log.info("Not allow restore schema set: {}", notAllowRestoreSchemaSet);
        for (String schema : schemas) {
            if (notAllowRestoreSchemaSet.contains(schema)) {
                log.error("{} is not allowed to restore.", schema);
                return false;
            }
        }
        return true;
    }

    private Set<String> getNotAllowRestoreSchemaSet(String rootUuid) {
        Set<String> notAllowRestoreSchemaSet = new HashSet<>();
        PageListResponse<ProtectedResource> result = resourceService.query(
            true, 0, DwsConstant.SCHEMA_SPECIFICATIONS,
            ImmutableMap.of("rootUuid", rootUuid, "subType", ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType()));
        List<ProtectedResource> schemas = result.getRecords();
        log.info("Query result size: {}", schemas.size());
        for (ProtectedResource schema : schemas) {
            Map<String, String> extendInfo = schema.getExtendInfo();
            if (extendInfo == null) {
                continue;
            }
            if (!"true".equals(extendInfo.getOrDefault("isAllowRestore", "false"))) {
                String[] schemaList = extendInfo.getOrDefault("table", "").split(",");
                notAllowRestoreSchemaSet.addAll(Arrays.asList(schemaList));
            }
        }
        return notAllowRestoreSchemaSet;
    }
}
