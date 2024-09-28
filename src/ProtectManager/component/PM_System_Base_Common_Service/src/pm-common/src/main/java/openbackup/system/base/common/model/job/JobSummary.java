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
package openbackup.system.base.common.model.job;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * Task summary response
 *
 */
@Data
@NoArgsConstructor
@AllArgsConstructor
public class JobSummary {
    private long total;

    private long success;

    private long fail;

    private long pending;

    private long dispatching;

    private long redispatch;

    private long running;

    private long aborted;

    private long ready;

    private long aborting;

    @JsonProperty("partial_success")
    private long partialSuccess;

    private long abnormal;

    @JsonProperty("abort_failed")
    private long abortFailed;

    @JsonProperty("dispatch_failed")
    private long dispatchFailed;

    private long cancelled;
}
