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
package openbackup.system.base.sdk.anti.model;

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 安全卡响应
 *
 */
@Getter
@Setter
public class SecureCardResp {
    /**
     * 安全卡模块是否存在。true：存在；false：不存在
     */
    @JsonProperty("securityModuleFeature")
    boolean hasSecurityModuleFeature;

    /**
     * 安全卡模块详细信息。
     */
    @JsonProperty("securityModuleinfo")
    List<SecureCardInfo> securityModuleinfo;
}
