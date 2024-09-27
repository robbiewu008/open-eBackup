/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.goldendb.protection.access.provider;

import openbackup.access.framework.resource.util.EnvironmentParamCheckUtil;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceFeature;
import openbackup.data.protection.access.provider.sdk.resource.ResourceProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.dto.instance.Group;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.goldendb.protection.access.util.GoldenDbInstanceValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.List;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述 goldenDb实例处理
 *
 * @author s30036254
 * @since 2023-02-13
 */
@Component
@Slf4j
public class GoldenDbInstanceProvider implements ResourceProvider {
    private final GoldenDbService goldenDbService;

    /**
     * DatabaseResourceProvider 构造器注入
     *
     * @param goldenDbService goldenDbService
     */
    public GoldenDbInstanceProvider(GoldenDbService goldenDbService) {
        this.goldenDbService = goldenDbService;
    }

    /**
     * detect object applicable
     *
     * @param object object
     * @return detect result
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType().equals(object.getSubType());
    }

    /**
     * 检查受保护资源，创建逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeCreate(ProtectedResource resource) {
        EnvironmentParamCheckUtil.checkEnvironmentNameEmpty(resource.getName());
        EnvironmentParamCheckUtil.checkEnvironmentNamePattern(resource.getName());
        GoldenDbInstanceValidator.checkGoldenDbInstance(resource);
        checkNodeParam(resource);

        // 注册的时候校验实例唯一性
        String clusterInfo = resource.getExtendInfoByKey(GoldenDbConstant.CLUSTER_INFO);
        String instanceId = JsonUtil.read(clusterInfo, GoldenInstance.class).getId();
        if (resource.getExtendInfoByKey(GoldenDbConstant.INSTANCE_ID) == null) {
            List<ProtectedResource> instances = goldenDbService.getChildren(resource.getParentUuid());
            if (instances.size() > 0) {
                List<String> collect = instances.stream()
                    .map(instance -> instance.getExtendInfoByKey(GoldenDbConstant.INSTANCE_ID))
                    .collect(Collectors.toList());
                if (collect.contains(instanceId)) {
                    throw new LegoCheckedException(CommonErrorCode.DB_INSTANCE_HAS_REGISTERED,
                        "The instance has been registered.");
                }
            }
        }
        log.info("GoldenDbInstance start to check beforeCreate.resourceName:{}", resource.getName());
        checkNodeMatch(resource);
        checkGtmNodeMatch(resource);
        managerNodeCheck(BeanTools.copy(resource, ProtectedEnvironment::new));
        computeNodeCheck(BeanTools.copy(resource, ProtectedEnvironment::new));
        gtmNodeCheck(BeanTools.copy(resource, ProtectedEnvironment::new));
        resource.setExtendInfoByKey(DatabaseConstants.LINK_STATUS_KEY, LinkStatusEnum.ONLINE.getStatus().toString());
        resource.setExtendInfoByKey(GoldenDbConstant.INSTANCE_ID, instanceId);
        setClusterInstancePath(resource);
    }

    /**
     * 对注册实例时的参数进行校验
     *
     * @param resource resource
     */
    private void checkNodeParam(ProtectedResource resource) {
        log.info("start check computeNode");
        List<MysqlNode> computeNodes =
            Optional.ofNullable(goldenDbService.getComputeNode(BeanTools.copy(resource, ProtectedEnvironment::new)))
                .orElseGet(ArrayList::new);
        computeNodes.forEach(mysqlNode -> {
            checkComputeNodeName(mysqlNode);
            checkComputeNodeIp(mysqlNode);
            checkComputeNodePartParent(mysqlNode);
        });
        log.info("start check gtmNode");
        List<Gtm> gtmNodes =
            Optional.ofNullable(goldenDbService.getGtmNode(BeanTools.copy(resource, ProtectedEnvironment::new)))
                .orElseGet(ArrayList::new);
        gtmNodes.forEach(gtm -> {
            checkGtmNodeType(gtm);
            checkGtmNodeIp(gtm);
            checkGtmNodeFlag(gtm);
        });
    }

