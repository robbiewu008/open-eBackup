package openbackup.openstack.adapter.constants;

/**
 * OpenStack北向接口错误码
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-30
 */
public final class OpenStackErrorCodes {
    /**
     * 创建备份配额小于已占用配额
     */
    public static final int INIT_QUOTA_LESS_THAN_USED = 403;

    /**
     * 查不到备份、恢复任务或副本信息
     */
    public static final int NOT_FOUND = 404;

    private OpenStackErrorCodes() {}
}
