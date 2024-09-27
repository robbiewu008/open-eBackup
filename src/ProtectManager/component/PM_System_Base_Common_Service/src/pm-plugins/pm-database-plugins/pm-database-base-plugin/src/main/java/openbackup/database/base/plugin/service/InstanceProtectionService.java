package openbackup.database.base.plugin.service;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;

import java.util.List;

/**
 * 数据库的实例保护服务
 *
 * @author lWX776769
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-09-21
 */
public interface InstanceProtectionService {
    /**
     * 获取单实例的主机node
     *
     * @param resource 单实例信息
     * @return node信息
     */
    List<TaskEnvironment> extractEnvNodesBySingleInstance(ProtectedResource resource);

    /**
     * 获取集群实例的主机node
     *
     * @param resource 集群实例信息
     * @return node信息
     */
    List<TaskEnvironment> extractEnvNodesByClusterInstance(ProtectedResource resource);
}
