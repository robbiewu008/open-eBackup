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
package openbackup.data.protection.access.provider.sdk.backup;

/**
 * 预置的备份参数key
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-03
 */
public enum PredefineBackupParameters {
    // 是否加密
    ENCRYPTION("encryption"),
    // 前置脚本
    PRE_SCRIPT("pre_script"),
    // 后置脚本
    POST_SCRIPT("post_script"),
    // 失败脚本
    FAILED_SCRIPT("failed_script"),
    // 自动重试
    AUTO_RETRY("auto_retry"),
    // 重试次数
    AUTO_RETRY_TIMES("auto_retry_times"),
    // 重试等待时间
    AUTO_RETRY_WAIT_MINUTES("auto_retry_wait_minutes"),
    // QOS ID
    QOS_ID("qos_id"),
    // 重删
    DEDUPLICATION("deduplication"),
    // 源端重删
    SOURCE_DEDUPLICATION("source_deduplication"),

    // 重删压缩
    ENABLE_DEDUPTION_COMPRESSION("enable_deduption_compression");

    private String key;

    PredefineBackupParameters(String key) {
        this.key = key;
    }

    public String getKey() {
        return this.key;
    }
}
