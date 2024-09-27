package openbackup.system.base.common.system;

import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.sdk.SystemSpecificationService;
import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.infrastructure.InfrastructureRestApi;
import openbackup.system.base.sdk.infrastructure.model.InfraResponseWithError;
import openbackup.system.base.sdk.infrastructure.model.beans.NodeDetail;
import openbackup.system.base.service.DeployTypeService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.List;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * System Specification Service Impl
 *
 * @author l00272247
 * @since 2021-03-17
 */
@Component
@Slf4j
@EnableScheduling
public class SystemSpecificationServiceImpl implements SystemSpecificationService {
    /**
     * 节点数量更新时间周期：3分钟
     */
    private static final long NODE_COUNT_UPDATE_PERIOD = IsmNumberConstant.THREE * IsmNumberConstant.MINUTES_SECONDS
        * IsmNumberConstant.THOUSAND;

    private final AtomicInteger nodeCount = new AtomicInteger();

    private final Object nodeCountLock = new Object();

    @Value("${RUNNING_JOB_LIMIT_COUNT_ONE_NODE:20}")
    private int singleNodeJobMaximumConcurrency;

    @Value("${TOTAL_JOB_LIMIT_COUNT_ONE_NODE:10000}")
    private int singleNodeJobMaximumLimit;

    @Autowired
    private ClusterNativeApi clusterNativeApi;


    @Autowired
    private InfrastructureRestApi infrastructureRestApi;

    @Autowired
    private DeployTypeService deployTypeService;

    /**
     * 获取集群节点数量
     *
     * @return 集群节点数量
     */
    @Override
    public int getClusterNodeCount() {
        int count = nodeCount.get();
        if (count == IsmNumberConstant.ZERO) {
            log.debug("First time to query node count.");
            procNodeCountUpdate();
            count = nodeCount.get();
        }
        log.debug("Current node count: {}", count);
        return count;
    }

    /**
     * 定时更新节点数
     */
    @Scheduled(initialDelay = IsmNumberConstant.ZERO, fixedRate = NODE_COUNT_UPDATE_PERIOD)
    public void updateClusterNodeCount() {
        log.debug("Update cluster node count.");
        procNodeCountUpdate();
    }

    private void procNodeCountUpdate() {
        int count = queryClusterNodeCount();
        if (!nodeCount.compareAndSet(count, count)) {
            log.info("node count: {}", count);
            nodeCount.set(count);
        }
    }

    private int queryClusterNodeCount() {
        try {
            if (deployTypeService.isE1000() || deployTypeService.isCyberEngine()) {
                InfraResponseWithError<List<NodeDetail>> infraNodeInfo = infrastructureRestApi.getInfraNodeInfo();
                return Math.max(infraNodeInfo.getData().size(), 1);
            }

            int count = clusterNativeApi.queryCurrentGroupNodeCount();
            return Math.max(count, 1);
        } catch (RuntimeException e) {
            log.error("query cluster node number failed.", ExceptionUtil.getErrorMessage(e));
            return 1;
        }
    }

    /**
     * 获取单节点任务最大并发数
     *
     * @return 单节点任务最大并发数
     */
    @Override
    public int getSingleNodeJobMaximumConcurrency() {
        return singleNodeJobMaximumConcurrency;
    }

    /**
     * 获取单节点任务最大限额数
     *
     * @return 单节点任务最大限额数
     */
    @Override
    public int getSingleNodeJobMaximumLimit() {
        return singleNodeJobMaximumLimit;
    }
}
