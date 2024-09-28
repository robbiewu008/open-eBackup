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
package openbackup.system.base.sdk.resource.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 批量更新资源可恢复情况请求体
 *
 */
@Getter
@Setter
public class UpdateRestoreObjectReq {
    /**
     * 资源id列表
     */
    private List<String> resourceIds;

    /**
     * 当前资源是否支持恢复，true: 支持恢复， false:不支持恢复
     */
    private String isAllowRestore;
}
