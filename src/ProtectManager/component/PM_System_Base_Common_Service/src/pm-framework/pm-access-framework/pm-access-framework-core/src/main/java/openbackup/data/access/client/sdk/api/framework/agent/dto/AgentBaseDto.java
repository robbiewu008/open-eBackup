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
package openbackup.data.access.client.sdk.api.framework.agent.dto;

import openbackup.system.base.common.constants.LegoNumberConstant;
import openbackup.system.base.common.utils.VerifyUtil;

import lombok.Data;

/**
 * Agent Base Dto
 *
 */
@Data
public class AgentBaseDto {
    private String errorCode;

    private String errorMessage;

    /**
     * 从Agent获取的值是否成功
     *
     * @return 接口是否返回成功
     */
    public boolean isAgentBaseDtoReturnSuccess() {
        return !VerifyUtil.isEmpty(errorCode) && Integer.parseInt(errorCode) == LegoNumberConstant.ZERO;
    }
}
