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
package openbackup.system.base.sdk.lock;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

/**
 * 资源锁同步加锁响应
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/10/15
 **/
@Setter
@Getter
@NoArgsConstructor
public class LockResponse {
    /**
     * 是否加锁成功：true-成功，false-失败
     */
    @JsonProperty("isSuccess")
    private boolean isSuccess;

    /**
     * 失败的资源，当加锁失败时返回 <br>
     * isSuccess为false可能是由于其他错误导致，此字段并不一定会返回
     */
    private String failedResource;
}
