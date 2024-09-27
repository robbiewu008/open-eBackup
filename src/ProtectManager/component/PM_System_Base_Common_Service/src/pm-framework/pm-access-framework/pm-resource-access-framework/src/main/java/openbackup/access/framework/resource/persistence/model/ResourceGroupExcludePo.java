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
package openbackup.access.framework.resource.persistence.model;

import openbackup.system.base.query.PageQueryConfig;

import lombok.Data;

/**
 * 资源组查询参数
 *
 * @author l00853347
 * @version [OceanProtect DataBackup 1.7.0]
 * @since 2024-06-04
 */
@Data
@PageQueryConfig(conditions = {"resource_set_id"})
public class ResourceGroupExcludePo extends ResourceGroupPo {
    private String resourceSetId;
}
