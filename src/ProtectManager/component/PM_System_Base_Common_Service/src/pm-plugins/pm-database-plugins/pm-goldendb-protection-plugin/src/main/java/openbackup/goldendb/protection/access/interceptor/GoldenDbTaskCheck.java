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
package openbackup.goldendb.protection.access.interceptor;

import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.goldendb.protection.access.provider.GoldenDbClusterProvider;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.concurrent.atomic.AtomicReference;

/**
 * 功能描述 校验goldenDb任务
 *
 * @author s30036254
 * @since 2023-03-14
 */
@Slf4j
@Component
public class GoldenDbTaskCheck {
    private final GoldenDbService goldenDbService;

    private final GoldenDbClusterProvider goldenDbClusterProvider;

    /**
     * 构造器
     *
     * @param goldenDbService goldenDbService
     * @param goldenDbClusterProvider goldenDbClusterProvider
     */
    public GoldenDbTaskCheck(GoldenDbService goldenDbService, GoldenDbClusterProvider goldenDbClusterProvider) {
        this.goldenDbService = goldenDbService;
        this.goldenDbClusterProvider = goldenDbClusterProvider;
    }

    /**
     * 校验生产环境的变化
     *
     * @param clusterId 集群id
     * @param instanceId 实例id
     * @return GoldenInstance
     */
    public GoldenInstance checkEnvChange(String clusterId, String instanceId) {
        // 获取备份实例
        String backupInstanceJson =
            goldenDbService.getResourceById(instanceId).getExtendInfo().get(GoldenDbConstant.CLUSTER_INFO);
        GoldenInstance backupInstance = JsonUtil.read(backupInstanceJson, GoldenInstance.class);

        // 获取扫描实例
        ProtectedEnvironment cluster = goldenDbService.getEnvironmentById(clusterId);
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setResourceType(ResourceSubTypeEnum.GOLDENDB_CLUSTER.getType());
        AtomicReference<GoldenInstance> browseInstance = new AtomicReference<>(new GoldenInstance());
        goldenDbClusterProvider.browse(cluster, conditions).getRecords().forEach(protectedResource -> {
            String clusterInfo = protectedResource.getExtendInfo().get(GoldenDbConstant.CLUSTER_INFO);
            HashMap<String, String> read = JsonUtil.read(clusterInfo, HashMap.class);
            if (read.get(GoldenDbConstant.INSTANCE_ID).equals(backupInstance.getId())) {
                browseInstance.set(JsonUtil.read(clusterInfo, GoldenInstance.class));
            }
        });
        checkDifference(backupInstance, browseInstance.get(), instanceId);
        return backupInstance;
    }

    /**
     * 判断生产环境是否变化+主备切换
     *
     * @param backupInstance 备份实例
     * @param browseInstance 扫描出来的实例
     * @param instanceId 实例id
     */
    private void checkDifference(GoldenInstance backupInstance, GoldenInstance browseInstance, String instanceId) {
        // 判断节点个数是否相同
        ProtectedEnvironment backupEnvironment = new ProtectedEnvironment();
        backupEnvironment.setExtendInfoByKey(GoldenDbConstant.CLUSTER_INFO, JsonUtil.json(backupInstance));
        ProtectedEnvironment browseEnvironment = new ProtectedEnvironment();
        browseEnvironment.setExtendInfoByKey(GoldenDbConstant.CLUSTER_INFO, JsonUtil.json(browseInstance));
        if (goldenDbService.getComputeNode(backupEnvironment)
            .size() != goldenDbService.getComputeNode(browseEnvironment).size()) {
            updateInstanceStatus(instanceId);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                "The number of backup tasks is inconsistent with that in the production environment.");
        }

        // 判断节点ip+port是否相同
        backupInstance.getGroup().forEach(group -> {
            List<MysqlNode> mysqlNodes = group.getMysqlNodes();
            mysqlNodes.forEach(mysqlNode -> {
                MysqlNode getMysqlNodeById = getMysqlNodeById(mysqlNode.getId(), browseInstance, instanceId);
                if (!getMysqlNodeById.getIp().equals(mysqlNode.getIp())
                    || !getMysqlNodeById.getPort().equals(mysqlNode.getPort())) {
                    updateInstanceStatus(instanceId);
                    throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                        "The cluster node added by the user is different from the actual cluster node.");
                }
                mysqlNode.setRole(getMysqlNodeById.getRole());
            });
        });
    }

    /**
     * 获取对应id的节点
     *
     * @param mysqlNodeId 备份实例的id
     * @param browseInstance 扫描出来的实例
     * @param instanceId 实例id
     * @return 对应id的节点
     */
    private MysqlNode getMysqlNodeById(String mysqlNodeId, GoldenInstance browseInstance, String instanceId) {
        MysqlNode result = new MysqlNode();
        browseInstance.getGroup().forEach(group -> {
            List<MysqlNode> mysqlNodes = group.getMysqlNodes();
            mysqlNodes.forEach(mysqlNode -> {
                if (mysqlNode.getId().equals(mysqlNodeId)) {
                    BeanTools.copy(mysqlNode, result);
                }
            });
        });
        if (result.getId() == null) {
            log.error("The instance scanned from the production environment does not contain this node {}",
                mysqlNodeId);
            updateInstanceStatus(instanceId);
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                "The cluster node added by the user is different from the actual cluster node.");
        }
        return result;
    }

    /**
     * 若参数与生产环境不一致，需要置实例状态为离线
     *
     * @param instanceId 实例id
     */
    private void updateInstanceStatus(String instanceId) {
        goldenDbService.updateResourceLinkStatus(instanceId, LinkStatusEnum.OFFLINE.getStatus().toString());
    }
}
