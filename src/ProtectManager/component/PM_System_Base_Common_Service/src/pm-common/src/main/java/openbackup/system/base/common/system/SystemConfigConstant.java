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
package openbackup.system.base.common.system;

/**
 * 系统配置的key
 *
 */
public class SystemConfigConstant {
    /**
     * 任务超时时间，单位毫秒
     */
    public static final String LONG_TIME_JOB_TIMEOUT = "long_time_job_timeout_millisecond";

    /**
     * 复制任务总数
     */
    public static final String REPLICATION_JOB_LIMIT_COUNT_KEY = "replication_job_limit_count";

    /**
     * 复制任务查询从端cluster信息缓存时间，单位毫秒
     */
    public static final String QUERY_REPLICATION_CLUSTER_CACHE_TIMEOUT
        = "query_replication_cluster_cache_timeout_millisecond";

    /**
     * 任务总数量限制
     */
    public static final String TOTAL_JOB_LIMIT_COUNT_ONE_NODE = "TOTAL_JOB_LIMIT_COUNT_ONE_NODE";

    /**
     * 单节点运行任务数量限制
     */
    public static final String RUNNING_JOB_LIMIT_COUNT_ONE_NODE = "RUNNING_JOB_LIMIT_COUNT_ONE_NODE";
}