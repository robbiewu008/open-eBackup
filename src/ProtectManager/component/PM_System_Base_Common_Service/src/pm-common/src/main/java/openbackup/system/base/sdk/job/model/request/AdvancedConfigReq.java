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
import openbackup.system.base.common.validator.constants.RegexpConstants;

import java.util.List;
import java.util.Map;

import javax.validation.constraints.NotEmpty;
import javax.validation.constraints.Pattern;
import javax.validation.constraints.Size;

/**
 * 高级配置，k8s恢复才有
 *
 */
@Setter
@Getter
public class AdvancedConfigReq {
    // 工作负载类型
    @NotEmpty
    @Size(min = 1, max = 50)
    private String workLoadType;

    // 工作负载名称
    @NotEmpty
    @Size(min = 1, max = 50)
    @Pattern(regexp = RegexpConstants.NAME_STR)
    private String workLoadName;

    // 容器名称
    @NotEmpty
    @Size(min = 1, max = 50)
    @Pattern(regexp = RegexpConstants.NAME_STR)
    private String containerName;

    // 环境变量
    @Size(min = 1, max = 10)
    @NotEmpty
    private List<Map<String, String>> envMap;
}
