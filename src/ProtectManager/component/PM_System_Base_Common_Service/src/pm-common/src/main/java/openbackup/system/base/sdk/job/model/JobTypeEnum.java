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
package openbackup.system.base.sdk.job.model;

import openbackup.system.base.util.EnumUtil;

import com.fasterxml.jackson.annotation.JsonCreator;
import com.fasterxml.jackson.annotation.JsonValue;

/**
 * 功能描述
 *
 */
public enum JobTypeEnum {
    /**
     * Backup
     */
    BACKUP("BACKUP"),

    /**
     * RECOVERY
     */
    RESTORE("RESTORE"),

    /**
     * LIVE_RECOVERY
     */
    INSTANT_RESTORE("INSTANT_RESTORE"),

    /**
     * LIVE_MOUNT
     */
    LIVE_MOUNT("live_mount"),

    /**
     * DUPLICATE
     */
    COPY_REPLICATION("copy_replication"),

    /**
     * ARCHIVE
     */
    ARCHIVE("archive"),

    /**
     * CLOUD_ARCHIVE_RESTORE
     */
    CLOUD_ARCHIVE_RESTORE("cloud_archive_restore"),

    /**
     * DELETE_REPLICA
     */
    COPY_DELETE("COPY_DELETE"),

    /**
     * COPY_EXPIRE
     */
    COPY_EXPIRE("COPY_EXPIRE"),

    /**
     * ARCHIVE_IMPORT
     */
    ARCHIVE_IMPORT("archive_import"),

    /**
     * unmount
     */
    UNMOUNT("unmount"),

    /**
     * MIGRATE
     */
    MIGRATE("migrate"),

    /**
     * 资源扫描
     */
    RESOURCE_SCAN("resource_scan"),

    /**
     * 客户端代理自动注册
     */
    HOST_REGISTER("host_register"),

    /**
     * agent更新
     */
    PROTECT_AGENT_UPDATE("protect_agent_update"),

    /**
     * 集群同步
     */
    MULTI_CLUSTER_SYNC("multi_cluster_sync"),

    /**
     * 副本校验任务
     */
    COPY_VERIFY("copy_verify"),

    /**
     * 重新扫描
     */
    RESOURCE_RESCAN("job_type_manual_scan_resource"),

    /**
     * 删除用户
     */
    DELETE_USER("delete_user"),

    /**
     * 防勒索
     */
    ANTI_RANSOMWARE("anti_ransomware"),

    /**
     * agent修改主机资源类型
     */
    PROTECT_AGENT_UPDATE_PLUGIN_TYPE("protect_agent_update_plugin_type"),

    /**
     * 添加备份成员节点
     */
    ADD_BACKUP_MEMBER_CLUSTER("add_backup_member_cluster"),

    /**
     * 删除备份成员节点
     */
    DELETE_BACKUP_MEMBER_CLUSTER("delete_backup_member_cluster"),

    /**
     * 添加HA成员
     */
    ADD_CLUSTER_HA("add_cluster_ha"),

    /**
     * 修改HA参数
     */
    UPDATE_CLUSTER_HA("update_cluster_ha"),

    /**
     * 移除HA成员
     */
    DELETE_CLUSTER_HA("delete_cluster_ha"),

    /**
     * 升级备份存储单元
     */
    UPGRADE_BACKUP_STORAGE_UNIT("upgrade_backup_storage_unit"),

    /**
     * 资源保护
     */
    RESOURCE_PROTECTION("resource_protection"),

    /**
     * 资源修改保护
     */
    RESOURCE_PROTECTION_MODIFY("resource_protection_modify"),

    /**
     * 脱敏识别任务
     */
    DB_IDENTIFICATION("db_identification"),

    /**
     * 脱敏任务
     */
    DB_DESESITIZATION("DB_DESESITIZATION"),

    /**
     * 恢复演练任务
     */
    EXERCISE("exercise"),

    /**
     * 资源组备份
     */
    GROUP_BACKUP("GROUP_BACKUP"),

    /**
     * 证书推送更新的type
     */
    CERT_PUSH_UPDATE_TYPE("CertPushUpdate"),
    /**
     * 资源集创建
     */
    RESOURCE_SET_CREATE("resource_set_create"),

    /**
     * 资源集修改
     */
    RESOURCE_SET_MODIFY("resource_set_modify");

    private final String value;

    JobTypeEnum(String value) {
        this.value = value;
    }

    /**
     * get job log type enum by str
     *
     * @param str str
     * @return job log type enum
     */
    @JsonCreator
    public static JobTypeEnum get(String str) {
        return EnumUtil.get(JobTypeEnum.class, JobTypeEnum::getValue, str, false);
    }

    /**
     * get json value
     *
     * @return json value
     */
    @JsonValue
    public String getValue() {
        return value;
    }
}
