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
package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import java.util.List;

/**
 * The ActionResult
 *
 * @author g30003063
 * @since 2022-05-20
 */
@Getter
@Setter
@NoArgsConstructor
public class ActionResult {
    /**
     * 成功码
     */
    public static final long SUCCESS_CODE = 0L;

    private long code = 0L;

    /**
     * 错误码
     */
    private String bodyErr;

    private String message;

    private List<String> detailParams;

    /**
     * 有参构造
     *
     * @param code 错误码
     * @param message 信息
     */
    public ActionResult(final long code, final String message) {
        this.code = code;
        this.message = message;
    }
}