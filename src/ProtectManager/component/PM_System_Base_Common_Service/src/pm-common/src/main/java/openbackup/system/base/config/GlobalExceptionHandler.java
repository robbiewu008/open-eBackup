package openbackup.system.base.config;

import openbackup.system.base.common.annotation.Sensitive;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.DeviceManagerException;
import openbackup.system.base.common.exception.ErrorResponse;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.ExceptionUtil;
import openbackup.system.base.util.ErrorUtil;

import com.fasterxml.jackson.databind.JsonMappingException;
import com.fasterxml.jackson.databind.exc.InvalidFormatException;

import feign.FeignException;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringEscapeUtils;
import org.apache.commons.lang3.StringUtils;
import org.hibernate.validator.internal.engine.path.PathImpl;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.context.MessageSource;
import org.springframework.context.NoSuchMessageException;
import org.springframework.context.support.DefaultMessageSourceResolvable;
import org.springframework.core.MethodParameter;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.http.converter.HttpMessageNotReadableException;
import org.springframework.validation.BindException;
import org.springframework.validation.BindingResult;
import org.springframework.validation.FieldError;
import org.springframework.validation.ObjectError;
import org.springframework.web.bind.MethodArgumentNotValidException;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestControllerAdvice;
import org.springframework.web.context.request.RequestAttributes;
import org.springframework.web.context.request.RequestContextHolder;
import org.springframework.web.context.request.ServletRequestAttributes;

import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

import javax.servlet.http.HttpServletRequest;
import javax.validation.ConstraintViolation;
import javax.validation.ConstraintViolationException;
import javax.validation.Path;

/**
 * GlobalExceptionHandler
 *
 * @author w00448845
 * @version [CDM Integrated machine]
 * @since 2019-11-09
 */
@Slf4j
@RestControllerAdvice
public class GlobalExceptionHandler {
    private static final Set<Long> UNKNOWN_ERROR_CODES = new HashSet<>(Collections.singletonList(-1L));

    @Autowired
    @Qualifier("errorMessageSource")
    private MessageSource messageSource;

    /**
     * handleAllExceptions
     *
     * @param ex ex
     * @return ResponseEntity<Object>
     */
    @ExceptionHandler(value = Exception.class)
    @ResponseBody
    public final ResponseEntity<Object> handleAllExceptions(Exception ex) {
        logException(ex);
        ErrorResponse response = new ErrorResponse();
        response.setErrorCode(CommonErrorCode.OPERATION_FAILED + "");
        response.setErrorMessage(messageSource.getMessage(CommonErrorCode.OPERATION_FAILED + "", null, null));
        response.setRetryable(ErrorUtil.isRetryableException(ex));
        return new ResponseEntity<>(response, HttpStatus.INTERNAL_SERVER_ERROR);
    }

    /**
     * handleBindException
     *
     * @param ex BindException
     * @return ResponseEntity<ErrorResponse>
     */
    @ExceptionHandler(value = BindException.class)
    @ResponseBody
    public final ResponseEntity<ErrorResponse> handleBindException(BindException ex) {
        logException(ex);
        return getBindExceptionResponse(ex);
    }

    /**
     * handleHttpMessageNotReadableException
     *
     * @param ex HttpMessageNotReadableException
     * @return ResponseEntity<Object>
     */
    @ExceptionHandler(HttpMessageNotReadableException.class)
    @ResponseBody
    public ResponseEntity<Object> handleHttpMessageNotReadableException(HttpMessageNotReadableException ex) {
        if (ex.getCause() instanceof JsonMappingException) {
            return handleJsonMappingException((JsonMappingException) ex.getCause());
        }
        return handleAllExceptions(ex);
    }

