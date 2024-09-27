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
package openbackup.data.protection.access.provider.sdk.resource;

import lombok.Getter;
import lombok.Setter;

/**
 * 资源脱敏对象
 *
 * @author c30016231
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023-04-08
 */
@Getter
@Setter
public class ResourceDesesitization {
    /**
     * 脱敏状态
     */
    private String desesitizationStatus;

    /**
     * 识别状态
     */
    private String identificationStatus;

    /**
     * 脱敏任务id
     */
    private String desesitizationJobId;

    /**
     * 识别任务id
     */
    private String identificationJobId;

    /**
     * 脱敏策略id
     */
    private String desesitizationPolicyId;

    /**
     * 脱敏策略名称
     */
    private String desesitizationPolicyName;
}
