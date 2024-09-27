package openbackup.system.base.common.rest;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.ErrorResponse;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.exception.LegoUncheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.NumberUtil;

import feign.Response;
import feign.Util;
import feign.codec.Decoder;
import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.ObjectFactory;
import org.springframework.boot.autoconfigure.http.HttpMessageConverters;
import org.springframework.cloud.openfeign.support.ResponseEntityDecoder;
import org.springframework.cloud.openfeign.support.SpringDecoder;
import org.springframework.http.HttpStatus;

import java.io.IOException;
import java.net.ConnectException;

/**
 * ErrorDecoder
 *
 * @author dWX1009286
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-12-09
 */
@Slf4j
public class CommonDecoder {
    private static final String[] ERROR_RESPONSE_FIELDS =
            new String[] {"errorCode", "errorMessage", "detailParams", "retryable"};
    private static final String[] ERROR_FIELDS = new String[] {"timestamp", "status", "message", "path"};

    private static final String STATUS = "status";

    private static Exception baseErrorDecode(String methodKey, Response response, boolean shouldThrowRetryEx) {
        if (response == null) {
            return new LegoUncheckedException("response null exception");
        }
        try {
            if (response.body() == null) {
                log.error("response body is null. res status: {}, reason: {}", response.status(), response.reason());
                if (response.status() == HttpStatus.UNAUTHORIZED.value()) {
                    LegoCheckedException exception =
                            new LegoCheckedException(CommonErrorCode.ACCESS_DENIED, response.reason());
                    return new LegoUncheckedException(exception);
                }
                return new LegoUncheckedException("response body null exception");
            }
            String content = Util.toString(response.body().asReader());
            log.error("Error, url={}, content={}.", response.request().url(), content);
            if (!JSONObject.isValidJson(content)) {
                // 如果响应值不是正确的json格式，直接抛出需要重试的异常
                return getErrorException(new LegoUncheckedException("response is not valid json str"), response,
                        shouldThrowRetryEx);
            }
            JSONObject json = JSONObject.fromObject(content);
            JSONObject data = json.pick(ERROR_RESPONSE_FIELDS);
            if (data.size() > IsmNumberConstant.ZERO) {
                ErrorResponse errorResponse = JSONObject.fromObject(content).toBean(ErrorResponse.class);
                long errorCode = NumberUtil.convertToLong(errorResponse.getErrorCode());
                String[] params = errorResponse.getDetailParams();
                String message = errorResponse.getErrorMessage();
                LegoCheckedException exception = new LegoCheckedException(errorCode, params, message);
                exception.setRetryable(errorResponse.isRetryable());
                if (errorResponse.isRetryable()) {
                    return getErrorException(new LegoUncheckedException(errorCode, message), response,
                            shouldThrowRetryEx);
                }
                return exception;
            }
            JSONObject error = json.pick(ERROR_FIELDS);
            if (error.size() == ERROR_FIELDS.length) {
                return getErrorException(new ConnectException(error.getString("error") + " " + error.getString("path")),
                        response, shouldThrowRetryEx);
            }
            JSONObject status = json.pick(STATUS);
            if (Integer.parseInt(String.valueOf(status.get(STATUS))) == 500) {
                return getErrorException(new LegoUncheckedException(response.reason()), response, false);
            }
            return getErrorException(new LegoUncheckedException(response.reason()), response, shouldThrowRetryEx);
        } catch (IOException e) {
            return e;
        }
    }

    private static Exception getErrorException(Exception exception, Response response, boolean shouldThrowRetryEx) {
        if (shouldThrowRetryEx) {
            CommonRetryableException commonRetryableException = new CommonRetryableException(response.status(),
                    exception.getMessage(), response.request().httpMethod(), exception, null, response.request());
            commonRetryableException.setException(exception);
            return commonRetryableException;
        } else {
            return exception;
        }
    }

    /**
     * error decode
     *
     * @param methodKey method key
     * @param response response
     * @return exception
     */
    public static Exception errorDecode(String methodKey, Response response) {
        return baseErrorDecode(methodKey, response, false);
    }

    /**
     * retry error decode
     *
     * @param methodKey method key
     * @param response response
     * @return exception
     */
    public static Exception retryableErrorDecode(String methodKey, Response response) {
        return baseErrorDecode(methodKey, response, true);
    }

    /**
     * decoder
     *
     * @return decoder
     */
    public static Decoder decoder() {
        return new ResponseEntityDecoder(new SpringDecoder(feignHttpMessageConverter()));
    }

    private static ObjectFactory<HttpMessageConverters> feignHttpMessageConverter() {
        final HttpMessageConverters httpMessageConverters =
                new HttpMessageConverters(new PhpMappingJackson2HttpMessageConverter());
        return () -> httpMessageConverters;
    }
}