    private ResponseEntity<Object> handleJsonMappingException(JsonMappingException ex) {
        if (ex instanceof InvalidFormatException) {
            return getBadRequestResponse(ex);
        }
        if (ex.getCause() instanceof LegoCheckedException) {
            ErrorResponse errorResponse = new ErrorResponse();
            errorResponse.setErrorCode(CommonErrorCode.ILLEGAL_PARAM + StringUtils.EMPTY);
            errorResponse.setErrorMessage(ex.getCause().getMessage());
            errorResponse.setRetryable(ErrorUtil.isRetryableException(ex.getCause()));
            return new ResponseEntity<>(errorResponse, HttpStatus.INTERNAL_SERVER_ERROR);
        }
        return handleAllExceptions(ex);
    }

    /**
     * methodArgumentNotValidException
     *
     * @param ex methodArgumentNotValidException
     * @return response
     */
    @ExceptionHandler(MethodArgumentNotValidException.class)
    @ResponseBody
    public ResponseEntity<ErrorResponse> methodArgumentNotValidException(MethodArgumentNotValidException ex) {
        logException(ex);
        return getBindExceptionResponse(ex);
    }

    private ResponseEntity<ErrorResponse> getBindExceptionResponse(BindException ex) {
        BindingResult bindingResult = ex.getBindingResult();
        List<ObjectError> allErrors = bindingResult.getAllErrors();
        ErrorResponse response = new ErrorResponse();
        response.setErrorCode(CommonErrorCode.ERR_PARAM + "");
        String messages = allErrors.stream()
            .map(objectError -> getDefaultMessage(objectError).orElse(objectError.getDefaultMessage()))
            .collect(Collectors.joining(System.lineSeparator()));
        response.setErrorMessage(messages);
        return new ResponseEntity<>(response, HttpStatus.BAD_REQUEST);
    }

    /**
     * constraintViolationException
     *
     * @param ex ConstraintViolationException
     * @return response
     */
    @ExceptionHandler(ConstraintViolationException.class)
    @ResponseBody
    public ResponseEntity<ErrorResponse> constraintViolationException(ConstraintViolationException ex) {
        Set<ConstraintViolation<?>> violations = ex.getConstraintViolations();
        String messages = violations.stream().map(violation -> {
            final Path propertyPath = violation.getPropertyPath();
            assert propertyPath instanceof PathImpl;
            PathImpl pathImpl = (PathImpl) propertyPath;
            String key = pathImpl.getLeafNode().getName();
            String value = violation.getMessage();
            return String.join(": ", key, value);
        }).collect(Collectors.joining(", "));
        ErrorResponse response = new ErrorResponse();
        response.setErrorCode(CommonErrorCode.ERR_PARAM + "");
        response.setErrorMessage(messages);
        return new ResponseEntity<>(response, HttpStatus.BAD_REQUEST);
    }

    private Optional<String> getDefaultMessage(ObjectError objectError) {
        Object[] arguments = objectError.getArguments();
        if (arguments == null || arguments.length == 0) {
            return Optional.empty();
        }
        Object argument = arguments[0];
        if (!(argument instanceof DefaultMessageSourceResolvable)) {
            return Optional.empty();
        }
        DefaultMessageSourceResolvable resolvable = (DefaultMessageSourceResolvable) argument;
        String code = resolvable.getCode();
        if (code == null) {
            return Optional.empty();
        }
        final RequestAttributes requestAttributes = RequestContextHolder.getRequestAttributes();
        if (!(requestAttributes instanceof ServletRequestAttributes)) {
            return Optional.empty();
        }
        ServletRequestAttributes servletRequestAttributes = (ServletRequestAttributes) requestAttributes;
        HttpServletRequest request = servletRequestAttributes.getRequest();
        Locale locale = request.getLocale();
        String errorMessage;
        if (Locale.SIMPLIFIED_CHINESE.equals(locale)) {
            errorMessage = code + objectError.getDefaultMessage();
        } else {
            errorMessage = code + " " + objectError.getDefaultMessage();
        }
        return Optional.of(errorMessage);
    }

    /**
     * 处理错误码exception
     *
     * @param ex 异常信息
     * @return 返回给请求的对象
     */
    @ExceptionHandler(value = LegoCheckedException.class)
    @ResponseBody
    public final ResponseEntity<ErrorResponse> businessException(LegoCheckedException ex) {
        return businessException(ex, ex);
    }

