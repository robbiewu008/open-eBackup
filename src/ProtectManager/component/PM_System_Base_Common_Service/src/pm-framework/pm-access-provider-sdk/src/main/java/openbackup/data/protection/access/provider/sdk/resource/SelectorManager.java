/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.data.protection.access.provider.sdk.resource;

import openbackup.data.protection.access.provider.sdk.base.Endpoint;

import java.util.List;
import java.util.Map;

/**
 * 选择器管理器
 *
 * @author w30044259
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-29
 */
public interface SelectorManager {
    /**
     * 根据资源信息选择agent
     *
     * @param resource 资源
     * @param jobType 任务类型
     * @param parameters 扩展参数
     * @return 当前任务所需的agent
     */
    List<Endpoint> selectAgentByResource(ProtectedResource resource, String jobType, Map<String, String> parameters);
}
