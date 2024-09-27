package openbackup.system.base.common.exception;

/**
 * 系统异常，所有不需要用户直接介入的异常都定义为LegoUncheckedException
 *
 * @author s90004407
 * @version [Lego V100R002C10, 2014-12-17]
 * @since 2019-10-31
 */
public class LegoUncheckedException extends RuntimeException { // ExceptionLogDecorator
    private static final long serialVersionUID = 5625178995893882625L;

    private long errorCode;

    /**
     * 默认构造函数
     *
     * @param message message
     * @param cause   cause
     */
    public LegoUncheckedException(String message, Throwable cause) {
        super(message, cause);
    }

    /**
     * 默认构造函数
     *
     * @param message message
     */
    public LegoUncheckedException(String message) {
        super(message);
    }

    /**
     * 默认构造函数
     *
     * @param errorCode message
     */
    public LegoUncheckedException(long errorCode) {
        this.errorCode = errorCode;
    }

    /**
     * 构造函数
     *
     * @param errorCode 错误码
     * @param message 错误信息
     */
    public LegoUncheckedException(long errorCode, String message) {
        super(message);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param errorCode message
     * @param cause     cause
     */
    public LegoUncheckedException(long errorCode, Throwable cause) {
        super(cause);
        this.errorCode = errorCode;
    }

    /**
     * 默认构造函数
     *
     * @param cause message
     */
    public LegoUncheckedException(Throwable cause) {
        super(cause);
    }

    public long getErrorCode() {
        return errorCode;
    }
}
