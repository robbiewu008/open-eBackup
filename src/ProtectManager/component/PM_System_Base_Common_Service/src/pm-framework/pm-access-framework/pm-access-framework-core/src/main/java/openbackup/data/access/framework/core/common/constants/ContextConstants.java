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
package openbackup.data.access.framework.core.common.constants;

/**
 * 任务（保护任务、恢复任务、liveMount任务等）的上下文
 *
 * @author y00280557
 * @since 2020-08-21
 */
public final class ContextConstants {
    /**
     * trace ID
     */
    public static final String REQUEST_ID = "request_id";

    /**
     * 任务ID
     */
    public static final String JOB_ID = "job_id";

    /**
     * 任务request ID
     */
    public static final String JOB_REQUEST_ID = "job_request_id";

    /**
     * 任务关联的ID，比如爱数的实例ID，DME的TASK ID
     */
    public static final String JOB_ASSOCIATIVE_ID = "job_associative_id";

    /**
     * 任务状态
     */
    public static final String JOB_STATUS = "job_status";

    /**
     * 资源类型
     */
    public static final String RESOURCE_TYPE = "resource_type";

    /**
     * 任务类型
     */
    public static final String JOB_TYPE = "job_type";

    /**
     * 归档副本ID
     */
    public static final String ARCHIVE_COPY_ID = "archive_copy_id";

    /**
     * 副本ID
     */
    public static final String COPY_ID = "copy_id";

    /**
     * 强制删除副本标志
     */
    public static final String IS_FORCED = "is_forced";

    /**
     * 删除副本数据
     */
    public static final String IS_DELETE_DATA = "is_delete_data";

    /**
     * 备份类型
     */
    public static final String BACKUP_TYPE = "backup_type";

    /**
     * 副本是否可用
     */
    public static final String IS_VISIBLE = "is_visible";

    /**
     * 自动重试次数
     */
    public static final String AUTO_RETRY_TIMES = "auto_retry_times";

    /**
     * 索引失败信息
     */
    public static final String ERROR_CODE = "error_code";

    /**
     * Redis上下文中的资源key
     */
    public static final String RESOURCE = "resource";

    /**
     * 存入redis缓存中的即时挂载信息的键值
     */
    public static final String LIVE_MOUNT = "live_mount";

    /**
     * 存入redis缓存中的即时挂载任务进度
     */
    public static final String JOB_PROGRESS = "job_progress";

    /**
     * 存入redis缓存中的即时挂载topic
     */
    public static final String TOPIC = "topic";

    /**
     * 存入redis缓存中的即时挂载相应topic
     */
    public static final String RESPONSE_TOPIC = "response_topic";

    /**
     * 存入redis缓存中的即时挂载的拓展参数
     */
    public static final String EXTEND_INFO = "extend_info";

    /**
     * 存入redis缓存中的备份副本类型
     */
    public static final String COPY_FORMAT = "copy_format";

    private ContextConstants() {
    }
}
