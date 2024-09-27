package openbackup.goldendb.protection.access.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.dto.instance.Group;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.goldendb.protection.access.util.GoldenDbValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.StringUtil;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.lang.reflect.Field;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicReference;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-06
 */
@Slf4j
@Component
public class GoldenDbClusterProvider extends DatabaseEnvironmentProvider {
    private final AgentUnifiedService agentUnifiedService;

    private final GoldenDbService goldenDbService;

    /**
     * 构造方法
     *
     * @param providerManager provider管理器
     * @param pluginConfigManager 插件配置管理器
     * @param agentUnifiedService agentUnifiedService
     * @param goldenDbService goldenDbService
     */
    public GoldenDbClusterProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        AgentUnifiedService agentUnifiedService, GoldenDbService goldenDbService) {
        super(providerManager, pluginConfigManager);
        this.agentUnifiedService = agentUnifiedService;
        this.goldenDbService = goldenDbService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType().equals(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("start to check GoldenDB cluster: {}.", environment.getName());

        // 参数校验
        GoldenDbValidator.checkGoldenDb(environment);
        checkManagerNodeParam(environment);

        // 设置集群endpoint
        Set<String> agentUuids = new HashSet<>();
        Set<String> agentEndpoints = new HashSet<>();
        List<Node> manageDbNodes = goldenDbService.getManageDbNode(environment);
        manageDbNodes.forEach(node -> {
            String endpoint = goldenDbService.getEnvironmentById(node.getParentUuid()).getEndpoint();
            agentUuids.add(node.getParentUuid());
            agentEndpoints.add(endpoint);
        });
        String clusterEndpoint = agentEndpoints.stream().sorted().collect(Collectors.joining(","));
        environment.setEndpoint(clusterEndpoint);

        // 注册的时候校验集群唯一性
        if (environment.getUuid() == null) {
            // 生成环境资源唯一UUID,检查uuid是否已经存在
            String uuid = buildUniqueUuid(agentUuids);
            checkDuplicatedEnvironment(uuid);
            environment.setUuid(uuid);
        }

        // 连通性校验
        if (!checkConnect(environment)) {
            throw new LegoCheckedException(CommonErrorCode.TARGET_CLUSTER_ADD_FAILED,
                "Failed to connect to the target cluster.");
        }
        if (environment.getUuid() == null) {
            environment.setUuid(UUIDGenerator.getUUID());
        }

        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());

        // 检查注册的管理节点是否属于同一个集群（browse内部校验）
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setResourceType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
        String version = Optional
            .ofNullable(
                browse(environment, conditions).getRecords().get(0).getExtendInfo().get(GoldenDbConstant.VERSION))
            .orElse("0.0.0");
        if (version.compareTo(GoldenDbConstant.LOWEST_VERSION) < 0) {
            throw new LegoCheckedException(CommonErrorCode.VERSION_ERROR,
                "The version is too early to support backup.");
        }
        environment.setVersion(version);

        // 更新的时候检查实例状态
        healthCheckAllInstance(environment);
    }

    private void checkManagerNodeParam(ProtectedEnvironment environment) {
        log.info("start check manageNode");
        List<Node> manageDbNodes =
            Optional.ofNullable(goldenDbService.getManageDbNode(environment)).orElse(new ArrayList<>());
        manageDbNodes.forEach(manageDbNode -> {
            if (!StringUtils.isNotBlank(manageDbNode.getNodeType()) || !StringUtils.isNotBlank(manageDbNode.getOsUser())
                || !StringUtils.isNotBlank(manageDbNode.getParentUuid())) {
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "ManageNode param is empty");
            }
        });
    }

    /**
     * 根据管理节点agent主机的uuid和资源类型生成环境的唯一UUID
     *
     * @param agentsUuids agent主机的uuid集合
     * @return 唯一UUID
     */
    private String buildUniqueUuid(Set<String> agentsUuids) {
        // 设置唯一UUID
        String uuidTag = agentsUuids.stream().sorted().collect(Collectors.joining(";"));

        /**/
        String envIdentity = ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType() + uuidTag;
        String uuid = UUID.nameUUIDFromBytes(envIdentity.getBytes(Charset.defaultCharset())).toString();
        log.info("start to register new GoldenDB environment, uuid: {}", uuid);
        return uuid;
    }

    private void checkDuplicatedEnvironment(String uuid) {
        ProtectedResource resource = new ProtectedResource();
        try {
            resource = goldenDbService.getResourceById(uuid);
        } catch (LegoCheckedException exception) {
            log.info("cluster is unique");
        }
        if (resource.getUuid() != null) {
            throw new LegoCheckedException(CommonErrorCode.PROTECTED_ENV_REPEATED, "The cluster has been registered.");
        }
    }

    /**
     * 校验与集群的连通性
     *
     * @param environment environment
     * @return boolean
     */
    private boolean checkConnect(ProtectedEnvironment environment) {
        List<Node> manageDbNodes =
            Optional.ofNullable(goldenDbService.getManageDbNode(environment)).orElse(Lists.newArrayList());
        AtomicBoolean flag = new AtomicBoolean(true);
        manageDbNodes.stream().forEach(node -> {
            if (!goldenDbService.singleConnectCheck(BeanTools.copy(node, MysqlNode::new), environment)) {
                log.error("Failed to verify the manageDbNode,osUser is {}", node.getOsUser());
                flag.set(false);
            }
        });
        return flag.get();
    }

    // 看最后逻辑的修改
    @Override
    public void validate(ProtectedEnvironment environment) {
        // 管理节点至少一台可用
        AtomicBoolean result = new AtomicBoolean(false);
        List<Node> manageDbNodes =
            Optional.ofNullable(goldenDbService.getManageDbNode(environment)).orElse(Lists.newArrayList());
        manageDbNodes.stream().forEach(node -> {
            if (goldenDbService.singleHealthCheck(BeanTools.copy(node, MysqlNode::new), environment)) {
                log.info("success to verify the manageDbNode,osUser is {}", node.getOsUser());
                result.set(true);
            }
        });
        if (!result.get()) {
            // 集群检查失败，其下面的所有子实例也会对应离线
            setInstanceStatue(environment);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The GoldenDB instance manageNode failed.");
        }

        // 集群健康行检查的时候也检查下面子实例的健康性
        healthCheckAllInstance(environment);
    }

    /**
     * 检查所有实例的状态
     *
     * @param environment environment
     */
    private void healthCheckAllInstance(ProtectedEnvironment environment) {
        List<ProtectedResource> instances =
            Optional.ofNullable(goldenDbService.getChildren(environment.getUuid())).orElseGet(ArrayList::new);
        instances.forEach(instance -> {
            healthCheckInstance(instance);
        });
    }

    private void healthCheckInstance(ProtectedResource resource) {
        try {
            log.info("goldenDB instance is healthChecking");
            computeNodeHealthCheck(BeanTools.copy(resource, ProtectedEnvironment::new));
            gtmNodeHealthCheck(BeanTools.copy(resource, ProtectedEnvironment::new));
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.ONLINE.getStatus().toString());
        } catch (LegoCheckedException exception) {
            log.error("This single instance {} check connection fail.", resource.getName());
            resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY,
                LinkStatusEnum.OFFLINE.getStatus().toString());
        } finally {
            if (Objects.nonNull(resource.getAuth())) {
                StringUtil.clean(resource.getAuth().getAuthPwd());
            }
        }
        goldenDbService.updateResourceLinkStatus(resource.getUuid(),
            resource.getExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY));
    }

    private void computeNodeHealthCheck(ProtectedEnvironment environment) {
        String clusterInfo = environment.getExtendInfo().get(GoldenDbConstant.CLUSTER_INFO);
        List<Group> groups = JsonUtil.read(clusterInfo, GoldenInstance.class).getGroup();
        groups.stream().forEach(group -> {
            // 每一个分片上至少有一个节点可以连通成功
            AtomicBoolean flag = new AtomicBoolean(false);
            group.getMysqlNodes().stream().forEach(mysqlNode -> {
                if (goldenDbService.singleHealthCheck(mysqlNode, environment)) {
                    flag.set(true);
                }
            });
            if (!flag.get()) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                    "The GoldenDB instance computeNode healthCheck failed.");
            }
        });
    }

    private void gtmNodeHealthCheck(ProtectedEnvironment environment) {
        AtomicBoolean result = new AtomicBoolean(false);
        List<Gtm> gtmNodes = Optional.ofNullable(goldenDbService.getGtmNode(environment)).orElse(Lists.newArrayList());
        if (gtmNodes.size() == 0) {
            log.info("gtm size is zero");
            result.set(true);
        }
        gtmNodes.forEach(gtm -> {
            if (goldenDbService.singleHealthCheck(BeanTools.copy(gtm, MysqlNode::new), environment)) {
                log.info("success to verify the gtmNode,osUser is {}", gtm.getOsUser());
                result.set(true);
            }
        });
        if (!result.get()) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                "The GoldenDB instance gtmNode query failed");
        }
    }

    private void setInstanceStatue(ProtectedEnvironment environment) {
        // 集群健康检查失败，所有的实例都设置为离线
        List<ProtectedResource> children =
            Optional.ofNullable(goldenDbService.getChildren(environment.getUuid())).orElseGet(ArrayList::new);
        children.forEach(it -> {
            goldenDbService.updateResourceLinkStatus(it.getUuid(), LinkStatusEnum.OFFLINE.getStatus().toString());
        });
    }

    @Override
    public PageListResponse<ProtectedResource> browse(ProtectedEnvironment environment,
        BrowseEnvironmentResourceConditions environmentConditions) {
        List<Node> manageDbNodes = goldenDbService.getManageDbNode(environment);
        PageListResponse<ProtectedResource> detailPageList = getBrowseResult(environmentConditions, manageDbNodes);
        if (detailPageList.getRecords().size() == 0) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "browse compute node zero");
        }

        // 对于返回的结果进行处理
        return detailPageList;
    }

    /**
     * 校验是否是同一集群下的管理节点
     *
     * @param environmentConditions environmentConditions
     * @param manageDbNodes manageDbNodes
     * @return PageListResponse
     */
    private PageListResponse<ProtectedResource>
        getBrowseResult(BrowseEnvironmentResourceConditions environmentConditions, List<Node> manageDbNodes) {
        List<String> uniqueIds = new ArrayList<>();
        AtomicReference<PageListResponse<ProtectedResource>> result = new AtomicReference<>(new PageListResponse<>());
        manageDbNodes.forEach(node -> {
            ProtectedEnvironment agentEnvironment = goldenDbService.getEnvironmentById(node.getParentUuid());
            log.info("start to browse goldenDB,ip is {},resource type is {}, agentPort is {}",
                agentEnvironment.getEndpoint(), environmentConditions.getResourceType(), agentEnvironment.getPort());
            result.set(agentUnifiedService.getDetailPageListNoRetry(environmentConditions.getResourceType(),
                agentEnvironment.getEndpoint(), agentEnvironment.getPort(), getListResourceReq(agentEnvironment,
                    environmentConditions.getPageNo(), environmentConditions.getPageSize(), node), false));
            List<ProtectedResource> records = result.get().getRecords();
            if (records.size() == 0) {
                throw new LegoCheckedException(
                    GoldenDbConstant.NODE_TYPE_MISMATCH, new String[] {GoldenDbConstant.MANAGE_TYPE,
                        agentEnvironment.getEndpoint(), node.getOsUser(), node.getNodeType()},
                    "browse compute node zero");
            }
            String json = JsonUtil.json(records);
            String uniqueId = UUID.nameUUIDFromBytes(json.getBytes(Charset.defaultCharset())).toString();
            uniqueIds.add(uniqueId);
        });
        if (uniqueIds.stream().distinct().collect(Collectors.toList()).size() > 1) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_BELONG_TO_SAME_CLUSTER,
                "The selected management nodes do not belong to the same cluster.");
        }
        return result.get();
    }

    private ListResourceV2Req getListResourceReq(ProtectedEnvironment env, int pageNo, int pageSize, Node node) {
        Application application = BeanTools.copy(env, Application::new);
        application.setType(ResourceTypeEnum.DATABASE.getType());
        application.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
        HashMap<String, String> extendInfo = new HashMap<>();
        addFieds(extendInfo, node);
        application.setExtendInfo(extendInfo);
        ListResourceV2Req req = new ListResourceV2Req();
        req.setPageNo(pageNo);
        req.setPageSize(pageSize);
        req.setAppEnv(BeanTools.copy(env, AppEnv::new));
        req.setApplications(Lists.newArrayList(application));
        return req;
    }

    private void addFieds(HashMap<String, String> extendInfo, Node node) {
        // 将mysql对象中的所有属性添加到extentInfo
        for (Field field : node.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            try {
                if (field.get(node) != null) {
                    extendInfo.put(field.getName(), field.get(node).toString());
                }
            } catch (IllegalAccessException e) {
                log.error("get param fail");
            }
        }
    }
}
