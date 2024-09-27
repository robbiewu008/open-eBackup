package openbackup.oracle.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceDeleteContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.oracle.bo.OracleStorage;
import openbackup.oracle.constants.OracleConstants;
import openbackup.oracle.service.OracleBaseService;
import openbackup.oracle.utils.StorageCheckUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * oracle单实例注册provider
 *
 * @author c30038333
 * @since 2022-12-27
 */
@Component
@Slf4j
public class OracleSingleNodeDatabaseProvider implements ResourceProvider {
    private final ResourceService resourceService;
    private final ProviderManager providerManager;
    private final InstanceResourceService instanceResourceService;
    private final OracleBaseService oracleBaseService;
    private final AgentUnifiedService agentUnifiedService;

    public OracleSingleNodeDatabaseProvider(ResourceService resourceService, ProviderManager providerManager,
        InstanceResourceService instanceResourceService, OracleBaseService oracleBaseService,
        AgentUnifiedService agentUnifiedService) {
        this.resourceService = resourceService;
        this.providerManager = providerManager;
        this.instanceResourceService = instanceResourceService;
        this.oracleBaseService = oracleBaseService;
        this.agentUnifiedService = agentUnifiedService;
    }

    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.ORACLE.getType().equals(object.getSubType());
    }

    @Override
    public void beforeCreate(ProtectedResource resource) {
        // 校验存储信息
        checkStorage(resource);
        checkConnect(resource);
        // 校验是否已经存在实例
        checkInstanceExist(resource);
        // 设置path信息，否则复制的时候会报错
        String envId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        ProtectedEnvironment environment = oracleBaseService.getEnvironmentById(envId);
        resource.setPath(environment.getEndpoint());
        // 刷新子实例的集群状态
        oracleBaseService.refreshClusterInstanceActiveStandby(resource, environment);
        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        // 设置使用过的存储资源，用于删除资源上残留的保护组
        oracleBaseService.updateUsedStorageInfo(resource);
    }

    private void checkInstanceExist(ProtectedResource resource) {
        String hostId = resource.getExtendInfo().get(DatabaseConstants.HOST_ID);
        String port = resource.getExtendInfo().get(DatabaseConstants.PORT);
        Map<String, Object> cons = new HashMap<>();
        String instName = getDbExtendInfo(resource).get(OracleConstants.INST_NAME);
        cons.put(DatabaseConstants.HOST_ID, hostId);
        cons.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.ORACLE.getType());
        cons.put(DatabaseConstants.PORT, port);
        cons.put(OracleConstants.INST_NAME, instName);
        List<ProtectedResource> records = resourceService.query(0, 1, cons).getRecords();
        if (records.size() != 0) {
                throw new DataProtectionAccessException(CommonErrorCode.PROTECTED_ENV_REPEATED,
                        new String[]{}, "Oracle instance is already exist.");
        }
    }


    private void checkConnect(ProtectedResource resource) {
        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
                resource);
        ResourceCheckContext context = provider.tryCheckConnection(resource);
        checkConnectResult(resource, context);
    }

    private static void checkConnectResult(ProtectedResource resource, ResourceCheckContext context) {
        if (VerifyUtil.isEmpty(context.getActionResults())) {
            log.error("oracle instance check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check connection result is empty.");
        }
        ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
        if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
            log.error("oracle instance check connection failed. name: {}", resource.getName());
            if (VerifyUtil.isEmpty(actionResult.getDetailParams())) {
                throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), "check connection failed.");
            }
            String[] params = Optional.ofNullable(actionResult.getDetailParams())
                    .map(e -> e.toArray(new String[0]))
                    .orElse(new String[0]);
            throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), params,
                    "check connection failed.");
        }
    }

    @Override
    public ResourceDeleteContext preHandleDelete(ProtectedResource resource) {
        log.info("start oracle single node database delete pre check, resourceId:{}", resource.getUuid());
        if (!oracleBaseService.isAnonymizationDeletable(resource.getUuid())) {
            throw new LegoCheckedException(CommonErrorCode.ANONYMIZATION_JOB_IS_RUNNING, new String[] {},
                    "resource has running anonymization job");
        }
        // 解除保护的时候，存储层备份需删除生产环境上保护组
        oracleBaseService.cleanDoradoEnv(resource);
        return ResourceDeleteContext.defaultValue();
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        checkStorage(resource);
        checkConnect(resource);
        // 刷新子实例的集群状态
        oracleBaseService.refreshClusterInstanceActiveStandby(resource,
                getAgentBySingleInstanceUuid(resource.getUuid()));
        // 设置实例认证状态
        resource.getExtendInfo().put(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        // 设置使用过的存储资源，用于删除资源上残留的保护组
        oracleBaseService.updateUsedStorageInfo(resource);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedResource resource) {
        log.info("Oracle single scan started, resource id :{}", resource.getUuid());
        oracleBaseService.refreshClusterInstanceActiveStandby(resource, resource.getEnvironment());
        log.info("Oracle single scan finished, resource id :{}", resource.getUuid());
        return Collections.singletonList(resource);
    }

    @Override
    public void healthCheck(ProtectedResource resource) {
        instanceResourceService.healthCheckSingleInstance(resource);
    }

    private ProtectedEnvironment getAgentBySingleInstanceUuid(String singleInstanceUuid) {
        ProtectedResource singleInstanceResources = resourceService.getResourceById(singleInstanceUuid)
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "object not exists"));
        return singleInstanceResources.getEnvironment();
    }

    private void checkStorage(ProtectedResource resource) {
        log.info("start check storage");
        List<OracleStorage> storages = getStorages(resource);
        if (CollectionUtils.isEmpty(storages)) {
            return;
        }
        List<OracleStorage> authStorages = getAuthStorageInfo(resource);
        if (CollectionUtils.isEmpty(authStorages)) {
            return;
        }

        // 存储数量只能1个
        if (storages.size() > OracleConstants.ORACLE_SINGLE_STORAGE_MAN_NUM) {
            log.error("Storage count({}) over limit.", storages.size());
            throw new LegoCheckedException(CommonErrorCode.STORAGE_OVER_LIMIT,
                new String[] {String.valueOf(OracleConstants.ORACLE_CLUSTER_STORAGE_MAN_NUM)},
                "Storage count over limit.");
        }

        // 校验ip,端口
        storages.forEach(storage -> {
            StorageCheckUtil.checkIp(storage.getIpList(), resource.getName());
            StorageCheckUtil.checkPort(storage.getPort(), resource.getName());
        });
        // 校验证书
        authStorages.forEach(StorageCheckUtil::checkCert);

        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resource);

        ProtectedResource clone = JSONObject.fromObject(resource).toBean(ProtectedResource.class);
        clone.getExtendInfo().put("isCheckStorage", "true");
        String ipList = authStorages.get(0).getIpList();
        ResourceCheckContext context = new ResourceCheckContext();
        for (String ip : ipList.split(",")) {
            authStorages.get(0).setIpList(ip);
            clone.getAuth().getExtendInfo().put("storages", JsonUtil.json(authStorages));
            context = provider.tryCheckConnection(clone);
            ActionResult actionResult = context.getActionResults().get(IsmNumberConstant.ZERO);
            if (actionResult.getCode() == DatabaseConstants.SUCCESS_CODE) {
                break;
            }
        }
        checkConnectResult(clone, context);
        log.info("check storage end");
    }

    private List<OracleStorage> getStorages(ProtectedResource resource) {
        return Optional.of(resource)
            .map(ProtectedResource::getExtendInfo)
            .map(extendInfo -> extendInfo.get(DatabaseConstants.STORAGES))
            .map(storage -> JSONArray.toCollection(JSONArray.fromObject(storage), OracleStorage.class))
            .orElse(new ArrayList<>());
    }

    private List<OracleStorage> getAuthStorageInfo(ProtectedResource resource) {
        return Optional.of(resource)
            .map(ProtectedResource::getAuth)
            .map(Authentication::getExtendInfo)
            .map(extendInfo -> extendInfo.get(DatabaseConstants.STORAGES))
            .map(storage -> JSONArray.toCollection(JSONArray.fromObject(storage), OracleStorage.class))
            .orElse(new ArrayList<>());
    }

    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = new ResourceFeature();
        // oracle实例名称可以重复，不检查实例重名
        resourceFeature.setShouldCheckResourceNameDuplicate(false);
        return resourceFeature;
    }

    private Map<String, String> getDbExtendInfo(ProtectedResource protectedResource) {
        ProtectedEnvironment environment = protectedResource.getEnvironment();
        AppEnvResponse appEnvResponse = agentUnifiedService.getClusterInfo(protectedResource, environment);
        if (appEnvResponse == null || appEnvResponse.getExtendInfo() == null) {
            throw new DataProtectionAccessException(CommonErrorCode.WRONG_GET_DATABASE_INFORMATION, new String[]{},
                    "get oracle cluster error.");
        }
        return appEnvResponse.getExtendInfo();
    }
}
