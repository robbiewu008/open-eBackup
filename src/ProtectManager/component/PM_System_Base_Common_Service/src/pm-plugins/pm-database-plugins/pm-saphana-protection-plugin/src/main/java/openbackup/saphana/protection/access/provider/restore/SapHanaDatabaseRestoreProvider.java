package openbackup.saphana.protection.access.provider.restore;

import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.constant.SapHanaErrorCode;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;

/**
 * SAP HANA数据库恢复Provider
 *
 * @author wWX1013713
 * @version [DataBackup 1.5.0]
 * @since 2023-05-19
 */
@Component
@Slf4j
public class SapHanaDatabaseRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;

    private final SapHanaResourceService hanaResourceService;

    public SapHanaDatabaseRestoreProvider(CopyRestApi copyRestApi, SapHanaResourceService hanaResourceService) {
        this.copyRestApi = copyRestApi;
        this.hanaResourceService = hanaResourceService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.SAPHANA_DATABASE.equalsSubType(subType);
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        List<LockResourceBo> lockResourceList = new ArrayList<>();
        lockResourceList.add(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
        return lockResourceList;
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 注：目标数据库不在线可以恢复，无需检查连通性
        log.info("Start sap hana database restore interceptor set parameters. Task id: {}", task.getTaskId());
        ProtectedResource targetResource = hanaResourceService.getResourceById(task.getTargetObject().getUuid());
        // 检查是否支持恢复到目标实例
        checkSupportRestore(task, targetResource);
        // 设置目标资源信息
        setTargetObject(task, targetResource);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);
        List<ProtectedResource> savedHostResourceList = SapHanaUtil.parseDbHostProtectedResourceList(targetResource);
        List<ProtectedEnvironment> currHostEnvList = hanaResourceService.queryEnvironments(savedHostResourceList);
        List<TaskEnvironment> taskEnvList = SapHanaUtil.convertEnvListToTaskEnvList(currHostEnvList);
        // 设置目标环境扩展参数
        setTargetEnvExtendInfo(task, taskEnvList.size());
        // 设置目标环境nodes
        task.getTargetEnv().setNodes(taskEnvList);
        // 设置agents
        task.setAgents(SapHanaUtil.convertEnvListToEndpointList(currHostEnvList));
        // 设置目标数据库扩展参数
        setTargetObjectExtendInfo(task, targetResource);
        // 设置恢复存储仓
        setRestoreRepositories(task);
        // 设置高级参数
        setRestoreAdvanceParams(task);
        log.info("End sap hana database restore interceptor set parameters. Task id: {}", task.getTaskId());
        return task;
    }

    private void checkSupportRestore(RestoreTask task, ProtectedResource targetResource) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        Map<String, String> srcResExtendInfo = resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO)
            .toMap(String.class);
        Map<String, String> tgtResExtendInfo = Optional.ofNullable(targetResource.getExtendInfo())
            .orElseGet(HashMap::new);
        // 原位置恢复只检查节点数目
        if (RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            checkNodesNumForRestore(srcResExtendInfo, tgtResExtendInfo);
            return;
        }
        checkVersionForRestore(resourceJson.getString(DatabaseConstants.VERSION), targetResource.getVersion());
        checkDbTypeForRestore(srcResExtendInfo, tgtResExtendInfo, copy.getResourceName(), targetResource.getName());
        checkSystemIdForRestore(srcResExtendInfo, tgtResExtendInfo);
        checkNodesNumForRestore(srcResExtendInfo, tgtResExtendInfo);
    }

    private static void checkVersionForRestore(String srcVersion, String tgtVersion) {
        if (VerifyUtil.isEmpty(srcVersion) || VerifyUtil.isEmpty(tgtVersion)) {
            log.error("The source database version or the target database version is empty, "
                + "source version: {}, target version: {}.", srcVersion, tgtVersion);
            throw new LegoCheckedException(SapHanaErrorCode.VERSION_DISMATCH, new String[] {srcVersion, tgtVersion},
                "The source database version or the target database version is empty.");
        }
        String[] srcVersions = srcVersion.split("\\.");
        String[] tgtVersions = tgtVersion.split("\\.");
        // 从完整版本号“2.00.020.00.1500920972”中取主版本“2”对比，不能从高版本往低版本恢复
        if (Integer.parseInt(srcVersions[0]) > Integer.parseInt(tgtVersions[0])) {
            log.error("The source database version and the target database version do not match, "
                + "source version: {}, target version: {}.", srcVersion, tgtVersion);
            throw new LegoCheckedException(SapHanaErrorCode.VERSION_DISMATCH, new String[] {srcVersion, tgtVersion},
                "The source database version or the target database version do not match.");
        }
    }

    private static void checkDbTypeForRestore(Map<String, String> srcResExtendInfo,
        Map<String, String> tgtResExtendInfo, String srcName, String tgtName) {
        String srcDbType = srcResExtendInfo.get(SapHanaConstants.SAP_HANA_DB_TYPE);
        String tgtDbType = tgtResExtendInfo.get(SapHanaConstants.SAP_HANA_DB_TYPE);
        if (StringUtils.isEmpty(srcDbType) || StringUtils.isEmpty(tgtDbType)) {
            log.warn("The sap hana database type of the source database or the target database is empty.");
            throw new LegoCheckedException(SapHanaErrorCode.SYSTEM_TENANT_NOT_RECOVER_EACH_OTHER,
                new String[] {srcName, tgtName},
                "The sap hana database type of the source database or the target database is empty.");
        }
        // 系统数据库和租户数据库间不支持恢复
        if (!StringUtils.equals(srcDbType, tgtDbType)) {
            log.error("The sap hana database do not support restore to different database type, source db type: {},"
                + " target db type: {}.", srcDbType, tgtDbType);
            throw new LegoCheckedException(SapHanaErrorCode.SYSTEM_TENANT_NOT_RECOVER_EACH_OTHER,
                new String[] {srcName, tgtName},
                "The sap hana database do not support restore to different database type.");
        }
    }

    private static void checkSystemIdForRestore(Map<String, String> srcResExtendInfo,
        Map<String, String> tgtResExtendInfo) {
        String srcSystemId = srcResExtendInfo.get(SapHanaConstants.SYSTEM_ID);
        String tgtSystemId = tgtResExtendInfo.get(SapHanaConstants.SYSTEM_ID);
        if (StringUtils.isEmpty(srcSystemId) || StringUtils.isEmpty(tgtSystemId)) {
            log.warn("The system id of the source database or the target database is empty, source system id: {},"
                + " target system id: {}.", srcSystemId, tgtSystemId);
            throw new LegoCheckedException(SapHanaErrorCode.SYSTEM_ID_NOT_EQUAL,
                new String[] {srcSystemId, tgtSystemId},
                "The system id of the source database or the target database is empty.");
        }
        // systemId不一致不支持恢复
        if (!StringUtils.equals(srcSystemId, tgtSystemId)) {
            log.error(
                "The system id of the source database and the target database are inconsistent, source system id: {},"
                    + " target system id: {}.", srcSystemId, tgtSystemId);
            throw new LegoCheckedException(SapHanaErrorCode.SYSTEM_ID_NOT_EQUAL,
                new String[] {srcSystemId, tgtSystemId},
                "The system id of the source database and the target database are inconsistent.");
        }
    }

    private static void checkNodesNumForRestore(Map<String, String> srcResExtendInfo,
        Map<String, String> tgtResExtendInfo) {
        String srcNodesInfo = srcResExtendInfo.get(SapHanaConstants.NODES);
        String tgtNodesInfo = tgtResExtendInfo.get(SapHanaConstants.NODES);
        if (StringUtils.isEmpty(srcNodesInfo) || StringUtils.isEmpty(tgtNodesInfo)) {
            log.warn("The nodes info of the source database or the target database is empty.");
            throw new LegoCheckedException(SapHanaErrorCode.TOPOLOGY_DISMATCH,
                "The nodes info of the source database or the target database is empty.");
        }
        JSONArray srcNodesJsonArray = JSONArray.fromObject(srcNodesInfo);
        JSONArray tgtNodesJsonArray = JSONArray.fromObject(tgtNodesInfo);
        // 源数据库和目标数据库节点数必须一致
        if (srcNodesJsonArray.size() != tgtNodesJsonArray.size()) {
            log.error("The number of nodes in the source datbase and the target database are inconsistent, source db "
                + "nodes size: {}, target db nodes size: {}.", srcNodesJsonArray.size(), tgtNodesJsonArray.size());
            throw new LegoCheckedException(SapHanaErrorCode.TOPOLOGY_DISMATCH,
                "The number of nodes in the source datbase and the target database are inconsistent.");
        }
    }

    private void setTargetObject(RestoreTask task, ProtectedResource targetResource) {
        ProtectedResource instResource = hanaResourceService.getResourceById(task.getTargetObject().getParentUuid());
        TaskResource taskResource;
        // 原位置默认设置的副本中资源信息
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            taskResource = task.getTargetObject();
        } else {
            taskResource = new TaskResource();
            BeanUtils.copyProperties(targetResource, taskResource);
        }
        // 恢复目标位置设置为“实例名称/数据库名称”
        taskResource.setTargetLocation(
            String.format(Locale.ROOT, "%s/%s", instResource.getName(), targetResource.getName()));
        task.setTargetObject(taskResource);
    }

    private void setTargetEnvExtendInfo(RestoreTask task, int hostsNum) {
        Map<String, String> envExtendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        TaskResource taskResource = task.getTargetObject();
        Map<String, String> taskResourceExtInfo = Optional.of(taskResource.getExtendInfo()).orElseGet(HashMap::new);
        String hanaDbType = Optional.ofNullable(taskResourceExtInfo.get(SapHanaConstants.SAP_HANA_DB_TYPE))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "The sapHanaDbType parameter of target database is empty."));
        String deployType = SapHanaUtil.getDeployType(hostsNum, hanaDbType);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployType);
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private void setTargetObjectExtendInfo(RestoreTask task, ProtectedResource resource) {
        Map<String, String> targetObjectExtendInfo = Optional.ofNullable(task.getTargetObject().getExtendInfo())
            .orElseGet(HashMap::new);
        targetObjectExtendInfo.put(DatabaseConstants.VERSION, resource.getVersion());
        task.getTargetObject().setExtendInfo(targetObjectExtendInfo);
    }

    private JSONObject getCopyResource(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        return JSONObject.fromObject(copy.getResourceProperties());
    }

    private void setRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElseGet(HashMap::new);
        advanceParams.put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        JSONObject copyResource = getCopyResource(task.getCopyId());
        advanceParams.put(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY,
            copyResource.getString(DatabaseConstants.VERSION));
        task.setAdvanceParams(advanceParams);
    }

    private void setRestoreRepositories(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        // 归档副本（全量）恢复时删除日志仓
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            log.info("The current restore task is an archive copy restore, copy id: {}, copy generated by: {}, "
                + "task id: {}.", task.getCopyId(), generatedBy, task.getTaskId());
            List<StorageRepository> oriRepositories = Optional.ofNullable(task.getRepositories())
                .orElse(new ArrayList<>());
            List<StorageRepository> newRepositories = new ArrayList<>();
            for (StorageRepository tmpRepo : oriRepositories) {
                if (tmpRepo.getType() != RepositoryTypeEnum.LOG.getType()) {
                    newRepositories.add(tmpRepo);
                }
            }
            task.setRepositories(newRepositories);
        }
    }

    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature feature = new RestoreFeature();
        feature.setShouldCheckEnvironmentIsOnline(false);
        return feature;
    }
}
