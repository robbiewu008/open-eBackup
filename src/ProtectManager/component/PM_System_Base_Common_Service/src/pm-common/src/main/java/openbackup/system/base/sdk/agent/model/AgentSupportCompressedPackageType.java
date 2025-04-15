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
package openbackup.system.base.sdk.agent.model;

import lombok.Getter;
import lombok.Setter;

/**
 * 检查agent包支持类型请求体
 *
 */
@Getter
@Setter
public class AgentSupportCompressedPackageType {
    /**
     * 是否支持TAR命令
     */
    private String isSupportTar;

    /**
     * 是否支持zip，unzip命令
     */
    private String isSupportUnzip;
}
