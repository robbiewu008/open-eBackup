package openbackup.data.protection.access.provider.sdk.exception;

/**
 * 数据保护接入异常
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-10-12
 */
public class DataProtectionAccessException extends RuntimeException {
    private long errorCode;
    private String[] parameters;

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     */
    @Deprecated
    public DataProtectionAccessException(long errorCode, String[] parameter) {
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode errorCode
     * @param parameter parameter
     * @param cause     具体的异常
     */
    public DataProtectionAccessException(long errorCode, String[] parameter, Throwable cause) {
        super(cause.getMessage(), cause);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    /**
     * 构造函数
     *
     * @param errorCode 错误码
     * @param parameter 参数
     * @param message 错误消息
     */
    public DataProtectionAccessException(long errorCode, String[] parameter, String message) {
        super(message);
        this.errorCode = errorCode;
        this.parameters = parameter;
    }

    public long getErrorCode() {
        return errorCode;
    }

    public void setErrorCode(long errorCode) {
        this.errorCode = errorCode;
    }

    public String[] getParameters() {
        return parameters;
    }

    public void setParameters(String[] parameters) {
        this.parameters = parameters;
    }

    /**
     * raise exception
     */
    public void raise() {
        throw this;
    }
}
