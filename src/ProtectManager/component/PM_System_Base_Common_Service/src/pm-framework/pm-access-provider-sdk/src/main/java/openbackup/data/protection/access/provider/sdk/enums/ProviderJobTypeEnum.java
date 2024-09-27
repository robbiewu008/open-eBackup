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
package openbackup.data.protection.access.provider.sdk.enums;

import java.util.Arrays;

/**
 * SDK中使用的JobType枚举类
 *
 * @author y00559272
 * @since 2021-10-11
 */
public enum ProviderJobTypeEnum {
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
    PROTECT_AGENT_UPDATE("protect_agent_update");

    private final String value;

    ProviderJobTypeEnum(String value) {
        this.value = value;
    }

    /**
     * get job log type enum by str
     *
     * @param jobType job type
     * @return job log type enum
     */
    public static ProviderJobTypeEnum getByType(String jobType) {
        return Arrays.stream(ProviderJobTypeEnum.values())
            .filter(type -> type.value.equals(jobType))
            .findFirst()
            .orElseThrow(IllegalArgumentException::new);
    }

    /**
     * get json value
     *
     * @return json value
     */
    public String getValue() {
        return value;
    }
}
