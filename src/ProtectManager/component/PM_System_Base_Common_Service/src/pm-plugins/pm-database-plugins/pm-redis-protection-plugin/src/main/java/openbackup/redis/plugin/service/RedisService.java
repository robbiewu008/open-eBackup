package openbackup.redis.plugin.service;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

/**
 * Redis Service
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
public interface RedisService {
    /**
     * 在创建或者更新集群之前做一些校验工作。
     *
     * @param protectedEnvironment 集群
     */
    void preCheck(ProtectedEnvironment protectedEnvironment);

    /**
     * 选择一个在线的agent
     *
     * @param child child
     * @return agent
     */
    Endpoint selectAgent(ProtectedResource child);

    /**
     * 校验集群节点是否已经添加过。
     *
     * @param protectedResource 集群节点
     */
    void checkNodeExists(ProtectedResource protectedResource);
}