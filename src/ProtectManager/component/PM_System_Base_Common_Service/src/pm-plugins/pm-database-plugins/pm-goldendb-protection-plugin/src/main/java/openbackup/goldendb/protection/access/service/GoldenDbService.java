package openbackup.goldendb.protection.access.service;

import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.goldendb.protection.access.dto.cluster.Node;
import openbackup.goldendb.protection.access.dto.instance.Gtm;
import openbackup.goldendb.protection.access.dto.instance.MysqlNode;
import openbackup.system.base.common.model.job.JobBo;

import java.util.List;
import java.util.Optional;

/**
 * 功能描述 GoldenDb服务
 *
 * @author s30036254
 * @since 2023-02-13
 */
public interface GoldenDbService {
    /**
     * 获取受保护环境
     *
     * @param envId 环境id
     * @return 受保护环境
     */
    ProtectedEnvironment getEnvironmentById(String envId);

    /**
     * 获取受保护资源
     *
     * @param uuid 资源唯一id
     * @return 受保护资源
     */
    ProtectedResource getResourceById(String uuid);

    /**
     * 获取单个节点的连通性
     *
     * @param mysqlNode mysql节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean singleConnectCheck(MysqlNode mysqlNode, ProtectedEnvironment environment);

    /**
     * 获取单个节点的连通性
     *
     * @param mysqlNode mysql节点
     * @param environment 受保护环境
     * @return 是否连接
     */
    boolean singleHealthCheck(MysqlNode mysqlNode, ProtectedEnvironment environment);

    /**
     * 获取计算节点对应的mysql
     *
     * @param environment 受保护环境
     * @return 计算节点
     */
    List<MysqlNode> getComputeNode(ProtectedEnvironment environment);

    /**
     * 获取管理数据库节点对应的mysql
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    List<Node> getManageDbNode(ProtectedEnvironment environment);

    /**
     * 获取管理数据库节点对应的mysql
     *
     * @param environment 受保护环境
     * @return 管理数据库节点
     */
    List<Gtm> getGtmNode(ProtectedEnvironment environment);

    /**
     * 获取所有的GoldenDb环境
     *
     * @param updateUuid updateUuid
     * @return 受保护环境
     */
    List<ProtectedResource> getGoldenDbEnv(String updateUuid);

    /**
     * 更新资源的状态
     *
     * @param resourceId resourceId
     * @param status status
     */
    void updateResourceLinkStatus(String resourceId, String status);

    /**
     * 得到环境下面的所有子实例
     *
     * @param parentUuid 环境uuid
     * @return 子资源
     */
    List<ProtectedResource> getChildren(String parentUuid);

    /**
     * 查询当前最新的任务
     *
     * @param instanceId 实例id
     * @param type 备份/恢复
     * @return 任务状况
     */
    Optional<JobBo> queryLatestJob(String instanceId, String type);
}
