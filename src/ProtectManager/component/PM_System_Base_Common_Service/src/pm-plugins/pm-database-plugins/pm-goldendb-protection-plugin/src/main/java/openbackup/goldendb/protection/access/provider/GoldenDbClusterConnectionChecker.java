/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.goldendb.protection.access.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.cluster.GoldenCluster;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-11
 */
@Component
@Slf4j
public class GoldenDbClusterConnectionChecker extends UnifiedResourceConnectionChecker {
    private final GoldenDbService goldenDbService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param goldenDbService goldenDbService
     */
    public GoldenDbClusterConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, GoldenDbService goldenDbService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.goldenDbService = goldenDbService;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType().equals(object.getSubType());
    }

    /**
     * 获取集群节点及其对应的主机
     *
     * @param environment 集群
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>>
        collectConnectableResources(ProtectedResource environment) {
        log.info("GoldenDBConnectionChecker,collectConnectableResources,environment.uuid: {}", environment.getUuid());
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new LinkedHashMap<>();
        String goldenDb = environment.getExtendInfo().get(GoldenDbConstant.GOLDEN_CLUSTER);
        GoldenCluster goldenCluster = JsonUtil.read(goldenDb, GoldenCluster.class);
        goldenCluster.getNodes().stream().forEach(node -> {
            ProtectedEnvironment agentEnvironment = goldenDbService.getEnvironmentById(node.getParentUuid());
            log.info("agent_id: {},ip is {}", node.getParentUuid(), agentEnvironment.getEndpoint());
            ProtectedResource protectedResource = new ProtectedResource();
            protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
            protectedResource.setSubType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
            HashMap<java.lang.String, java.lang.String> extendInfo = new HashMap<>();
            collectManagerNodePart(node, extendInfo);
            protectedResource.setExtendInfo(extendInfo);
            nodeHostMap.put(protectedResource,
                Lists.newArrayList(BeanTools.copy(agentEnvironment, ProtectedEnvironment::new)));
        });
        return nodeHostMap;
    }

    private void collectManagerNodePart(Node node, HashMap<String, String> extendInfo) {
        for (Field field : node.getClass().getDeclaredFields()) {
            field.setAccessible(true);
            try {
                if (field.get(node) != null) {
                    extendInfo.put(field.getName(), field.get(node).toString());
                }
            } catch (IllegalAccessException e) {
                log.error("can not get param");
            }
        }
    }
}
