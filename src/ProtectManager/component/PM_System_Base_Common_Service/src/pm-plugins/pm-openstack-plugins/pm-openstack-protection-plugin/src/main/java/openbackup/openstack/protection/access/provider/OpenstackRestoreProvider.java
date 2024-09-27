package openbackup.openstack.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentListener;
import openbackup.data.access.framework.core.common.util.RestoreUtil;
import openbackup.data.protection.access.provider.sdk.anti.ransomware.CopyRansomwareService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.openstack.protection.access.constant.OpenstackConstant;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.MessageTemplate;

import com.alibaba.fastjson.JSON;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * 功能描述: Openstack恢复拦截器
 *
 * @author c30016231
 * @version [OceanProtect X8000 1.3.0]
 * @since 2023-01-10
 */
@Slf4j
@Component
public class OpenstackRestoreProvider implements RestoreInterceptorProvider {
    /**
     * 服务器扩展参数：网络等信息
     */
    public static final String SERVER_EXTEND_INFO = "serverExtendInfo";

    private static final String FLAVOR = "flavor";

    private static final String AVAILABILITY_ZONE = "availabilityZone";

    private static final String NETWORK = "network";

    private static final String NOT_EXIST = "NOT_EXIST";

    private static final String OLD_PLACE = "original";

    private static final String NOT_NEW_DISK = "false";

    private final ResourceService resourceService;

    private final CopyRestApi copyRestApi;

    private final MessageTemplate<String> messageTemplate;

    private CopyRansomwareService copyRansomwareService;

    public OpenstackRestoreProvider(ResourceService resourceService, CopyRestApi copyRestApi,
        MessageTemplate<String> messageTemplate) {
        this.resourceService = resourceService;
        this.copyRestApi = copyRestApi;
        this.messageTemplate = messageTemplate;
    }

    @Autowired
    public void setCopyRansomwareService(CopyRansomwareService copyRansomwareService) {
        this.copyRansomwareService = copyRansomwareService;
    }

