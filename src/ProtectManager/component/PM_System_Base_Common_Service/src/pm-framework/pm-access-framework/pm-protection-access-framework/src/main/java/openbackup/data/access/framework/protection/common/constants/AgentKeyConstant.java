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
package openbackup.data.access.framework.protection.common.constants;

/**
 * Agent选择器对应的高级参数
 *
 * @since 2022-04-21
 */
public class AgentKeyConstant {
    /**
     * 环境uuid
     */
    public static final String ENVIRONMENT_UUID_KEY = "environment_id";

    /**
     * agents选择参数
     */
    public static final String AGENTS_KEY = "agents";

    /**
     * 用户信息
     */
    public static final String USER_INFO = "user_info";

    /**
     * 恢复任务多个agent uuid分隔符号
     */
    public static final String AGENTS_SPLIT = ";";

    /**
     * nas插件所需agent,多个以;分割
     */
    public static final String COPY_AGENT = "copy_agent";

    /**
     * 集群esn
     */
    public static final String CLUSTER_ESN = "esn";
}
