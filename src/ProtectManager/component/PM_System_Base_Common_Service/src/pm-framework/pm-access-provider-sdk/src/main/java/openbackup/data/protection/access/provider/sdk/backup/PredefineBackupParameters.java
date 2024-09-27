/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
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