    @Override
    public boolean applicable(String subtype) {
        return ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.equalsSubType(subtype);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        String taskId = task.getTaskId();
        String targetId = task.getTargetObject().getUuid();
        log.info("start Openstack restore interception.taskId:{}, targetId:{}", taskId, targetId);
        // 填充恢复高级参数
        fillAdvanceParams(task);
        Copy copy = this.copyRestApi.queryCopyByID(task.getCopyId());
        // 磁盘恢复，原位置恢复，且虚拟机不存在，则不能恢复(前端有拦截)
        if (RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation()) && NOT_EXIST.equals(copy.getResourceStatus())
            && OpenstackConstant.VOLUME_RESTORE.equals(task.getAdvanceParams().get(OpenstackConstant.RESTORE_LEVEL))) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_NOT_EXIST, "resource is not exist");
        }
        // 补充环境扩展信息
        fillEnvironmentExtendInfo(task);
        // 填充虚拟机参数
        fillTargetObject(task);
        // 填充域认证信息
        fillDomainAuth(task);
        // 填充恢复卷信息
        fillSubObjects(task);
        // 填充恢复目标扩展信息
        fillTargetExtendInfo(task);
        // 填充恢复目标位置信息
        fillRestoreTargetLocation(task);
        // 填充恢复模式
        fillRestoreMode(task);
        // 填充网络等信息
        fillNetwork(task, copy);
        // 磁盘恢复原位置场景下，检验restore_volume_map参数，其中当前磁盘和目标恢复磁盘的uuid不能为空
        String restoreLevel = task.getAdvanceParams().get(OpenstackConstant.RESTORE_LEVEL);
        if (OpenstackConstant.VOLUME_RESTORE.equals(restoreLevel)
                && RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            checkSubObjectsNotEmpty(task);
        }
        log.info("end Openstack restore interception.taskId:{}, targetId:{}", taskId, targetId);
        return task;
    }

    private void fillTargetObject(RestoreTask task) {
        TaskResource taskResource = task.getTargetObject();
        Map<String, String> extendInfo = Optional.ofNullable(taskResource.getExtendInfo()).orElse(new HashMap<>());
        extendInfo.put(AVAILABILITY_ZONE, task.getAdvanceParams().remove(AVAILABILITY_ZONE));
        extendInfo.put(FLAVOR, task.getAdvanceParams().remove(FLAVOR));
        extendInfo.put(NETWORK, task.getAdvanceParams().remove(NETWORK));
    }

    private void fillNetwork(RestoreTask task, Copy copy) {
        TaskResource taskResource = task.getTargetObject();
        Map<String, String> extendInfo = taskResource.getExtendInfo();
        if (extendInfo == null) {
            extendInfo = new HashMap<>();
            taskResource.setExtendInfo(extendInfo);
        }
        String properties = copy.getProperties();
        if (!StringUtils.isEmpty(properties)) {
            com.alibaba.fastjson.JSONObject jsonObject = JSON.parseObject(properties);
            String serverInfo = jsonObject.getString(SERVER_EXTEND_INFO);
            extendInfo.put(SERVER_EXTEND_INFO, serverInfo);
        }
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        copyRansomwareService.checkCopyOperationValid(task.getCopyId(),
                RestoreUtil.getRestoreOperation(task.getRestoreType(), task.getTargetLocation()));
        if (OpenstackConstant.VM_RESTORE.equals(task.getAdvanceParams().get(OpenstackConstant.RESTORE_LEVEL))) {
            // 新建虚拟机场景
            TaskResource taskResource = JSON.parseObject(task.getTargetObject().getName(), TaskResource.class);
            // 用于加锁，无UUID加锁会导致恢复新虚拟机只有一个进行
            taskResource.setUuid(UUID.randomUUID().toString());
            taskResource.setType(ResourceTypeEnum.CLOUD_SERVER.getType());
            taskResource.setSubType(ResourceSubTypeEnum.OPENSTACK_CLOUD_SERVER.getType());
            String parentUuid = taskResource.getParentUuid();
            Optional<ProtectedResource> parentResource = resourceService.getBasicResourceById(parentUuid);
            parentResource.ifPresent(protectedResource -> {
                String parentPath = protectedResource.getPath() == null ? "" : protectedResource.getPath();
                taskResource.setPath(parentPath + "/" + taskResource.getName());
            });
            task.setTargetObject(taskResource);
            fillRestoreTargetLocation(task);
        }
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    @Override
    public void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
        TaskResource targetObject = task.getTargetObject();
        resourceService.getResourceById(targetObject.getParentUuid())
            .ifPresent(this::scanProject);
        log.info("Openstack restore postProcess finished. taskId:{}, targetId:{}", task.getTaskId(),
            targetObject.getUuid());
    }

    private void scanProject(ProtectedResource project) {
        JSONObject messageData = new JSONObject();
        messageData.set("uuid", project.getUuid());
        log.info("Openstack send scan project message after restore, projectId:{}", project.getUuid());
        messageTemplate.send(ProtectedEnvironmentListener.SCANNING_ENVIRONMENT_V2, messageData);
    }

    private void fillEnvironmentExtendInfo(RestoreTask task) {
        TaskEnvironment protectEnv = task.getTargetEnv();
        Map<String, String> envExtendInfo = Optional.ofNullable(protectEnv.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(OpenstackConstant.PROJECT_ID, task.getTargetObject().getParentUuid());
        protectEnv.setExtendInfo(envExtendInfo);
    }

    private void fillDomainAuth(RestoreTask restoreTask) {
        TaskResource protectObject = restoreTask.getTargetObject();
        String domainId = protectObject.getExtendInfo().get(OpenstackConstant.DOMAIN_ID_KEY);
        if (Objects.equals(domainId, OpenstackConstant.DEFAULT_DOMAIN_ID)) {
            domainId = restoreTask.getTargetEnv().getExtendInfo().get(OpenstackConstant.SERVICE_ID_KEY);
        }
        boolean shouldDecrypt = true;
        resourceService.getResourceById(shouldDecrypt, domainId)
            .map(ProtectedResource::getAuth)
            .ifPresent(protectObject::setAuth);
        log.info("openstack restore fill domain info, taskId: {}, domainId: {}", restoreTask.getTaskId(), domainId);
    }

    private void fillRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        switch (CopyGeneratedByEnum.getByGenerateType(copy.getGeneratedBy())) {
            case BY_CLOUD_ARCHIVE:
                task.setRestoreMode(RestoreModeEnum.REMOTE_RESTORE.getMode());
                break;
            case BY_TAPE_ARCHIVE:
                task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
                break;
            default:
                task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build Openstack copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    private void fillSubObjects(RestoreTask task) {
        Optional.ofNullable(task.getSubObjects())
            .ifPresent(taskResources -> {
                List<TaskResource> subObjects = taskResources.stream()
                    .map(item -> JSONObject.toBean(item.getName(), TaskResource.class))
                    .collect(Collectors.toList());
                task.setSubObjects(subObjects);
            });
    }

    private void fillAdvanceParams(RestoreTask task) {
        Optional.ofNullable(task.getAdvanceParams())
            .ifPresent(advanceParams ->
                advanceParams.putIfAbsent(OpenstackConstant.RESTORE_LEVEL, OpenstackConstant.VOLUME_RESTORE));
    }

    private void fillTargetExtendInfo(RestoreTask task) {
        String powerState = Optional.ofNullable(task.getAdvanceParams())
            .map(advanceParams -> advanceParams.get(OpenstackConstant.POWER_STATE))
            .orElse(OpenstackConstant.POWER_ON);
        Optional.of(task)
            .map(RestoreTask::getTargetObject)
            .map(TaskResource::getExtendInfo)
            .ifPresent(extendInfo -> extendInfo.put(OpenstackConstant.POWER_STATE, powerState));
    }

    private void fillRestoreTargetLocation(RestoreTask task) {
        TaskResource targetObject = task.getTargetObject();
        String targetLocation = Optional.ofNullable(task.getAdvanceParams())
            .map(advanceParams -> advanceParams.get(OpenstackConstant.RESTORE_LOCATION))
            .orElse(targetObject.getPath());
        targetObject.setTargetLocation(targetLocation);
    }

    private void checkSubObjectsNotEmpty(RestoreTask task) {
        List<TaskResource> subObjects = task.getSubObjects();
        for (TaskResource taskResource : subObjects) {
            String targetVolumeMap = taskResource.getExtendInfo().get(OpenstackConstant.TARGET_VOLUME_KEY);
            JSONObject targetVolume = JSONObject.fromObject(targetVolumeMap);
            String isNewDisk = targetVolume.getString(OpenstackConstant.IS_NEW_DISK);
            if (Boolean.parseBoolean(isNewDisk)) {
                continue;
            }
            String targetUuid = targetVolume.getString(OpenstackConstant.ID_KEY);
            if (VerifyUtil.isEmpty(taskResource.getUuid()) || VerifyUtil.isEmpty(targetUuid)) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "illegal parameter");
            }
        }
    }
}
