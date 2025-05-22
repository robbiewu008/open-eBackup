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
package openbackup.oceanprotect.k8s.protection.access.constant;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * 功能描述: ExtendInfoKey
 *
 */
public class K8sExtendInfoKey {
    /**
     * 框架的 environment 的 Auth 扩展信息 extendInfo 中 config 的 Key 名称
     */
    public static final String CONFIG = "configKey";

    /**
     * 框架的 environment 的 Auth 扩展信息 extendInfo 中 token 的 Key 名称
     */
    public static final String TOKEN = "token";

    /**
     * 框架的 environment 的 Auth 扩展信息 extendInfo 中 certificateAuthorityData 的 Key 名称
     */
    public static final String CERTIFICATE_AUTHORITY_DATA = "certificateAuthorityData";

    /**
     * 内置AGENT与k8s的连通性的结果，放在extend info中，该值为key的前缀，后续跟agentId; 每次健康检查更新
     */
    public static final String INTERNAL_AGENT_CONNECTION_PREFIX = "InternalAgentConnection_";

    /**
     * 框架的 environment 的 extendInfo扩展信息中jobNumOnSingleNode的 Key名称
     */
    public static final String JOB_NUM_ON_SINGLE_NODE = "jobNumOnSingleNode";

    /**
     * 框架的 environment 的 extendInfo扩展信息中imageNameAndTag的 Key名称
     */
    public static final String IMAGE_NAME_AND_TAG = "imageNameAndTag";

    /**
     * 框架的 environment 的 extendInfo扩展信息中nodeSelector的 Key名称
     */
    public static final String NODE_SELECTOR = "nodeSelector";

    /**
     * 框架的 environment 的 extendInfo扩展信息中nodeSelector的 Key名称
     */
    public static final String IS_VERIFY_SSL = "isVerifySsl";

    /**
     * k8s备份、恢复任务超时时间，默认24小时
     */
    public static final String TASK_TIMEOUT = "taskTimeout";

    /**
     * k8s一致性备份脚本执行超时时间，默认1小时
     */
    public static final String CONSISTENT_SCRIPT_EXEC_TIMEOUT = "consistentScriptTimeout";

    /**
     * k8s的集群类型：cce、openshift、k8s
     */
    public static final String CLUSTER_TYPE = "clusterType";

    /**
     * k8s的集群类型：cce、openshift、k8s
     */
    public static final List<String> SUPPORT_CLUSTER_TYPE = Collections
            .unmodifiableList(Arrays.asList("cce", "openshift", "k8s"));

    /**
     * k8s的集群版本前缀正则匹配规则：获取v1.22.1-h0.dsiv.puma.r28-dirty的v1.22
     */
    public static final String VERSION_PREFIX_PATTERN = "(v\\d+\\.\\d+)";
}