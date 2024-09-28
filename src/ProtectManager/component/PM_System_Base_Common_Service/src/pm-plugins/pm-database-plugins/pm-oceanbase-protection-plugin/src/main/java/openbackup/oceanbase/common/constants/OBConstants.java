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
package openbackup.oceanbase.common.constants;

/**
 * 功能描述
 *
 */
public interface OBConstants {
    /**
     * 扩展extendinfo中的集群信息key
     */
    String KEY_CLUSTER_INFO = "clusterInfo";

    /**
     * 扩展extendinfo中的该租户是否已被注册的key
     */
    String EXTEND_KEY_IS_USED = "isUsed";

    /**
     * OceanBase最大规格, 使用系统默认规格
     */
    int OB_CLUSTER_MAX_COUNT = 2000;

    /**
     * comma 逗号
     */
    String COMMA = ",";

    /**
     * semicolon 分号
     */
    String SEMICOLON = ";";

    /**
     * queryType
     */
    String QUERY_TYPE_KEY = "queryType";

    /**
     * 资源池
     */
    String QUERY_TYPE_POOL = "pool";

    /**
     * 连接检查
     */
    String CONTENT_KEY_CONNECT_RESULT = "connect-result";

    /**
     * 检查类型
     */
    String KEY_CHECK_TYPE = "checkType";

    /**
     * agentIp,用于传递给agent
     */
    String KEY_AGENT_IP = "obAgentIp";

    /**
     * 检查OBClient
     */
    String CHECK_OBCLIENT = "check_obclient";

    /**
     * 检查OBServer
     */
    String CHECK_OBSERVER = "check_observer";

    /**
     * type，标明是注册或修改场景使用
     */
    String KEY_CHECK_SCENE = "scene";

    /**
     * 注册或修改集群
     */
    String CLUSTER_REGISTER = "register";

    /**
     * observer对应的agent
     */
    String SERVER_AGENTS = "serverAgents";

    /**
     * obclient对应的agent
     */
    String CLIENT_AGENTS = "clientAgents";

    /**
     * 执行任务的节点ID
     */
    String EXEC_NODE_ID = "execNodeId";

    /**
     * OceanBase持续挂载key
     */
    String PERSISTENT_MOUNT = "persistentMount";

    /**
     * OceanBase手动挂载key
     */
    String MANUAL_MOUNT = "manualMount";

    /**
     * 是否需要删除Dtree
     */
    String NEED_DELETE_DTREE = "needDeleteDtree";

    /**
     * OceanBase-cluster
     */
    String OCEANBASE_CLUSTER = "OceanBase-cluster";

    /**
     * 传给UBC，标识资源是否存在
     */
    String RESOURCE_EXISTS = "resourceExists";
}
