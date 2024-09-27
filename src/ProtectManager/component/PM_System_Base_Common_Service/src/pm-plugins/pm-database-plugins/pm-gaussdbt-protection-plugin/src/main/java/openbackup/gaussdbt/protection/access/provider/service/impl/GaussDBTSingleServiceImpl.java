package openbackup.gaussdbt.protection.access.provider.service.impl;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.service.DatabaseRestoreService;
import openbackup.database.base.plugin.service.InstanceProtectionService;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTReleaseTypeEnum;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTSingleStateEnum;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.nio.charset.Charset;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * GaussDBT单机服务
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/14
 */
@Service
@Slf4j
public class GaussDBTSingleServiceImpl implements GaussDBTSingleService {
    private final ResourceService resourceService;

    private final ProviderManager providerManager;

    private final InstanceProtectionService instanceProtectionService;

    private final CopyRestApi copyRestApi;

    private final DatabaseRestoreService databaseRestoreService;

    public GaussDBTSingleServiceImpl(ResourceService resourceService, ProviderManager providerManager,
        InstanceProtectionService instanceProtectionService, CopyRestApi copyRestApi,
        DatabaseRestoreService databaseRestoreService) {
        this.resourceService = resourceService;
        this.providerManager = providerManager;
        this.instanceProtectionService = instanceProtectionService;
        this.copyRestApi = copyRestApi;
        this.databaseRestoreService = databaseRestoreService;
    }

    @Override
    public void checkSingleIsRegistered(ProtectedEnvironment environment) {
        int singleResourceCount = queryGaussDBTSingleNums(environment);
        if (singleResourceCount != IsmNumberConstant.ZERO) {
            log.error("This GaussDBT single is registered. name: {}", environment.getName());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_IS_REGISTERED,
                "This GaussDBT single is registered.");
        }
    }

    @Override
    public void checkSingleInstallUserIsChanged(ProtectedEnvironment environment) {
        int singleResourceCount = queryGaussDBTSingleNums(environment);
        if (singleResourceCount != IsmNumberConstant.ONE) {
            log.error("This GaussDBT single install user is changed.. name: {}", environment.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This GaussDBT single install user is changed.");
        }
    }

    @Override
    public String checkConnection(ProtectedEnvironment environment) {
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            environment);
        ResourceCheckContext context = provider.tryCheckConnection(environment);
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("GaussDBT single check connection result is empty. name: {}", environment.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM,
                "GaussDBT single check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("GaussDBT single check connection failed. name: {}", environment.getName());
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()),
                "GaussDBT single check connection failed.");
        }
        return actionResult.getMessage();
    }

    @Override
    public void fillGaussDBTSingleProperties(ProtectedEnvironment environment, String checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult).toMap(String.class);
        environment.setVersion(messageMap.get(DatabaseConstants.VERSION));
        String agentUuid = getAgentUuid(environment);
        ProtectedResource agent = getResourceById(agentUuid);
        environment.setPath(agent.getEndpoint());
        environment.setEndpoint(agent.getEndpoint());
        environment.setUuid(buildUniqueUuid(agentUuid, environment.getAuth().getAuthKey()));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        environment.setExtendInfoByKey(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        environment.setExtendInfoByKey(GaussDBTConstant.INSTALL_USER_KEY, environment.getAuth().getAuthKey());
        environment.setExtendInfoByKey(GaussDBTConstant.RELEASE_TYPE_KEY,
            GaussDBTReleaseTypeEnum.STAND_ALONE.getType());
        environment.setExtendInfoByKey(GaussDBTConstant.CLUSTER_STATE_KEY, GaussDBTSingleStateEnum.NORMAL.getState());
        environment.setExtendInfoByKey(DatabaseConstants.HOST_ID, agentUuid);
    }

    @Override
    public void checkGaussDTSingleStatus(ProtectedEnvironment environment) {
        log.debug("Enter gaussdbt single health check. uuid: {}.", environment.getUuid());
        try {
            checkConnection(environment);
            updateGaussDBTSingleStatus(environment.getUuid(), GaussDBTSingleStateEnum.NORMAL.getState());
        } catch (LegoCheckedException e) {
            log.error("GaussDBT single health check failed.", e);
            updateGaussDBTSingleStatus(environment.getUuid(), GaussDBTSingleStateEnum.OFFLINE.getState());
        }
    }

    @Override
    public List<TaskEnvironment> getEnvNodes(ProtectedResource resource) {
        return instanceProtectionService.extractEnvNodesBySingleInstance(resource);
    }

    @Override
    public List<Endpoint> getAgents(ProtectedResource resource) {
        List<TaskEnvironment> nodeList = getEnvNodes(resource);
        return nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
    }

    @Override
    public void checkSupportRestore(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resource = JSONObject.fromObject(copy.getResourceProperties());
        databaseRestoreService.checkResourceSubType(copy.getResourceSubType(), task.getTargetObject().getSubType());
        databaseRestoreService.checkVersion(resource.getString(DatabaseConstants.VERSION),
            getResourceById(task.getTargetObject().getUuid()).getVersion());
    }

    private int queryGaussDBTSingleNums(ProtectedEnvironment environment) {
        Map<String, Object> filter = new HashMap<>();
        filter.put(GaussDBTConstant.INSTALL_USER_KEY, environment.getAuth().getAuthKey());
        filter.put(DatabaseConstants.SUB_TYPE, environment.getSubType());
        filter.put(DatabaseConstants.HOST_ID, getAgentUuid(environment));
        return resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter).getTotalCount();
    }

    private String getAgentUuid(ProtectedEnvironment environment) {
        return environment.getDependencies().get(DatabaseConstants.AGENTS).get(IsmNumberConstant.ZERO).getUuid();
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "This protected resource is not exist, id: " + resourceId));
    }

    private String buildUniqueUuid(String agentUuid, String installUser) {
        // 设置唯一UUID
        String envIdentity = ResourceSubTypeEnum.GAUSSDBT_SINGLE.getType() + agentUuid + installUser;
        return UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
    }

    private void updateGaussDBTSingleStatus(String uuid, String status) {
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(uuid);
        newEnv.setExtendInfoByKey(GaussDBTConstant.CLUSTER_STATE_KEY, status);
        resourceService.update(new ProtectedResource[] {newEnv});
    }
}
