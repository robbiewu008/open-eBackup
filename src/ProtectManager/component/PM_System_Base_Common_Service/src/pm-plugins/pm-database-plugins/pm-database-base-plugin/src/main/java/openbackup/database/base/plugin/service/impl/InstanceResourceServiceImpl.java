package openbackup.database.base.plugin.service.impl;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.CheckAppReq;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.ClusterInstanceOnlinePolicy;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.FaultEnum;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.constants.LegoInternalAlarm;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.alarm.CommonAlarmService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 数据库的实例资源服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-08-24
 */
@Service
@Slf4j
public class InstanceResourceServiceImpl implements InstanceResourceService {
    private static final String HEALTH_ALARM_CODE = "0x6403320005";

    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final CommonAlarmService commonAlarmService;

    private final MemberClusterService memberClusterService;

    /**
     * 构造方法
     *
     * @param resourceService resourceService
     * @param agentUnifiedService agentUnifiedService
     * @param commonAlarmService  commonAlarmService
     * @param memberClusterService memberClusterService
     */
    public InstanceResourceServiceImpl(ResourceService resourceService, AgentUnifiedService agentUnifiedService,
        CommonAlarmService commonAlarmService, MemberClusterService memberClusterService) {
        this.resourceService = resourceService;
        this.agentUnifiedService = agentUnifiedService;
        this.commonAlarmService = commonAlarmService;
        this.memberClusterService = memberClusterService;
    }

    @Override
    public AgentBaseDto checkIsClusterInstance(ProtectedResource resource) {
        ProtectedResource subInstance = getSubInstance(resource);
        AppEnv appEnv = buildAppEnv(resource, subInstance);
        Application application = buildApplication(resource, subInstance);
        return checkClusterInstanceByAgent(appEnv, application);
    }

    @Override
    public AgentBaseDto checkClusterInstance(ProtectedResource resource) {
        ProtectedResource subInstance = getSubInstance(resource);
        AppEnv appEnv = buildAppEnv(resource, subInstance);
        Application application = buildApplication(resource, subInstance);
        return checkClusterInstance(appEnv, application);
    }

    private ProtectedResource getSubInstance(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This sub instance is empty."));
    }

