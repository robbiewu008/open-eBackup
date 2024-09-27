package openbackup.system.base.common.constants;

/**
 * 功能描述
 *
 * @author y00413474
 * @version [BCManager 8.0.0]
 * @since 2020-06-01
 */
public interface DistributeLockConstant {
    /**
     * 启动时同步最新密钥文件的锁
     */
    String SECRET_SYNC_LOCK = "/kms/lock/SECRET_SYNC_LOCK";

    /**
     * 更新密钥文件的锁
     */
    String SECRET_UPDATE_LOCK = "/kms/lock/SECRET_UPDATE_LOCK";

    /**
     * 更新密钥生命周期的锁
     */
    String KEY_LIFETIME_UPDATE_LOCK = "/kms/lock/KEY_LIFETIME_UPDATE_LOCK";
}
