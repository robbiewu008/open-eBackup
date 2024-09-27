package openbackup.system.base.common.exception;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@ResponseStatus(HttpStatus.UNAUTHORIZED)
public class EmeiStorUnauthorizedException extends RuntimeException {
    /**
     * 包含全部参数的构造函数
     *
     * @param message 错误信息
     */
    public EmeiStorUnauthorizedException(String message) {
        super(message);
    }
}
