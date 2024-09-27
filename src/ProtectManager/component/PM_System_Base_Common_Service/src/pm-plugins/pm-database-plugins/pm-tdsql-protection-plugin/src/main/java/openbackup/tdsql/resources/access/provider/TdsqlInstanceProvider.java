package openbackup.tdsql.resources.access.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.instance.DataNode;
import openbackup.tdsql.resources.access.dto.instance.Group;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.interceptor.TdsqlTaskCheck;
import openbackup.tdsql.resources.access.service.TdsqlService;
import openbackup.tdsql.resources.access.util.TdsqlInstanceValidator;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述
 *
 * @author z30047175
 * @since 2023-05-22
 */
@Component
@Slf4j
public class TdsqlInstanceProvider implements ResourceProvider {
    private final TdsqlService tdsqlService;

    private final TdsqlTaskCheck tdsqlTaskCheck;

    /**
     * 构造
     *
     * @param tdsqlService tdsqlService
     * @param tdsqlTaskCheck tdsqlTashCheck
     */
    public TdsqlInstanceProvider(TdsqlService tdsqlService, TdsqlTaskCheck tdsqlTaskCheck) {
        this.tdsqlService = tdsqlService;
        this.tdsqlTaskCheck = tdsqlTaskCheck;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType().equals(object.getSubType());
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(resource.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(resource.getName());
        TdsqlInstanceValidator.checkTdsqlInstance(resource);
        checkNodeParam(resource);
        checkLinkStatus(resource.getParentUuid());

        // 注册的时候校验实例唯一性
        String clusterInstanceInfo = resource.getExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        TdsqlInstance tdsqlInstance = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class);
        String instanceId = tdsqlInstance.getId();
        if (resource.getExtendInfoByKey(TdsqlConstant.INSTANCE_ID) == null) {
            List<ProtectedResource> instances = tdsqlService.getChildren(resource.getParentUuid(),
                ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());
            if (instances.size() > 0) {
                List<String> collect = instances.stream()
                    .map(instance -> instance.getExtendInfoByKey(TdsqlConstant.INSTANCE_ID))
                    .collect(Collectors.toList());
                if (collect.contains(instanceId)) {
                    throw new LegoCheckedException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED,
                        "The instance has been registered！");
                }
            }
        }
        log.info("TDSQL Instance start to check beforeCreate.resourceName:{}", resource.getName());

        // 校验输入的实例信息和查询的OSS查询的实例信息是否一致
        checkClusterInstanceDifference(tdsqlInstance, resource);

