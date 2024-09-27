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

import com.fasterxml.jackson.annotation.JsonProperty;

import lombok.Data;
import lombok.EqualsAndHashCode;

import java.util.List;

/**
 * FileSet Entity
 *
 * @author l00272247
 * @since 2020-07-14
 */
@EqualsAndHashCode(callSuper = true)
@Data
public class FileSetEntity extends ResourceEntity {
    @JsonProperty("sla_id")
    private String slaId;

    @JsonProperty("sla_name")
    private String slaName;

    @JsonProperty("sla_status")
    private String slaStatus;

    @JsonProperty("sla_compliance")
    private String slaCompliance;

    // 文件集中包含的全路径
    private List<String> paths;

    // 四种过滤方式
    private List<Filter> filters;
}
