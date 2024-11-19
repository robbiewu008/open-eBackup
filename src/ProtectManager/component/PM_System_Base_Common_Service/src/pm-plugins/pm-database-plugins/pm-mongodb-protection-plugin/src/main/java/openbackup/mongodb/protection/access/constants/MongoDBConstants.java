/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
package openbackup.mongodb.protection.access.constants;

import openbackup.mongodb.protection.access.enums.MongoDBClusterRoleEnum;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

/**
 * MongoDB常量
 *
 */
public class MongoDBConstants {
    /**
     * MongoDB集群最少节点数
     */
    public static final int CLUSTER_NODE_LEAST_COUNT = 1;

    /**
     * 节点角色类型的key 1->主 2->备
     */
    public static final String ROLE = "role";

    /**
     * 集群节点ip和port
     */
    public static final String DATA_PATH = "data_path";

    /**
     * 投票权
     */
    public static final String VOTE_RIGHT = "votes";

    /**
     * 优先级
     */
    public static final String PRIORITY = "priority";

    /**
     * 备份配置文件目录
     */
    public static final String CONFIG_PATH = "config_path";

    /**
     * 集群节点ip和port
     */
    public static final String CLUSTER_NODES = "clusterNodes";

    /**
     * 节点IP
     */
    public static final String SERVICE_IP = "serviceIp";

    /**
     * 节点Port
     */
    public static final String SERVICE_PORT = "servicePort";

    /**
     * 集群类型
     */
    public static final String CLUSTE_TYPE = "clusterType";

    /**
     * 集群类型
     */
    public static final String AGENT_CLUSTE_TYPE = "cluster_type";

    /**
     * 集群类型
     */
    public static final String CLUSTER_STATUS = "1";

    /**
     * local_host
     */
    public static final String LOCAL_HOST = "local_host";

    /**
     * agent_host
     */
    public static final String AGENT_HOST = "agent_host";

    /**
     * 集群类型
     */
    public static final String SHARD_CLUSTER_TYPE = "shard_cluster_type";

    /**
     * 集群节点state
     */
    public static final String STATE_STR = "stateStr";

    /**
     * 日志备份类型
     */
    public static final String LOG_BACKUP_TYPE = "logBackup";

    /**
     * 多节点执行
     */
    public static final String MULTI_POST_JOB = "multiPostJob";

    /**
     * 多节状态 1-在线,0离线
     */
    public static final String NODE_HEALTH = "health";

    /**
     * 集群节点列表
     */
    public static final String ENDPOINT_LIST = "endpointList";

    /**
     * 删除节点列表
     */
    public static final String DELETE_NODES_KEY = "-children";

    /**
     * 主机uuid
     */
    public static final String AGENT_UUID = "agentUuid";

    /**
     * 恢复的目标对象key
     */
    public static final String TARGET_LOCATION_KEY = "targetLocation";

    /**
     * 节点nodes存在标识
     */
    public static final String EXIST_NODES = "exist_nodes";

    /**
     * 节点启动key
     */
    public static final String ARGV = "argv";

    /**
     * 份配置文件所有信息key
     */
    public static final String PARSED = "parsed";

    /**
     * 份配置文件所有信息key
     */
    public static final String INSTANCE_NAME = "ret_instance_name";

    /**
     * 实例所属集群列表
     */
    public static final String INSTANCE_NAME_LIST = "instance_name_list";

    /**
     * 实例所属集群列表
     */
    public static final String ID = "_id";

    /**
     * 分片列表集合
     */
    public static final String AGENT_SHARD_LIST = "agent_shard_list";

    /**
     * 查询信息失败的标志
     */
    public static final String FAILED_MARK = "0";

    /**
     * Agent返回信息节点信息列表
     */
    public static final String AGENT_NODES = "agent_nodes";

    /**
     * 多文件系统key
     */
    public static final String MULTI_FILE_SYSTEM = "multiFileSystem";

    /**
     * 错误码Key
     */
    public static final String ERROR_CODE = "errorCode";

    /**
     * 错误码参数Key
     */
    public static final String ERROR_PARAM = "errorParam";

    /**
     * 认证authkey authpwd的长度限制
     */
    public static final int AUTH_KEY_LENGTH = 32;

    /**
     * 单实例类型
     */
    public static final String SINGLE_TYPE = "singleType";

    /**
     * 单实例
     */
    public static final String SINGLE = "single";

    /**
     * 判断是否继续往后走类型
     */
    public static final Map<MongoDBClusterRoleEnum, String> CLUSTER_ROLE_MAP = Collections.unmodifiableMap(
        new HashMap<MongoDBClusterRoleEnum, String>() {
            {
                put(MongoDBClusterRoleEnum.PRIMARY, "1");
                put(MongoDBClusterRoleEnum.SECONDARY, "2");
                put(MongoDBClusterRoleEnum.ARBITER, "7");
                put(MongoDBClusterRoleEnum.FAULT, "0");
            }
        });
}
