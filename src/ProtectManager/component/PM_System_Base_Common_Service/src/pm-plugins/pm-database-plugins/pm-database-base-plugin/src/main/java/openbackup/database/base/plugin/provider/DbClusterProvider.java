/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.database.base.plugin.provider;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * 集群实例注册校验使用
 *
 * @author wx950025
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-25
 */
public interface DbClusterProvider extends DataProtectionProvider<String> {
    /**
     * 校验集群条件是否构成物理集群
     *
     * @param protectedResource 受保护资源
     * @return true 构成集群， false 不构成集群
     */
    boolean checkIsCluster(ProtectedResource protectedResource);

    /**
     * 校验节点是否满足组成集群
     *
     * @param nodesCondition 节点是否是主节点情况
     */
    default void checkNodeRoleCondition(List<Boolean> nodesCondition) {
    }

    /**
     * 根据集群类型过滤对应的集群校验bean
     *
     * @param clusterType 集群类型
     * @return 匹配到bean true, 未匹配到 false
     */
    @Override
    boolean applicable(String clusterType);
}
