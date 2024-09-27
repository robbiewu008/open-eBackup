/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
