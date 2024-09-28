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
package openbackup.system.base.common.rest;

import openbackup.system.base.common.constants.FeignClientConstant;

import lombok.Data;

/**
 * Common Retry Policy
 *
 */
@Data
public class CommonRetryPolicy {
    /**
     * FeignClient Retry重试次数
     */
    private int attempts = 6;

    /**
     * FeignClient Retry 间隔周期(ms)
     */
    private long waitTime = FeignClientConstant.PERIOD;
}
