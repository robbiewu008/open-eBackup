package openbackup.system.base.common.exception;

import org.springframework.http.HttpStatus;
import org.springframework.web.bind.annotation.ResponseStatus;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@ResponseStatus(HttpStatus.BAD_REQUEST)
public class EmeiStorBadRequestException extends RuntimeException {
    /**
     * EmeiStorBadRequestException
     *
     * @param errorCode errorCode
     */
    public EmeiStorBadRequestException(String errorCode) {
        super(errorCode);
    }
}
