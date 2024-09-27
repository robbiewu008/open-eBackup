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
package com.huawei.emeistor.console.bean;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;

/**
 * 证书详情对象
 *
 * @author fwx1022842
 * @version [OceanProtect A8000 1.1.0]
 * @since 2022/3/18
 */
@Data
public class CertDetailResponse {
    /**
     * 是否安全
     */
    @JsonProperty("safety")
    private Boolean isSafety;
}
