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
package openbackup.oceanprotect.k8s.protection.access.dto;

import lombok.Getter;
import lombok.Setter;
import openbackup.system.base.common.utils.JSONArray;

/**
 * 对应某个具体的pod/deployment/statefulset/daemonset
 *
 * @author n30046257
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/8/2
 */
@Getter
@Setter
public class ApplicationResource {
    private String name;
    private int selectedPvcNumbers;
    private JSONArray pvcs;
}