    private final ResponseEntity<ErrorResponse> businessException(LegoCheckedException ex, Throwable cause) {
        logException(ex);
        Long errorCode = getErrorCode(ex.getErrorCode());
        String errorMessage = getErrorMessage(ex);
        String[] parameters = ex.getParameters();
        ErrorResponse errorResp = new ErrorResponse(errorCode + "", errorMessage, parameters,
            ErrorUtil.isRetryableException(cause));
        HttpStatus status = HttpStatus.INTERNAL_SERVER_ERROR;
        if (errorCode == CommonErrorCode.ERR_PARAM) {
            status = HttpStatus.BAD_REQUEST;
        }
        if (errorCode == CommonErrorCode.ACCESS_DENIED) {
            status = HttpStatus.FORBIDDEN;
        }
        return new ResponseEntity<>(errorResp, status);
    }

    /**
     * process feign exception
     *
     * @param ex exception
     * @return response
     */
    @ExceptionHandler(value = FeignException.class)
    @ResponseBody
    public final ResponseEntity<ErrorResponse> processFeignException(FeignException ex) {
        LegoCheckedException legoCheckedException = ExceptionUtil.lookFor(ex, LegoCheckedException.class);
        if (legoCheckedException != null) {
            return businessException(legoCheckedException);
        }
        DeviceManagerException deviceManagerException = ExceptionUtil.lookFor(ex, DeviceManagerException.class);
        if (deviceManagerException != null) {
            businessException(deviceManagerException.toLegoException());
        }
        log.error("invoke external service failed.", ExceptionUtil.getErrorMessage(ex));
        return businessException(new LegoCheckedException(CommonErrorCode.OPERATION_FAILED), ex);
    }

    private void logException(Exception ex) {
        Exception newException = new Exception(StringEscapeUtils.escapeJava(ex.getMessage()));
        newException.setStackTrace(ex.getStackTrace());
        log.error("An error occurred when invoking the REST interface", newException);
    }

    private void logMethodArgumentNotValidError(MethodArgumentNotValidException ex) {
        if (containsSensitiveInformation(ex)) {
            MethodParameter methodParameter = ex.getParameter();
            log.error("argument {} of {} is not valid", methodParameter.getParameterIndex(),
                methodParameter.getExecutable());
        } else {
            logException(ex);
        }
    }

    private boolean containsSensitiveInformation(MethodArgumentNotValidException ex) {
        MethodParameter parameter = ex.getParameter();
        return ex.getBindingResult()
            .getAllErrors()
            .stream()
            .anyMatch(error -> containsSensitiveInformation(error, parameter));
    }

    private boolean containsSensitiveInformation(ObjectError error, MethodParameter parameter) {
        if (error instanceof FieldError) {
            FieldError fieldError = (FieldError) error;
            return containsSensitiveInformation(fieldError, parameter);
        }
        return parameter.getParameterAnnotation(Sensitive.class) != null;
    }

    private Long getErrorCode(Long code) {
        if (UNKNOWN_ERROR_CODES.contains(code)) {
            return CommonErrorCode.OPERATION_FAILED;
        } else {
            return code;
        }
    }

    private String getErrorMessage(LegoCheckedException ex) {
        String message;
        try {
            String errorCode = getErrorCode(ex.getErrorCode()) + "";
            message = messageSource.getMessage(errorCode, ex.getParameters(), null);
        } catch (NoSuchMessageException noSuchMessageException) {
            message = ex.getMessage();
        }
        return message;
    }

    private ResponseEntity<Object> getBadRequestResponse(Exception ex) {
        ErrorResponse errorResponse = new ErrorResponse();
        errorResponse.setErrorCode(CommonErrorCode.ERR_PARAM + StringUtils.EMPTY);
        errorResponse.setErrorMessage(ExceptionUtil.getErrorMessage(ex).getMessage());
        errorResponse.setRetryable(ErrorUtil.isRetryableException(ex));
        return new ResponseEntity<>(errorResponse, HttpStatus.BAD_REQUEST);
    }
}
