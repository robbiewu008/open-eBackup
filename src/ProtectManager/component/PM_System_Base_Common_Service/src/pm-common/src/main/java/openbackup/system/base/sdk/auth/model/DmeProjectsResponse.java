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
package openbackup.system.base.sdk.auth.model;

import lombok.Getter;
import lombok.Setter;

import java.util.List;

/**
 * 查询dme用户关联的项目列表返回体
 *
 * @author z30062305
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-08-29
 */
@Setter
@Getter
public class DmeProjectsResponse {
    /**
     * 查询的用户关联资源集的总数
     */
    private Integer total;

    /**
     * dme资源集列表
     */
    private List<DmeProject> projects;
}
