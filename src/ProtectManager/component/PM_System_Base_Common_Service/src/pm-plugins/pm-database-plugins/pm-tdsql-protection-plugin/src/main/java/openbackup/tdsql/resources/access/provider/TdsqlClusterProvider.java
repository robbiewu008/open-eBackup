package openbackup.tdsql.resources.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.ResourceCheckContextUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.cluster.SchedulerNode;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlGroup;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.service.TdsqlService;
import openbackup.tdsql.resources.access.util.TdsqlValidator;

import com.alibaba.fastjson.JSONObject;
import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections4.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.stereotype.Component;

import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.function.Function;
import java.util.stream.Collectors;

/**
 * 功能描述 TDSQL Cluster Provider
 *
 * @author z30047175
 * @since 2023-05-22
 */
@Slf4j
@Component
public class TdsqlClusterProvider extends DatabaseEnvironmentProvider {
    private final TdsqlService tdsqlService;

    private final ResourceService resourceService;

    /**
     * 构造方法
     *
     * @param providerManager provider manager
     * @param pluginConfigManager provider config manager
     * @param tdsqlService tdsqlService
     * @param resourceService resourceService
     */
    public TdsqlClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        TdsqlService tdsqlService, ResourceService resourceService) {
        super(providerManager, pluginConfigManager);
        this.tdsqlService = tdsqlService;
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.TDSQL_CLUSTER.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check Tdsql cluster: {}.", environment.getName());

        // 参数校验
        TdsqlValidator.checkTdsql(environment);
        checkOssNodeParam(environment);
        checkSchedulerNodeParam(environment);

        // 设置集群endpoint
        Set<String> ossUuids = new HashSet<>();
        Set<String> ossEndpoints = new HashSet<>();
        List<OssNode> ossNodes = tdsqlService.getOssNode(environment);
        ossNodes.forEach(ossNode -> {
            String ossEndpoint = tdsqlService.getEnvironmentById(ossNode.getParentUuid()).getEndpoint();
            ossUuids.add(ossNode.getParentUuid());
            ossEndpoints.add(ossEndpoint);
        });
        Set<String> schedulerUuids = new HashSet<>();
        Set<String> schedulerEndpoints = new HashSet<>();
        List<SchedulerNode> schedulerNodes = tdsqlService.getSchedulerNode(environment);
        schedulerNodes.forEach(schedulerNode -> {
            String schedulerEndpoint = tdsqlService.getEnvironmentById(schedulerNode.getParentUuid()).getEndpoint();
            schedulerUuids.add(schedulerNode.getParentUuid());
            schedulerEndpoints.add(schedulerEndpoint);
        });
        Set<String> endpoints = new HashSet<>();
        endpoints.addAll(ossEndpoints);
        endpoints.addAll(schedulerEndpoints);
        String clusterEndpoint = endpoints.stream().sorted().collect(Collectors.joining(TdsqlConstant.COMMA));
        environment.setEndpoint(clusterEndpoint);

        // 注册的时候校验集群唯一性
        if (environment.getUuid() == null) {
            // 生成环境资源唯一UUID,检查uuid是否已经存在
            String uuid = buildUniqueUuid(ossUuids, schedulerUuids);
            checkDuplicatedEnvironment(uuid);
            environment.setUuid(uuid);
        }

        // 连通性校验
        // 遍历OSS节点，触发check
        if (!checkOssConnect(environment)) {
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED,
                "Failed to connect to the target oss cluster.");
        }

        // 遍历Scheduler节点，触发check
        if (!checkSchedulerConnect(environment)) {
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED,
                "Failed to connect to the target scheduler cluster.");
        }

        if (environment.getUuid() == null) {
            environment.setUuid(UUIDGenerator.getUUID());
        }

        // 查询集群版本号，检验集群节点数
        String agentId = ossNodes.get(0).getParentUuid();
        AppEnvResponse appEnvResponse = tdsqlService.queryClusterInfo(environment, agentId);
        if (Objects.isNull(appEnvResponse) || MapUtils.isEmpty(appEnvResponse.getExtendInfo())) {
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED, "get cluster version failed.");
        }
        environment.setVersion(appEnvResponse.getExtendInfo().get("version"));
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    /**
     * 检查Oss节点是否为空/参数是否为空
     *
     * @param environment 受保护环境
     */
    private void checkOssNodeParam(ProtectedEnvironment environment) {
        log.info("start check ossNode");
        List<OssNode> ossNodes = Optional.ofNullable(tdsqlService.getOssNode(environment)).orElse(new ArrayList<>());
        ossNodes.forEach(ossNode -> {
            if (!StringUtils.isNotBlank(ossNode.getNodeType()) || !StringUtils.isNotBlank(ossNode.getParentUuid())
                || !StringUtils.isNotBlank(ossNode.getIp()) || !StringUtils.isNotBlank(ossNode.getPort())) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ossNode param is empty");
            }
        });
    }

    /**
     * 检查Scheduler节点是否为空/参数是否为空
     *
     * @param environment 受保护环境
     */
    private void checkSchedulerNodeParam(ProtectedEnvironment environment) {
        log.info("start check schedulerNode");
        List<SchedulerNode> schedulerNodes = Optional.ofNullable(tdsqlService.getSchedulerNode(environment))
            .orElse(new ArrayList<>());
        schedulerNodes.forEach(ossNode -> {
            if (!StringUtils.isNotBlank(ossNode.getNodeType()) || !StringUtils.isNotBlank(ossNode.getParentUuid())
                || !StringUtils.isNotBlank(ossNode.getIp())) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "schedulerNode param is empty");
            }
        });
    }

    /**
     * 根据管理节点agent主机的uuid和资源类型生成环境的唯一UUID
     *
     * @param ossUuids oss节点代理主机的uuid集合
     * @param schedulerUuids scheduler节点代理主机的uuid集合
     * @return 唯一UUID
     */
    private String buildUniqueUuid(Set<String> ossUuids, Set<String> schedulerUuids) {
        // 设置唯一UUID
        String uuidTag = ossUuids.stream().sorted().collect(Collectors.joining(TdsqlConstant.SEMICOLON))
            + TdsqlConstant.SEMICOLON + schedulerUuids.stream()
            .sorted()
            .collect(Collectors.joining(TdsqlConstant.SEMICOLON));

        // 生成uuid
        String envIdentity = ResourceSubTypeEnum.TDSQL_CLUSTER.getType() + uuidTag;
        String uuid = UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
        log.info("start to register new TDSQL environment, uuid: {}", uuid);
        return uuid;
    }

    /**
     * 检查环境是否存在
     *
     * @param uuid uuid
     */

    private void checkDuplicatedEnvironment(String uuid) {
        ProtectedResource resource = new ProtectedResource();
        try {
            resource = tdsqlService.getResourceById(uuid);
        } catch (LegoCheckedException exception) {
            log.info("cluster is unique");
        }
        if (resource.getUuid() != null) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "The cluster has been registered.");
        }
    }

    /**
     * 校验与oss节点集群的连通性
     *
     * @param environment environment
     * @return boolean
     */
    private boolean checkOssConnect(ProtectedEnvironment environment) {
        List<OssNode> ossNodes = Optional.ofNullable(tdsqlService.getOssNode(environment)).orElse(Lists.newArrayList());
        AtomicBoolean flag = new AtomicBoolean(true);
        ossNodes.forEach(ossNode -> {
            if (!tdsqlService.singleOssNodeConnectCheck(BeanTools.copy(ossNode, OssNode::new), environment)) {
                log.error("Failed to verify the ossNode, agent uuid is {}, ip is {}, port is {}",
                    ossNode.getParentUuid(), ossNode.getIp(), ossNode.getPort());
                flag.set(false);
            }
        });
        return flag.get();
    }

    /**
     * 校验与scheduler节点集群的连通性
     *
     * @param environment environment
     * @return boolean
     */
    private boolean checkSchedulerConnect(ProtectedEnvironment environment) {
        List<SchedulerNode> schedulerNodes = Optional.ofNullable(tdsqlService.getSchedulerNode(environment))
            .orElse(Lists.newArrayList());
        AtomicBoolean flag = new AtomicBoolean(true);
        schedulerNodes.forEach(schedulerNode -> {
            if (!tdsqlService.singleSchedulerNodeConnectCheck(BeanTools.copy(schedulerNode, SchedulerNode::new),
                environment)) {
                log.error("Failed to verify the schedulerNode, agent uuid is {}, ip is {}",
                    schedulerNode.getParentUuid(), schedulerNode.getIp());
                flag.set(false);
            }
        });
        return flag.get();
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        JSONObject jsonObject = JsonUtil.getJsonObjectFromStr(environmentConditions.getConditions());
        String type = Optional.ofNullable(jsonObject)
            .map(item -> item.get(TdsqlConstant.QUERY_TYPE_KEY))
            .map(Object::toString)
            .orElse(Strings.EMPTY);
        log.info("tdsql cluster browse type is {}", type);
        if (StringUtils.equals(type, TdsqlConstant.RESOURCE)) {
            // 查询集群主机和支持的机型信息
            return queryClusterHosts(environmentConditions, environment, type);
        }
        PageListResponse<ProtectedResource> detailPageList = tdsqlService.getBrowseResult(environmentConditions,
            environment);
        if (detailPageList.getRecords().size() == 0) {
            if (!environmentConditions.getConditions().contains("dataNodes")) {
                // 当返回结果为空，并且environmentConditions里Conditions中不含dataNodes字段，说明是注册时扫描实例节点返回值为空
                throw new LegoCheckedException(TdsqlConstant.NO_DATA_NODE_INFO,
                    "The data node information of the entered instance cannot be scanned or queried.");
            } else {
                // 如果有dataNodes字段，说明是在备份过程中调用的browse，由于赤兔上实例被删除导致备份报错
                throw new LegoCheckedException(TdsqlConstant.NO_INSTANCE_EXISTS,
                    "Check whether the registered instance exists on the TDSQL chitu management console.");
            }
        }
        checkBrowseResult(detailPageList.getRecords().get(0), environmentConditions);

        // 对于返回的结果进行处理
        return detailPageList;
    }

    private void checkBrowseResult(ProtectedResource resource,
        BrowseEnvironmentResourceConditions environmentConditions) {
        Map<String, String> extendInfo = resource.getExtendInfo();
        String subType = resource.getSubType();
        log.info("checkBrowseResult subType is {}", subType);
        if (StringUtils.equals(subType, TdsqlConstant.TDSQL_CLUSTERINSTACE)) {
            String clusterInstance = extendInfo.get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
            TdsqlInstance instanceFromEnv = JsonUtil.read(clusterInstance, TdsqlInstance.class);
            if (instanceFromEnv.getGroups().size() == 0) {
                if (!environmentConditions.getConditions().contains("dataNodes")) {
                    // 当返回结果为空，并且environmentConditions里Conditions中不含dataNodes字段，说明是注册时扫描实例节点返回值为空
                    throw new LegoCheckedException(TdsqlConstant.NO_DATA_NODE_INFO,
                        "The data node information of the entered instance cannot be scanned or queried");
                } else {
                    // 如果有dataNodes字段，说明是在备份过程中调用的browse，由于赤兔上实例被删除导致备份报错
                    throw new LegoCheckedException(TdsqlConstant.NO_INSTANCE_EXISTS,
                        "Check whether the registered instance exists on the TDSQL chitu management console");
                }
            }
        } else if (StringUtils.equals(subType, TdsqlConstant.TDSQL_CLUSTER_GROUP)) {
            TdsqlGroup tdsqlGroup = JsonUtil.read(extendInfo.get(TdsqlConstant.CLUSTER_GROUP_INFO), TdsqlGroup.class);
            if (tdsqlGroup.getGroup() == null || tdsqlGroup.getGroup().getSetIds().size() == 0
                || tdsqlGroup.getGroup().getDataNodes().size() == 0) {
                throw new LegoCheckedException(TdsqlConstant.NO_DATA_NODE_INFO,
                    "The data node information of the entered group cannot be scanned or queried");
            }
        } else {
            throw new LegoCheckedException(TdsqlConstant.NO_DATA_NODE_INFO, "The subType is invalid");
        }
    }

    private PageListResponse<ProtectedResource> queryClusterHosts(
        BrowseEnvironmentResourceConditions environmentConditions, ProtectedEnvironment environment, String queryType) {
        log.info("begin query machine from cluster. name: {}, uuid: {}", environment.getName(), environment.getUuid());
        PageListResponse<ProtectedResource> detailPageList = tdsqlService.getClusterHosts(environmentConditions,
            environment, queryType);
        log.info("success query machine size is {}", detailPageList.getRecords().size());
        return detailPageList;
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        log.info("start check tdsql cluster health. cluster name: {}, uuid: {}.", environment.getName(),
            environment.getUuid());
        // 通过OBClient检查集群的状态
        ResourceCheckContext context = providerManager.findProvider(ResourceConnectionCheckProvider.class, environment)
            .tryCheckConnection(environment);

        boolean isSuccess = ResourceCheckContextUtil.isSuccess(context.getActionResults());

        // 检查集群健康
        if (isSuccess) {
            // 集群健康行检查的时候也检查下面子实例的健康性,（集群状态OFFLINE， 则所有租户集都是OFFLINE，无需检查）
            healthCheckAllInstance(environment);
            log.info("end check tdsql cluster health. cluster name: {}, uuid: {}, and result is ONLINE.",
                environment.getName(), environment.getUuid());
            return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            log.info("end check tdsql cluster health. cluster name: {}, uuid: {}, and result is OFFLINE.",
                environment.getName(), environment.getUuid());
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
    }

    /**
     * 检查所有实例的状态
     *
     * @param environment environment
     */
    private void healthCheckAllInstance(ProtectedEnvironment environment) {
        log.info("start healthCheck all instance of cluster[{}]", environment.getUuid());
        // 检查非分布式实例状态
        List<ProtectedResource> instances = Optional.ofNullable(
            tdsqlService.getChildren(environment.getUuid(), ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType()))
            .orElseGet(ArrayList::new);
        instances.forEach(resource -> healthCheckInstance(resource, environment));
        // 检查分布式实例状态
        List<ProtectedResource> clusterGroup = Optional.ofNullable(
            tdsqlService.getChildren(environment.getUuid(), ResourceSubTypeEnum.TDSQL_CLUSTERGROUP.getType()))
            .orElseGet(ArrayList::new);
        clusterGroup.forEach(resource -> healthCheckClusterGroup(resource, environment));
        log.info("end healthCheck all instance of cluster[{}]", environment.getUuid());
    }

    /**
     * 分布式实例健康检查
     *
     * @param resource resource
     * @param clusterEnv clusterEnv
     */
    private void healthCheckClusterGroup(ProtectedResource resource, ProtectedEnvironment clusterEnv) {
        log.info("start check tdsql cluster group[{}] of cluster[{}] link status", resource.getUuid(),
            clusterEnv.getUuid());
        String clusterGroupInfo = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO);
        TdsqlGroup tdsqlGroup = JsonUtil.read(clusterGroupInfo, TdsqlGroup.class);
        ProtectedEnvironment environment = BeanTools.copy(resource, ProtectedEnvironment::new);
        String status;
        try {
            if (checkClusterGroup(tdsqlGroup, environment)) {
                status = LinkStatusEnum.ONLINE.getStatus().toString();
            } else {
                log.error("This cluster group {} check connection fail.", resource.getName());
                status = LinkStatusEnum.OFFLINE.getStatus().toString();
            }
        } catch (LegoCheckedException exception) {
            log.error("This cluster group {} check connection error.", resource.getName());
            status = LinkStatusEnum.OFFLINE.getStatus().toString();
        } finally {
            if (Objects.nonNull(resource.getAuth())) {
                StringUtil.clean(resource.getAuth().getAuthPwd());
            }
        }

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resource.getUuid());
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
        updateResource.setExtendInfoByKey(TdsqlConstant.CLUSTER_GROUP_INFO, JsonUtil.json(tdsqlGroup));
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));
        log.info("end check tdsql cluster group[{}] of cluster[{}] link status", resource.getUuid(),
            clusterEnv.getUuid());
    }

    private boolean checkClusterGroup(TdsqlGroup tdsqlGroup, ProtectedEnvironment environment) {
        if (!tdsqlService.checkGroupInfo(tdsqlGroup, environment)) {
            log.error("checkGroupInfo fail.");
            return false;
        }
        for (DataNode dataNode : tdsqlGroup.getGroup().getDataNodes()) {
            if (!tdsqlService.checkDataNodeIsMatchAgent(dataNode, environment)) {
                log.error("checkDataNodeIsMatchAgent fail.");
                return false;
            }
        }
        return true;
    }

    /**
     * 非分布式实例健康检查
     *
     * @param resource resource
     * @param clusterEnv clusterEnv
     */
    private void healthCheckInstance(ProtectedResource resource, ProtectedEnvironment clusterEnv) {
        log.info("start check tdsql instance[{}] of cluster[{}] link status", resource.getUuid(), clusterEnv.getUuid());
        String clusterInstanceInfo = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);

        // 查询实例的数据节点
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setResourceType(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());
        conditions.setConditions(JsonUtil.json(tdsqlInstance));
        PageListResponse<ProtectedResource> detailPageList = tdsqlService.getBrowseResult(conditions, clusterEnv);
        String status;
        try {
            if (detailPageList.getRecords().size() == 0) {
                log.error("This single instance {} check connection size 0.", resource.getName());
                status = LinkStatusEnum.OFFLINE.getStatus().toString();
            } else {
                String clusterInstance = detailPageList.getRecords()
                    .get(0)
                    .getExtendInfo()
                    .get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
                TdsqlInstance instanceFromEnv = JsonUtil.read(clusterInstance, TdsqlInstance.class);
                dataNodeHealthCheck(resource, tdsqlInstance, instanceFromEnv);
                status = LinkStatusEnum.ONLINE.getStatus().toString();
            }
        } catch (LegoCheckedException exception) {
            log.error("This single instance {} check connection fail.", resource.getName());
            status = LinkStatusEnum.OFFLINE.getStatus().toString();
        } finally {
            if (Objects.nonNull(resource.getAuth())) {
                StringUtil.clean(resource.getAuth().getAuthPwd());
            }
        }

        ProtectedResource updateResource = new ProtectedResource();
        updateResource.setUuid(resource.getUuid());
        updateResource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, status);
        updateResource.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(tdsqlInstance));
        resourceService.updateSourceDirectly(Collections.singletonList(updateResource));

        log.info("end check tdsql instance[{}] of cluster[{}] link status", resource.getUuid(), clusterEnv.getUuid());
    }

    private void dataNodeHealthCheck(ProtectedResource resource, TdsqlInstance tdsqlInstance,
        TdsqlInstance instanceFromEnv) {
        if (instanceFromEnv.getGroups().size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The TDSQL instance healthCheck failed.");
        }
        Map<String, DataNode> dataNodesFromEnv = instanceFromEnv.getGroups()
            .get(0)
            .getDataNodes()
            .stream()
            .collect(Collectors.toMap(dataNode -> dataNode.getIp() + ":" + dataNode.getPort(), Function.identity()));

        // 每一个分片上必须所有节点都可以连通成功
        AtomicBoolean flag = new AtomicBoolean(true);
        for (DataNode dataNode : tdsqlInstance.getGroups().get(0).getDataNodes()) {
            log.info("start check data node,ip={},port={}", dataNode.getIp(), dataNode.getPort());
            String key = dataNode.getIp() + ":" + dataNode.getPort();
            if (dataNodesFromEnv.get(key) == null) {
                log.warn("data node {} not exists in the cluster instance {}", key, tdsqlInstance.getName());

                // 设置数据节点状态
                dataNode.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
                flag.set(false);
                continue;
            }

            if (!tdsqlService.singleDataNodeHealthCheck(dataNode,
                BeanTools.copy(resource, ProtectedEnvironment::new))) {
                log.warn("check data node connection failed,ip is {}, parentUuid is {}", dataNode.getIp(),
                    dataNode.getParentUuid());
                dataNode.setLinkStatus(LinkStatusEnum.OFFLINE.getStatus().toString());
                flag.set(false);
            } else {
                log.warn("check data node connection success,ip is {}, parentUuid is {}", dataNode.getIp(),
                    dataNode.getParentUuid());
                dataNode.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
            }

            // 设置主备状态
            dataNode.setIsMaster(dataNodesFromEnv.get(key).getIsMaster());
        }

        if (!flag.get()) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The TDSQL instance computeNode healthCheck failed.");
        }
    }
}
