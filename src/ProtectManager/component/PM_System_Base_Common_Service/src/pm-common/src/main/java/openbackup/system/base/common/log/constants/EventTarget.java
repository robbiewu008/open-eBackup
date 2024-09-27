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
package openbackup.system.base.common.log.constants;

/**
 * 功能描述
 *
 * @author w00448845
 * @version [BCManager 8.0.0]
 * @since 2020-01-15
 */
public interface EventTarget {
    /**
     * 用户管理
     */
    String USER = "@User";

    /**
     * 用户管理 适用于@Logging
     */
    String USER_V2 = "User";

    /**
     * 告警管理
     */
    String ALARM = "Alarm";

    /**
     * 事件
     */
    String EVENT = "Event";

    /**
     * 集群管理
     */
    String CLUSTER = "Cluster";

    /**
     * 存储服务
     */
    String REPOSITORY = "Repository";

    /**
     * 许可证管理
     */
    String LICENSE = "License";


    /**
     * 秘钥管理 适用于@Logging
     */
    String KMS_V2 = "KMS";

    /**
     * SLA
     */
    String SLA = "SLA";

    /**
     * 调度器
     */
    String SCHEDULER = "Scheduler";

    /**
     * 即时挂载
     */
    String LIVE_MOUNT = "LiveMount";

    /**
     * 恢复
     */
    String RESTORE = "Restore";

    /**
     * 副本目录
     */
    String COPY_CATALOG = "CopyCatalog";

    /**
     * 通知管理
     */
    String NOTIFY = "Notify";

    /**
     * 资源管理
     */
    String RESOURCE = "Resource";

    /**
     * 保护
     */
    String PROTECTION = "Protection";

    /**
     * 恢复
     */
    String RECOVERY = "Recovery";

    /**
     * 复制
     */
    String REPLICATION = "Replication";

    /**
     * 数据保护引擎
     * 适用于新的操作日志标签
     */
    String BACKUP_CLUSTER_NEW = "BackupCluster";

    /**
     * 系统
     */
    String SYSTEM = "System";


    /**
     * 任务中心
     * 适用于新的操作日志标签
     */
    String JOB_NEW = "JOB";


    /**
     * 导出文件
     */
    String FILE_EXPORT = "FileExport";

    /**
     * 外部系统 适用于@Logging
     */
    String EXTERNAL_SYSTEM = "ExternalSystem";

    /**
     * 资源集
     */
    String RESOURCE_SET = "ResourceSet";

    /**
     * void method
     */
    void voidMethod();
}
