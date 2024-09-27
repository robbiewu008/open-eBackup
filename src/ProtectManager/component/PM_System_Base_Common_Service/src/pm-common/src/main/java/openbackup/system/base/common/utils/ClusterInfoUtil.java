package openbackup.system.base.common.utils;

import openbackup.system.base.sdk.cluster.api.ClusterNativeApi;
import openbackup.system.base.sdk.cluster.model.ClustersInfoVo;
import openbackup.system.base.security.callee.CalleeMethod;
import openbackup.system.base.security.callee.CalleeMethods;

import org.springframework.stereotype.Component;

/**
 * 获取当前集群信息工具类
 *
 * @author y30044273
 * @since 2023-09-18
 */
@Component
@CalleeMethods(name = "cluster_info_util", value = {@CalleeMethod(name = "getCurrentClusterInfo")})
public class ClusterInfoUtil {
    private final ClusterNativeApi clusterNativeApi;

    private ClusterInfoUtil(ClusterNativeApi clusterNativeApi) {
        this.clusterNativeApi = clusterNativeApi;
    }

    /**
     * 记录操作事件logging使用
     *
     * @return 当前节点信息
     */
    public ClustersInfoVo getCurrentClusterInfo() {
        return clusterNativeApi.getCurrentClusterVoInfo();
    }
}
