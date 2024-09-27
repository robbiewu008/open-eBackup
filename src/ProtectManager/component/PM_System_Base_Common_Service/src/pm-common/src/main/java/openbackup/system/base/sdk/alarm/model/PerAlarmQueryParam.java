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
package openbackup.system.base.sdk.alarm.model;

import lombok.Getter;
import lombok.NoArgsConstructor;
import lombok.Setter;

import org.hibernate.validator.constraints.Length;

import javax.validation.constraints.NotEmpty;

/**
 * 查询告警详情接口的参数
 *
 * @author y30021475
 * @since 2023-11-13
 */
@Getter
@Setter
@NoArgsConstructor
public class PerAlarmQueryParam {
    /**
     * 告警唯一标识（经过base64编码）
     */
    @NotEmpty
    @Length(min = 1, max = 10240, message = "Please fill in the correct entity.")
    private String entityId;

    /**
     * 告警 Entity 构造函数
     *
     * @param entityId 告警唯一标识（经过base64编码）
     */
    public PerAlarmQueryParam(String entityId) {
        this.entityId = entityId;
    }
}
