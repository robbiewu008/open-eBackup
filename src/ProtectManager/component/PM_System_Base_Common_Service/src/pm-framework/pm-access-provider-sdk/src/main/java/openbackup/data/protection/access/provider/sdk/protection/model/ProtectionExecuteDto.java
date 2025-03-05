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
package openbackup.data.protection.access.provider.sdk.protection.model;

import lombok.Data;
import openbackup.data.protection.access.provider.sdk.resourcegroup.dto.ResourceGroupDto;
import openbackup.system.base.common.utils.JSONObject;

import java.util.List;

/**
 * 保护请求体
 *
 */
@Data
public class ProtectionExecuteDto {
    /**
     * 策略id
     */
    private String slaId;

    /**
     * 资源组
     */
    private ResourceGroupDto resource;

    /**
     * 保护扩展参数
     */
    private JSONObject extParameters;

    /**
     * 后置处理动作
     */
    private String postAction;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 用户id
     */
    private String userId;

    /**
     * 资源组子资源
     */
    private List<ProtectionResourceDto> subResources;
}