    private void checkComputeNodeName(MysqlNode mysqlNode) {
        if (!StringUtils.isNotBlank(mysqlNode.getId()) || !StringUtils.isNotBlank(mysqlNode.getName())
            || !StringUtils.isNotBlank(mysqlNode.getRole())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "goldenDb computeNode param is empty");
        }
    }

    private void checkComputeNodeIp(MysqlNode mysqlNode) {
        if (!StringUtils.isNotBlank(mysqlNode.getIp()) || !StringUtils.isNotBlank(mysqlNode.getPort())
            || !StringUtils.isNotBlank(mysqlNode.getNodeType())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "goldenDb computeNode param is empty");
        }
    }

    private void checkComputeNodePartParent(MysqlNode mysqlNode) {
        if (!StringUtils.isNotBlank(mysqlNode.getGroup()) || !StringUtils.isNotBlank(mysqlNode.getParentUuid())
            || !StringUtils.isNotBlank(mysqlNode.getOsUser())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "goldenDb computeNode param is incorrect");
        }
    }

    private void checkGtmNodeType(Gtm gtm) {
        if (!StringUtils.isNotBlank(gtm.getNodeType()) || !StringUtils.isNotBlank(gtm.getOsUser())
            || !StringUtils.isNotBlank(gtm.getParentUuid())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "goldenDb gtmNode param is empty");
        }
    }

    private void checkGtmNodeIp(Gtm gtm) {
        if (!StringUtils.isNotBlank(gtm.getGtmId()) || !StringUtils.isNotBlank(gtm.getGtmIp())
            || !StringUtils.isNotBlank(gtm.getPort())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "goldenDb gtmNode param is empty");
        }
    }

    private void checkGtmNodeFlag(Gtm gtm) {
        if (!StringUtils.isNotBlank(gtm.getMasterFlag())) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "goldenDb gtmNode param is incorrect");
        }
    }

    /**
     * 判断节点所选代理与实际所需的代理是否相同
     *
     * @param resource 受保护资源
     */
    private void checkNodeMatch(ProtectedResource resource) {
        List<MysqlNode> computeNodes =
            Optional.ofNullable(goldenDbService.getComputeNode(BeanTools.copy(resource, ProtectedEnvironment::new)))
                .orElseGet(ArrayList::new);
        computeNodes.stream().forEach(mysqlNode -> {
            ProtectedEnvironment agent = goldenDbService.getEnvironmentById(mysqlNode.getParentUuid());
            if (!agent.getExtendInfo().get(GoldenDbConstant.AGENT_IP_LIST).contains(mysqlNode.getIp())) {
                throw new LegoCheckedException(CommonErrorCode.AGENT_MISMATCH_NODE,
                    new String[] {agent.getEndpoint(), mysqlNode.getName()}, "The node information does not match.");
            }
        });
    }

    /**
     * 判断节点所选代理与实际所需的代理是否相同
     *
     * @param resource 受保护资源
     */
    private void checkGtmNodeMatch(ProtectedResource resource) {
        log.info("goldenDB start checkGtmNodeMatch");
        List<Gtm> gtmNodes =
            Optional.ofNullable(goldenDbService.getGtmNode(BeanTools.copy(resource, ProtectedEnvironment::new)))
                .orElseGet(ArrayList::new);
        gtmNodes.forEach(gtm -> {
            ProtectedEnvironment agent = goldenDbService.getEnvironmentById(gtm.getParentUuid());
            if (!agent.getExtendInfo().get(GoldenDbConstant.AGENT_IP_LIST).contains(gtm.getGtmIp())) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT, "gtm Node misMatch");
            }
        });
    }

    /**
     * 设置集群实例的path，保证副本复制不会出错
     *
     * @param resource 集群实例资源
     */
    private void setClusterInstancePath(ProtectedResource resource) {
        List<String> managerNodeEndpoints = goldenDbService.getEnvironmentById(resource.getParentUuid())
            .getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(protectedResource -> goldenDbService.getEnvironmentById(protectedResource.getUuid()).getEndpoint())
            .collect(Collectors.toList());
        List<String> instanceNodeEndpoints = resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(protectedResource -> goldenDbService.getEnvironmentById(protectedResource.getUuid()).getEndpoint())
            .collect(Collectors.toList());
        managerNodeEndpoints.addAll(instanceNodeEndpoints);
        String path = managerNodeEndpoints.stream().distinct().sorted().collect(Collectors.joining(","));
        resource.setPath(path);
    }

    /**
     * 检查受保护资源， 修改逻辑资源（文件集，NAS共享）时调用该接口
     * 检查不通过抛出DataProtectionAccessException,并携带对应的错误码<br/>
     * 回调函数中不允许对资源的UUID等关键字段进行修改。
     * <p>
     * 提供的资源不包含dependency信息，如果应用需要补齐depen信息，请调用 “补充资源的dependency信息” 接口
     *
     * @param resource 受保护资源
     */
    @Override
    public void beforeUpdate(ProtectedResource resource) {
        beforeCreate(resource);
    }

    private void computeNodeCheck(ProtectedEnvironment environment) {
        String clusterInfo = environment.getExtendInfo().get(GoldenDbConstant.CLUSTER_INFO);
        List<Group> groups = JsonUtil.read(clusterInfo, GoldenInstance.class).getGroup();
        groups.stream().forEach(group -> {
            group.getMysqlNodes().stream().forEach(mysqlNode -> {
                if (!goldenDbService.singleConnectCheck(mysqlNode, environment)) {
                    throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                        "The GoldenDB instance computeNode query failed.");
                }
            });
        });
    }

    private void gtmNodeCheck(ProtectedEnvironment environment) {
        List<Gtm> gtmNodes = Optional.ofNullable(goldenDbService.getGtmNode(environment)).orElse(Lists.newArrayList());
        gtmNodes.forEach(gtm -> {
            if (!goldenDbService.singleConnectCheck(BeanTools.copy(gtm, MysqlNode::new), environment)) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                    "The GoldenDB instance gtmNode query failed");
            }
        });
    }

    private void managerNodeCheck(ProtectedEnvironment environment) {
        ProtectedEnvironment cluster = goldenDbService.getEnvironmentById(environment.getParentUuid());
        List<Node> managerNode =
            Optional.ofNullable(goldenDbService.getManageDbNode(cluster)).orElse(Lists.newArrayList());
        managerNode.forEach(node -> {
            if (!goldenDbService.singleConnectCheck(BeanTools.copy(node, MysqlNode::new), environment)) {
                throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_QUERY_FAILED,
                    "The GoldenDB instance gtmNode query failed");
            }
        });
    }

    /**
     * 不支持lanfree的应用实现ResourceProvider接口
     *
     * @return GaussDB资源是否更新主机信息配置
     */
    @Override
    public ResourceFeature getResourceFeature() {
        ResourceFeature resourceFeature = ResourceProvider.super.getResourceFeature();
        resourceFeature.setSupportedLanFree(false);
        return resourceFeature;
    }
}
