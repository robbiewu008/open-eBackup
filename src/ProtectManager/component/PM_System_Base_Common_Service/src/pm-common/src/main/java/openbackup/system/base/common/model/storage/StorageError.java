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
package openbackup.system.base.common.model.storage;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.VerifyUtil;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 功能描述
 *
 * @author y00413474
 * @since 2020-07-01
 */
@Data
public class StorageError {
    /**
     * 错误编码
     */
    @JsonProperty("code")
    private String code;

    /**
     * 错误描述
     */
    @JsonProperty("description")
    private String description;

    /**
     * 错误建议
     */
    @JsonProperty("suggestion")
    private String suggestion;

    /**
     * 错误参数
     */
    @JsonProperty("errorParam")
    private String errorParam;

    /**
     * 从device获取的值是否成功
     *
     * @return 接口是否返回成功
     */
    public boolean isSuccess() {
        return !VerifyUtil.isEmpty(code) && String.valueOf(LegoNumberConstant.ZERO).equals(code);
    }
}
