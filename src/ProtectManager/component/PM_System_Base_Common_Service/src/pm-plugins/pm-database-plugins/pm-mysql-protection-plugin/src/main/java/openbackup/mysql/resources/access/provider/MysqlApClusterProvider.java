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
package openbackup.mysql.resources.access.provider;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DbClusterProvider;
import openbackup.mysql.resources.access.common.MysqlConstants;
import openbackup.mysql.resources.access.common.MysqlErrorCode;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * MYSQL 主备集群 检验provider
 *
 * @author fwx1022842
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/14
 */
@Slf4j
@Component
public class MysqlApClusterProvider implements DbClusterProvider {
    private final ResourceService resourceService;

    /**
     * 构造器注入
     *
     * @param resourceService 资源服务
     */
    public MysqlApClusterProvider(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    /**
     * 校验集群条件是否构成物理集群
     *
     * @param protectedResource 资源
     * @return true 构成集群， false 不构成集群
     */
    @Override
    public boolean checkIsCluster(ProtectedResource protectedResource) {
        // 检验实例数量是否和agent数量一致
        checkClusterNum(protectedResource);
        return true;
    }

    /**
     * 校验主备节点数是否能满足需求
     *
     * @param nodesCondition 节点是否是主节点情况
     */
    @Override
    public void checkNodeRoleCondition(List<Boolean> nodesCondition) {
        long masterCount = nodesCondition.stream().filter(bool -> bool).count();
        long slaveCount = nodesCondition.stream().filter(bool -> !bool).count();
        if (masterCount != 1 || slaveCount < 1) {
            throw new LegoCheckedException(MysqlErrorCode.CHECK_MYSQL_DEPLOYMENT_MODEL_FAILED,
                "node deploy type not legal");
        }
    }

    /**
     * 校验集群主机数量和实例数量是否匹配
     * 根据环境id查询该环境下的主机数与当前用户选择的主机数量进行比较
     *
     * @param protectedResource 前端传回的数据
     */
    private void checkClusterNum(ProtectedResource protectedResource) {
        String envUuid = protectedResource.getRootUuid();
        ProtectedResource clusterEnvResource = resourceService.getResourceById(envUuid)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "No host exists in the cluster."));
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(protectedResource.getDependencies())
            .orElse(new HashMap<>());
        List<ProtectedResource> instances = Optional.ofNullable(dependencies.get(DatabaseConstants.CHILDREN))
            .orElse(Collections.emptyList());
        int agentsNums = clusterEnvResource.getDependencies().get(DatabaseConstants.AGENTS).size();
        if (agentsNums != instances.size()) {
            log.error("The number of hosts in the cluster {} does not match the number of instances", envUuid);
            throw new LegoCheckedException(MysqlErrorCode.CHECK_CLUSTER_FAILED,
                "The number of cluster nodes does not match");
        }
    }

    /**
     * 适用范围
     *
     * @param clusterType 集群类型
     * @return 是否适用
     */
    @Override
    public boolean applicable(String clusterType) {
        return MysqlConstants.MYSQL_AP.equals(clusterType);
    }
}