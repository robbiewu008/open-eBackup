package openbackup.openstack.adapter.exception;

/**
 * OpenStack北向接口异常类
 *
 * @author w00616953
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-01-30
 */
public class OpenStackException extends RuntimeException {
    private final long errorCode;

    public OpenStackException(long errorCode) {
        this.errorCode = errorCode;
    }

    /**
     * 构造方法
     *
     * @param errorCode 错误码
     * @param message 错误信息
     */
    public OpenStackException(long errorCode, String message) {
        super(message);
        this.errorCode = errorCode;
    }

    public long getErrorCode() {
        return errorCode;
    }
}
