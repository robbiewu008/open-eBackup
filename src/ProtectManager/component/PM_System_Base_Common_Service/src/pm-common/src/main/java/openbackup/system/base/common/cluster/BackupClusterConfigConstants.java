/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.system.base.common.cluster;

/**
 * 获取configMap中集群配置常量类
 *
 * @author z00613137
 * @since 2023-05-26
 */
public class BackupClusterConfigConstants {
    /**
     * 挂载路径
     */
    public static final String OPT_CONFIG = "/opt/cluster_config";

    /**
     * 节点Esn
     */
    public static final String CLUSTER_ESN = "CLUSTER_ESN";

    /**
     * 节点角色ROLE
     */
    public static final String CLUSTER_ROLE = "CLUSTER_ROLE";

    /**
     * service ip
     */
    public static final String CLUSTER_SERVICE_IPS = "CLUSTER_SERVICE_IPS";
}
