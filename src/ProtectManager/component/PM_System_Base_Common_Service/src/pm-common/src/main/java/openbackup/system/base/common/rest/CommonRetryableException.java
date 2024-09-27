package openbackup.system.base.common.rest;

import feign.Request;
import feign.RetryableException;

import java.util.Date;

/**
 * CommonRetryableException
 *
 * @author h30027154
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2024/1/17
 */
public class CommonRetryableException extends RetryableException {
    private Exception exception;

    public CommonRetryableException(int status, String message, Request.HttpMethod httpMethod, Throwable cause,
            Date retryAfter, Request request) {
        super(status, message, httpMethod, cause, retryAfter, request);
    }

    public CommonRetryableException(int status, String message, Request.HttpMethod httpMethod, Date retryAfter,
            Request request) {
        super(status, message, httpMethod, retryAfter, request);
    }

    @Override
    public Date retryAfter() {
        return super.retryAfter();
    }

    @Override
    public Request.HttpMethod method() {
        return super.method();
    }

    public Exception getException() {
        return exception;
    }

    public void setException(Exception exception) {
        this.exception = exception;
    }
}
