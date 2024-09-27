/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 */

package openbackup.system.base.common.exception;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 异常对象类
 *
 * @author zKF66175
 * @version [V100R002C00, 2013-2-5]
 * @since 2019-10-25
 */
@NoArgsConstructor
@AllArgsConstructor
@Getter
@Setter
public class ErrorResponse {
    private String errorCode;

    private String errorMessage;

    @JsonProperty("parameters")
    private String[] detailParams;

    private boolean isRetryable = false;

    /**
     * create error response
     *
     * @param ex lego checked exception
     * @return error response
     */
    public static ErrorResponse create(LegoCheckedException ex) {
        String errorCodeKey = ex.getErrorMessageKey();
        return new ErrorResponse(errorCodeKey, ex.getMessage(), ex.getParameters(), ex.isRetryable());
    }
}