        // 遍历dataNode节点
        dataNodeCheck(BeanTools.copy(resource, ProtectedEnvironment::new));
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        resource.setExtendInfoByKey(TdsqlConstant.INSTANCE_ID, instanceId);
        setClusterInstancePath(resource);
    }

    private void checkClusterInstanceDifference(TdsqlInstance tdsqlInstance, ProtectedResource resource) {
        // 查询实例的数据节点
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setResourceType(ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType());
        conditions.setConditions(JsonUtil.json(tdsqlInstance));
        ProtectedEnvironment clusterEnv = tdsqlService.getClusterEnv(resource);
        PageListResponse<ProtectedResource> detailPageList =
            tdsqlService.getBrowseResult(conditions, clusterEnv);
        if (detailPageList.getRecords().size() == 0) {
            throw new LegoCheckedException(TdsqlConstant.NO_DATA_NODE_INFO, "browse compute node zero");
        }

        // 和下发的请求中节点数做对比
        String clusterInstance =
            detailPageList.getRecords().get(0).getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        TdsqlInstance instanceFromEnv = JsonUtil.read(clusterInstance, TdsqlInstance.class);
        if (instanceFromEnv.getGroups().size() == 0) {
            throw new LegoCheckedException(TdsqlConstant.NO_DATA_NODE_INFO, "browse compute node zero");
        }
        int dataNodeNum = getDataNodeNumber(BeanTools.copy(resource, ProtectedEnvironment::new));
        if (!(dataNodeNum == instanceFromEnv.getGroups().get(0).getDataNodes().size())) {
            throw new LegoCheckedException(TdsqlConstant.MISSING_DATA_NODE,
                "Instance dataNodes number does not match");
        }

        // 校验单个数据节点：ip port 调用agent校验服务状态
        if (tdsqlTaskCheck.checkDifference(tdsqlInstance, instanceFromEnv)) {
            log.error("create instance check dataNode difference failed");
            throw new LegoCheckedException(TdsqlConstant.MISSING_DATA_NODE,
                "The number of backup tasks is inconsistent with that in the production environment.");
        }
    }

    @Override
    public void beforeUpdate(ProtectedResource resource) {
        beforeCreate(resource);
    }

    /**
     * 对注册实例时的参数进行校验
     *
     * @param resource resource
     */
    private void checkNodeParam(ProtectedResource resource) {
        log.info("start check dataNode");
        List<DataNode> dataNodes =
            Optional.ofNullable(tdsqlService.getInstanceDataNodes(BeanTools.copy(resource, ProtectedEnvironment::new)))
                .orElseGet(ArrayList::new);
        dataNodes.forEach(dataNode -> {
            checkDataNodePart1(dataNode);
            checkDataNodePart2(dataNode);
            checkDataNodePart3(dataNode);
        });
    }

    /**
     * 对注册实例时的参数进行校验 第一部分
     *
     * @param dataNode dataNode
     */
    private void checkDataNodePart1(DataNode dataNode) {
        if (!StringUtils.isNotBlank(dataNode.getIp()) || !StringUtils.isNotBlank(dataNode.getPort())
            || !StringUtils.isNotBlank(dataNode.getDefaultsFile())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TDSQL DataNode param is empty");
        }
    }

    /**
     * 对注册实例时的参数进行校验 第二部分
     *
     * @param dataNode dataNode
     */
    private void checkDataNodePart2(DataNode dataNode) {
        if (!StringUtils.isNotBlank(dataNode.getSocket()) || !StringUtils.isNotBlank(dataNode.getIsMaster())
            || !StringUtils.isNotBlank(dataNode.getPriority())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TDSQL DataNode param is empty");
        }
    }

    /**
     * 对注册实例时的参数进行校验 第三部分
     *
     * @param dataNode dataNode
     */
    private void checkDataNodePart3(DataNode dataNode) {
        if (!StringUtils.isNotBlank(dataNode.getNodeType()) || !StringUtils.isNotBlank(dataNode.getParentUuid())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "TDSQL DataNode param is empty.");
        }
    }

    /**
     * 遍历dataNode，调用check
     *
     * @param environment environment
     */
    private void dataNodeCheck(ProtectedEnvironment environment) {
        // 遍历dataNode节点
        String clusterInstanceInfo = environment.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        List<Group> groups = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class).getGroups();
        for (Group group : groups) {
            for (DataNode dataNode : group.getDataNodes()) {
                if (!tdsqlService.singleDataNodeConnectCheck(dataNode, environment)) {
                    throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                        "The TDSQL instance dataNode query failed.");
                }
            }
        }
    }

    /**
     * 校验是否是同一集群下的管理节点
     *
     * @param environment environment
     * @return int
     */
    private int getDataNodeNumber(ProtectedEnvironment environment) {
        String clusterInstanceInfo = environment.getExtendInfo().get(TdsqlConstant.CLUSTER_INSTANCE_INFO);
        List<Group> groups = JsonUtil.read(clusterInstanceInfo, TdsqlInstance.class).getGroups();
        int[] dataNodesNum = new int[groups.size()];
        groups.forEach(group -> {
            dataNodesNum[0] = group.getDataNodes().size();
        });
        return dataNodesNum[0];
    }

    /**
     * 设置集群实例的path，保证副本复制不会出错
     *
     * @param resource 集群实例资源
     */
    private void setClusterInstancePath(ProtectedResource resource) {
        List<String> managerNodeEndpoints = tdsqlService.getEnvironmentById(resource.getParentUuid())
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(protectedResource -> tdsqlService.getEnvironmentById(protectedResource.getUuid()).getEndpoint())
            .collect(Collectors.toList());
        List<String> instanceNodeEndpoints = resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(protectedResource -> tdsqlService.getEnvironmentById(protectedResource.getUuid()).getEndpoint())
            .collect(Collectors.toList());
        managerNodeEndpoints.addAll(instanceNodeEndpoints);
        String path = managerNodeEndpoints.stream().distinct().sorted().collect(Collectors.joining(","));
        resource.setPath(path);
    }

    /**
     * 校验应用集群是否在线;
     *
     * @param uuid 集群uuid
     */
    public void checkLinkStatus(String uuid) {
        ProtectedEnvironment clusterEnvironment = tdsqlService.getEnvironmentById(uuid);
        if (EnvironmentLinkStatusHelper.isOnlineAdaptMultiCluster(clusterEnvironment)) {
            return;
        }
        log.error("tdsql instance Cluster {} Not Online", clusterEnvironment.getName());
        throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
            "tdsql instance cluster " + clusterEnvironment.getName() + " not online");
    }
}
