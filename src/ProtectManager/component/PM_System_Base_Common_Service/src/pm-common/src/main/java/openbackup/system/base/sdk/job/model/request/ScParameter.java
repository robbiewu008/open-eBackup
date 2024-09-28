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
package openbackup.system.base.sdk.job.model.request;

import lombok.Getter;
import lombok.Setter;

import java.util.Map;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.Size;

/**
 * 高级配置，k8s恢复才有
 *
 */
@Setter
@Getter
public class ScParameter {
    /**
     * 储存类名字
     */
    @NotEmpty
    @Size(min = 1, max = 50)
    private String scName;

    /**
     * 参数
     */
    @NotEmpty
    @Size(min = 1, max = 10)
    private Map<String, String> paramMap;
}
