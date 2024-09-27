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

import lombok.Data;

/**
 * Agent返回的支持的应用数据模型
 *
 * @author w00616953
 * @since 2021-12-03
 */
@Data
public class SupportApplicationDto {
    /**
     * 应用类型
     */
    private String application;

    /**
     * 支持应用的最小版本，可能没有限制
     */
    private String minVersion;

    /**
     * 支持应用的最大版本，可能没有限制
     */
    private String maxVersion;
}
