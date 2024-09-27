package openbackup.data.protection.access.provider.sdk.backup;

/**
 * 更改下次备份的原因
 *
 * @author h30027154
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-10-13
 */
public enum NextBackupChangeCauseEnum {
    // 恢复成功
    // 资源进行恢复操作之后第一次备份将转换为全量备份
    RESTORE_SUCCESS_TO_FULL(1, "restore_success_to_full_label"),

    // 副本删除成功
    // 资源进行最新全量，差异副本删除/过期操作之后第一次备份将转换为全量备份
    NEWEST_FULL_OR_DIFFERENCE_COPY_DELETE_SUCCESS_TO_FULL(1,
        "newest_full_or_difference_copy_delete_success_to_full_label"),

    // 副本删除成功
    // 资源进行最新全量，增量副本删除/过期操作之后第一次备份将转换为全量备份
    LATEST_FULL_OR_INCREMENTAL_COPY_DELETE_SUCCESS_TO_FULL_LABEL(1,
        "latest_full_or_incremental_copy_delete_success_to_full_label"),

    // 全量备份失败，通用逻辑
    // 由于上次全量备份任务失败，本次任务将执行全量备份
    BIGDATA_PLUGIN_TO_FULL_LABEL(2, "bigdata_plugin_to_full_label"),

    VERIFY_FAILED_TO_FULL_LABEL(3, "verify_failed_to_full_label"),

    // 多集群任务分发,当备份任务不是全量和永久增量备份，并且最近一次全量备份的节点故障时，需要转增量为全量
    MULTIPLE_CLUSTERS_INCREMENTAL_DISTRIBUTION_FAILED_TO_FULL_LABEL(4,
        "multiple_clusters_incremental_distribution_failed_to_full_label"),

    // SanClient场景，从关闭转为开启，需转全量
    SANCLIENT_ENABLED(5, "job_log_sanclient_enabled_first_backup_convert_full_label"),

    // SanClient场景，从开启转为关闭，需转全量
    SANCLIENT_DISABLED(6, "job_log_sanclient_disabled_first_backup_convert_full_label");

    private final int index;

    private final String label;

    NextBackupChangeCauseEnum(int index, String label) {
        this.index = index;
        this.label = label;
    }

    public String getLabel() {
        return label;
    }

    public int getIndex() {
        return index;
    }
}
