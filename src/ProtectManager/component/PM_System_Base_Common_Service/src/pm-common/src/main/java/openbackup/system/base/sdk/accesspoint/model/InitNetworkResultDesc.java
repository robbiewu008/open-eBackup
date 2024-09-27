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
package openbackup.system.base.sdk.accesspoint.model;

import openbackup.system.base.sdk.accesspoint.model.enums.InitNetworkResultCode;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;
import lombok.ToString;

/**
 * 初始化动作结果描述
 *
 * @author swx1010572
 * @since 2021-01-15
 */
@Data
@ToString
@NoArgsConstructor
@AllArgsConstructor
public class InitNetworkResultDesc {
    /**
     * 动过结果编码
     */
    private InitNetworkResultCode code;

    /**
     * 动作结果描述
     */
    private String desc;
}
