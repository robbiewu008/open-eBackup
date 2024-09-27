/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.opengauss.resources.access.constants;

/**
 * OpenGauss相关常量
 *
 * @author jwx701567
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-23
 */
public class OpenGaussConstants {
    /**
     * 集群状态 Normal 为正常状态
     */
    public static final String CLUSTER_STATE = "clusterState";

    /**
     * 集群信息 Normal 为正常状态
     */
    public static final String CLUSTER_INFO = "clusterInfo";

    /**
     * 集群nodes信息
     */
    public static final String NODES = "nodes";

    /**
     * 集群角色对应的role
     */
    public static final String CLUSTER_NODE_ROLE = "role";

    /**
     * 前端下发的节点信息
     */
    public static final String GUI_NODES = "guiNodes";

    /**
     * 状态信息
     */
    public static final String STATUS = "status";

    /**
     * 分隔符息
     */
    public static final String PATH_DELIMITER = "/";

    /**
     * 数据库环境的唯一系统id
     */
    public static final String SYSTEM_ID = "systemId";

    /**
     * 单节点
     */
    public static final int SINGLE_NODE = 1;

    /**
     * OpenGauss注册集群上限值
     */
    public static final int OPENGAUSS_CLUSTER_MAX_COUNT = 2000;
}
