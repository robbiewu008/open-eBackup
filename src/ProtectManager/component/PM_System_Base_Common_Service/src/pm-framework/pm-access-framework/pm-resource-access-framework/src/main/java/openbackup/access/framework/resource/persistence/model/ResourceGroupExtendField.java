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

import lombok.Data;
import openbackup.system.base.query.PageQueryConfig;

/**
 * 功能描述
 *
 */
@Data
@PageQueryConfig(conditions = {"%sla_name%", "is_sla_compliance"})
public class ResourceGroupExtendField extends ResourceGroupPo {
    /**
     * 任务所在节点名称
     */
    private String slaName;

    /**
     * 任务所在节点名称
     */
    private Boolean isSlaCompliance;
}