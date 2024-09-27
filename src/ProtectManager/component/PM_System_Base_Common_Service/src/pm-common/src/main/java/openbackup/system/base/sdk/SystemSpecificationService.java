package openbackup.system.base.sdk;

/**
 * System Specification Service
 *
 * @author l00272247
 * @since 2021-03-20
 */
public interface SystemSpecificationService {
    /**
     * 获取集群节点数量
     *
     * @return 集群节点数量
     */
    int getClusterNodeCount();

    /**
     * 获取单节点任务最大并发数
     *
     * @return 单节点任务最大并发数
     */
    int getSingleNodeJobMaximumConcurrency();

    /**
     * 获取单节点任务最大限额数
     *
     * @return 单节点任务最大限额数
     */
    int getSingleNodeJobMaximumLimit();
}