    private AppEnv buildAppEnv(ProtectedResource resource, ProtectedResource subInstance) {
        ProtectedResource agent = subInstance.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ERR_PARAM, "This agent resource is empty."));
        Map<String, List<ProtectedEnvironment>> agentMap = resource.getEnvironment()
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .collect(Collectors.groupingBy(ProtectedEnvironment::getUuid));
        ProtectedEnvironment environment = agentMap.get(agent.getUuid()).get(IsmNumberConstant.ZERO);
        AppEnv appEnv = BeanTools.copy(environment, AppEnv::new);
        appEnv.setExtendInfo(buildAppEnvExtendInfo(appEnv, resource));
        appEnv.setSubType(resource.getEnvironment().getSubType());
        return appEnv;
    }

    private Map<String, String> buildAppEnvExtendInfo(AppEnv appEnv, ProtectedResource resource) {
        Map<String, String> extendInfo = Optional.ofNullable(appEnv.getExtendInfo()).orElseGet(HashMap::new);
        extendInfo.put(DatabaseConstants.ALL_NODES, getAllNodes(resource));
        extendInfo.putAll(resource.getExtendInfo());
        return extendInfo;
    }

    private String getAllNodes(ProtectedResource resource) {
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(this::getIpAndPort)
            .collect(Collectors.joining(DatabaseConstants.SPLIT_CHAR));
    }

    private String getIpAndPort(ProtectedResource subInstance) {
        return subInstance.getExtendInfoByKey(DatabaseConstants.SERVICE_IP) + DatabaseConstants.IP_PORT_SPLIT_CHAR
            + subInstance.getExtendInfoByKey(DatabaseConstants.INSTANCE_PORT);
    }

    private Application buildApplication(ProtectedResource resource, ProtectedResource subInstance) {
        Application application = BeanTools.copy(subInstance, Application::new);
        application.setSubType(resource.getSubType());
        if (VerifyUtil.isEmpty(application.getAuth())) {
            application.setAuth(resource.getAuth());
        }
        return application;
    }

    private AgentBaseDto checkClusterInstance(AppEnv appEnv, Application application) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        checkAppReq.setApplication(application);
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(application.getSubType(), appEnv.getEndpoint(), appEnv.getPort(),
                checkAppReq);
        } catch (LegoCheckedException e) {
            long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
            if (!VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = JSONObject.toBean(e.getMessage(), ActionResult.class);
                errorCode = Long.parseLong(actionResult.getBodyErr());
            }
            log.error("Cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(errorCode, "Cluster instance check fail.");
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Cluster instance check fail.");
        }
        return agentBaseDto;
    }

    private AgentBaseDto checkClusterInstanceByAgent(AppEnv appEnv, Application application) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        checkAppReq.setApplication(application);
        AgentBaseDto agentBaseDto;
        try {
            agentBaseDto = agentUnifiedService.check(application.getSubType(), appEnv.getEndpoint(), appEnv.getPort(),
                checkAppReq);
            if (!agentBaseDto.isAgentBaseDtoReturnSuccess()) {
                throw new LegoCheckedException(agentBaseDto.getErrorMessage());
            }
        } catch (LegoCheckedException e) {
            long errorCode = CommonErrorCode.AGENT_NETWORK_ERROR;
            if (!VerifyUtil.isEmpty(e.getMessage())) {
                ActionResult actionResult = JSONObject.toBean(e.getMessage(), ActionResult.class);
                errorCode = Long.parseLong(actionResult.getBodyErr());
            }
            log.error("Cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(errorCode, "Cluster instance check fail.");
        } catch (LegoUncheckedException | FeignException e) {
            log.error("Cluster instance check fail. name: {}", appEnv.getName());
            throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Cluster instance check fail.");
        }
        return agentBaseDto;
    }

    @Override
    public void setClusterInstanceNodeRole(ProtectedResource resource) {
        AppEnvResponse clusterInstanceInfo = queryClusterInstanceNodeRoleByAgent(resource);
        Map<String, List<NodeInfo>> appEnvMap = clusterInstanceInfo.getNodes()
            .stream()
            .collect(Collectors.groupingBy(NodeInfo::getEndpoint));
        resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .forEach(childNode -> buildClusterNodeRole(childNode, appEnvMap));
    }

    private AppEnvResponse queryClusterInstanceNodeRoleByAgent(ProtectedResource resource) {
        ProtectedResource subInstance = getSubInstance(resource);
        AppEnv appEnv = buildAppEnv(resource, subInstance);
        Application application = buildApplication(resource, subInstance);
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(appEnv);
        checkAppReq.setApplication(application);
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setEndpoint(appEnv.getEndpoint());
        environment.setPort(appEnv.getPort());
        return agentUnifiedService.getClusterInfo(application.getSubType(), environment, checkAppReq, true);
    }

    private void buildClusterNodeRole(ProtectedResource childNode, Map<String, List<NodeInfo>> appEnvMap) {
        childNode.getExtendInfo()
            .put(DatabaseConstants.ROLE, appEnvMap.get(childNode.getExtendInfoByKey(DatabaseConstants.SERVICE_IP))
                .get(IsmNumberConstant.ZERO)
                .getExtendInfo()
                .get(DatabaseConstants.ROLE));
    }

    @Override
    public void checkClusterInstanceIsRegistered(ProtectedResource resource) {
        int instanceCount = queryClusterInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ZERO) {
            log.error("This cluster instance is registered. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED,
                "This cluster instance is registered.");
        }
    }

    @Override
    public void checkClusterInstancePortIsChanged(ProtectedResource resource) {
        int instanceCount = queryClusterInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ONE) {
            log.error("This cluster instance port is changed. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_PORT_IS_MODIFIED,
                "This cluster instance port is changed.");
        }
    }

    private int queryClusterInstanceNums(ProtectedResource resource) {
        String instancePort = resource.getExtendInfo().get(DatabaseConstants.INSTANCE_PORT);
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.INSTANCE_PORT, instancePort);
        filter.put(DatabaseConstants.PARENT_UUID, resource.getParentUuid());
        return resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter).getTotalCount();
    }

    @Override
    public void checkSignalInstanceIsRegistered(ProtectedResource resource) {
        int instanceCount = querySignalInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ZERO) {
            log.error("This signal instance is registered. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED,
                "This signal instance is registered.");
        }
    }

    @Override
    public void checkSignalInstancePortIsChanged(ProtectedResource resource) {
        int instanceCount = querySignalInstanceNums(resource);
        if (instanceCount != IsmNumberConstant.ONE) {
            log.error("This signal instance port is changed. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_PORT_IS_MODIFIED,
                "This signal instance port is changed.");
        }
    }

    private int querySignalInstanceNums(ProtectedResource resource) {
        String instancePort = resource.getExtendInfo().get(DatabaseConstants.INSTANCE_PORT);
        Map<String, Object> filter = new HashMap<>();
        filter.put(DatabaseConstants.INSTANCE_PORT, instancePort);
        filter.put(DatabaseConstants.HOST_ID, resource.getExtendInfo().get(DatabaseConstants.HOST_ID));
        return resourceService.query(IsmNumberConstant.ZERO, IsmNumberConstant.ONE, filter).getTotalCount();
    }

    @Override
    public void healthCheckSingleInstance(ProtectedResource resource) {
        log.info("Enter execute single instance health check. id: {}, subType: {}", resource.getUuid(),
            resource.getSubType());
        try {
            healthCheckSingleInstanceByAgent(resource, resource.getEnvironment());
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.ONLINE.getStatus().toString());
            clearHealthCheckAlarm(resource);
        } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
            log.error("This single instance check connection fail.", e);
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.OFFLINE.getStatus().toString());
            generateHealthCheckAlarm(resource);
        } finally {
            if (Objects.nonNull(resource.getAuth())) {
                StringUtil.clean(resource.getAuth().getAuthPwd());
            }
        }
        updateResourceStatus(resource);
    }

    private void generateHealthCheckAlarm(ProtectedResource environment) {
        if (!canSendHealthAlarm(environment)) {
            log.info("Current node won't send alarm to DM. Env id : {}, env type: {}, current node role: {}",
                environment.getUuid(), environment.getType(), memberClusterService.getCurrentClusterRole());
            return;
        }
        commonAlarmService.generateAlarm(genHealthAlarm(environment));
    }

    private boolean canSendHealthAlarm(ProtectedResource environment) {
        // 资源类型为主机，可以在所有节点上报告警
        if (ResourceTypeEnum.HOST.equalsType(environment.getType())) {
            return true;
        }
        // 资源类型为其他，只允许在主节点上报告警
        return ClusterEnum.BackupRoleTypeEnum.PRIMARY.getBackupRoleType()
            .equals(memberClusterService.getCurrentClusterRole());
    }

    private void clearHealthCheckAlarm(ProtectedResource environment) {
        commonAlarmService.clearAlarm(genHealthAlarm(environment));
    }

    private LegoInternalAlarm genHealthAlarm(ProtectedResource environment) {
        LegoInternalAlarm legoInternalAlarm = new LegoInternalAlarm();
        legoInternalAlarm.setAlarmTime(System.currentTimeMillis() / IsmNumberConstant.THOUSAND);
        legoInternalAlarm.setAlarmSequence(IsmNumberConstant.TWO);
        legoInternalAlarm.setMoIp("127.0.0.1");
        legoInternalAlarm.setMoName("Resource");
        String subTypeOrder = null;
        legoInternalAlarm.setAlarmId(HEALTH_ALARM_CODE);
        subTypeOrder = String.valueOf(ResourceSubTypeEnum.getOrderBySubTypeSilent(environment.getSubType()));
        if (ResourceSubTypeEnum.VM_BACKUP_AGENT.getType().equals(environment.getSubType())) {
            legoInternalAlarm.setAlarmParam(new String[] {environment.getEndpoint()});
        } else {
            legoInternalAlarm.setAlarmParam(new String[] {environment.getUuid(), environment.getName(), subTypeOrder});
        }
        legoInternalAlarm.setSourceType(FaultEnum.AlarmResourceType.RESOURCE.getValue());
        legoInternalAlarm.setResourceId(environment.getUuid());
        return legoInternalAlarm;
    }

    private void healthCheckSingleInstanceByAgent(ProtectedResource resource, ProtectedEnvironment environment) {
        CheckAppReq checkAppReq = new CheckAppReq();
        checkAppReq.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        checkAppReq.setApplication(BeanTools.copy(resource, Application::new));
        AgentBaseDto checkResult = agentUnifiedService.check(resource.getSubType(), environment, checkAppReq);
        if (!checkResult.isAgentBaseDtoReturnSuccess()) {
            if (VerifyUtil.isEmpty(checkResult.getErrorCode())) {
                throw new LegoCheckedException("health check error");
            }
            throw new LegoCheckedException(Long.parseLong(checkResult.getErrorCode()), checkResult.getErrorMessage());
        }
    }

    private void updateResourceStatus(ProtectedResource resource) {
        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resource.getUuid());
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
    }

    @Override
    public void healthCheckClusterInstanceOfEnvironment(ProtectedEnvironment environment,
        ClusterInstanceOnlinePolicy policy) {
        log.info("Enter cluster instance of environment health check. id: {}, subType: {}", environment.getUuid(),
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
        resources.forEach(resource -> healthCheckClusterInstanceByAgent(resource, policy));
    }

    private void healthCheckClusterInstanceByAgent(ProtectedResource resource, ClusterInstanceOnlinePolicy policy) {
        ProtectedResource fullResource = resourceService.getResourceById(resource.getUuid())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This resource not exist."));
        List<ProtectedResource> childrenResource = fullResource.getDependencies().get(DatabaseConstants.CHILDREN);
        switch (policy) {
            case ALL_NODES_ONLINE:
                allNodesInstanceOnline(resource, childrenResource);
                break;
            case ANY_NODE_ONLINE:
                anyNodeInstanceOnline(resource, childrenResource);
                break;
            default:
                throw new IllegalArgumentException("This policy is not support.");
        }
        cleanResourceAuth(resource);
        cleanResourceAuth(fullResource);
        updateResourceStatus(resource);
    }

    private void allNodesInstanceOnline(ProtectedResource resource, List<ProtectedResource> childrenResource) {
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        for (ProtectedResource protectedResource : childrenResource) {
            try {
                healthCheckSingleInstanceByAgent(protectedResource, getEnvironmentOfInstance(protectedResource));
            } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
                log.error("This sub resource health check is fail.", e);
                resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                    LinkStatusEnum.OFFLINE.getStatus().toString());
                break;
            }
        }
    }

    private void anyNodeInstanceOnline(ProtectedResource resource, List<ProtectedResource> childrenResource) {
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.OFFLINE.getStatus().toString());
        for (ProtectedResource protectedResource : childrenResource) {
            try {
                healthCheckSingleInstanceByAgent(protectedResource, getEnvironmentOfInstance(protectedResource));
                resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                    LinkStatusEnum.ONLINE.getStatus().toString());
                break;
            } catch (LegoCheckedException | LegoUncheckedException | FeignException e) {
                log.error("This sub resource health check is fail.", e);
            }
        }
    }

    private ProtectedEnvironment getEnvironmentOfInstance(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "This environment not exist."));
    }

    private void cleanResourceAuth(ProtectedResource resource) {
        if (Objects.nonNull(resource.getAuth())) {
            StringUtil.clean(resource.getAuth().getAuthPwd());
        }
        if (Objects.nonNull(resource.getDependencies())) {
            List<ProtectedResource> childrenResource = resource.getDependencies().get(DatabaseConstants.CHILDREN);
            if (Objects.isNull(childrenResource)) {
                return;
            }
            childrenResource.forEach(subResource -> {
                if (Objects.nonNull(subResource.getAuth())) {
                    StringUtil.clean(subResource.getAuth().getAuthPwd());
                }
            });
        }
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected resource not exist. uuid: " + resourceId));
    }
}
