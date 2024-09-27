/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.system.base.common.constants;

/**
 * Redis Constants
 *
 * @author l00272247
 * @since 2021-01-04
 */
public interface RedisConstants {
    /**
     * TARGET_CLUSTER_RELATED_TASKS
     */
    String TARGET_CLUSTER_RELATED_TASKS = "target-cluster-related-tasks-";

    /**
     * 同步文件到所有k8s节点的data的key
     */
    String SYNC_FILE_TO_ALL_K8S_NODE_DATA_KEY = "syncFileDataKey";

    /**
     * 同步文件到所有k8s节点的stream name
     */
    String SYNC_FILE_TO_ALL_K8S_NODE_NAME = "syncFileToAllK8sNodeName";
}
