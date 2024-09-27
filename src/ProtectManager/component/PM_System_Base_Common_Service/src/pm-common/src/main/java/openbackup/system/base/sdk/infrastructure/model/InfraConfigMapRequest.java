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
package openbackup.system.base.sdk.infrastructure.model;

import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NoArgsConstructor;

/**
 * k8s configMap 对象
 *
 * @author fwx1022842
 * @version [OceanProtect DataBackup 1.3.0]
 * @since 2023/2/7
 */
@Data
@AllArgsConstructor
@NoArgsConstructor
public class InfraConfigMapRequest {
    /**
     * 命名空间
     */
    private String nameSpace;

    /**
     * configMap的名称
     */
    private String secretMap;

    /**
     * data中的key
     */
    private String secretKey;

    /**
     * data中的value
     */
    private String secretValue;
}
