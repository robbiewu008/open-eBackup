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
package openbackup.system.base.sdk.auth.model.request;

import static openbackup.system.base.common.constants.LegoNumberConstant.THIRTY_TWO;
import static openbackup.system.base.common.constants.LegoNumberConstant.THROUND_TWENTY_FOUR;
import static openbackup.system.base.common.constants.LegoNumberConstant.TWO_HUNDRED_FIFTY_SIX;

import lombok.Getter;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotNull;

/**
 * 预置账号request
 *
 */
@Getter
@Setter
public class PresetAccountRequest {
    @Length(max = TWO_HUNDRED_FIFTY_SIX)
    @NotNull
    private String userName;

    @Length(max = THROUND_TWENTY_FOUR)
    @NotNull
    private String userPwd;

    @Length(max = THIRTY_TWO)
    @NotNull
    private String sourceType;
}
