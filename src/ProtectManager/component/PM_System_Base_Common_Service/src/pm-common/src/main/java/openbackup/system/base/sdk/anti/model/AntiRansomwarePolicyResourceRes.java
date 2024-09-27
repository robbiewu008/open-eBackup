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

import lombok.Data;

/**
 * 防勒索资源对象
 *
 * @author nwx1077006
 * @since 2021-11-12
 */
@Data
public class AntiRansomwarePolicyResourceRes {
    // 资源id
    private String resourceId;

    // 资源名称
    private String resourceName;

    // 资源路径
    private String resourceLocation;

    // 资源子类型
    private String resourceSubType;
}