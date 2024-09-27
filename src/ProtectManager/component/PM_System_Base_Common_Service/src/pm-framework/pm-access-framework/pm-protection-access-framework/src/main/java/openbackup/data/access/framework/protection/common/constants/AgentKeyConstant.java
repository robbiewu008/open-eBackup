/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
